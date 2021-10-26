type action =
  | ScriptAdvanced
  | HelpDialogOpened
  | HelpDialogClosed
  | ChoiceSelected(int);

type t = {
  script: list(Script.event),
  isShowingHelpDialog: bool,
  backgroundColorHex: option(string),
  backgroundImage: option(string),
  title: string,
  currentSpeakingCharacter: option(Character.t),
  yksiExpression: Character.expression,
  kaxigExpression: Character.expression,
  kolmeExpression: Character.expression,
  text: React.element,
  displayedChoices: option(array(Script.choice)),
};

let defaultState = {
  script: InitialScript.script,
  isShowingHelpDialog: false,
  backgroundColorHex: None,
  backgroundImage: None,
  title: "",
  currentSpeakingCharacter: None,
  yksiExpression: Character.Neutral,
  kaxigExpression: Character.Neutral,
  kolmeExpression: Character.Neutral,
  text: React.null,
  displayedChoices: None,
};

let textFadeInTime = 1000;

let reducer = (action: action, state: t) =>
  switch (action) {
  | ScriptAdvanced =>
    Belt.Option.isSome(state.displayedChoices)
      ? ReactUpdate.NoUpdate
      : (
        switch (state.script) {
        | [] => ReactUpdate.NoUpdate
        | [nextEvent, ...futureEvents] =>
          let state = {...state, script: futureEvents};
          switch (nextEvent) {
          | Speech(character, text) =>
            ReactUpdate.Update({
              ...state,
              currentSpeakingCharacter: Some(character),
              text:
                <>
                  <FadeInDiv
                    fadeInTime={
                      switch (state.currentSpeakingCharacter) {
                      | Some(currentCharacter) =>
                        currentCharacter == character ? 0 : textFadeInTime
                      | None => textFadeInTime
                      }
                    }>
                    <Text character>
                      {Character.getName(character) ++ ": "}
                    </Text>
                  </FadeInDiv>
                  <FadeInDiv key=text fadeInTime=textFadeInTime>
                    <Text> text </Text>
                  </FadeInDiv>
                </>,
            })
          | ExpressionChange(character, expression) =>
            ReactUpdate.UpdateWithSideEffects(
              switch (character) {
              | Yksi => {...state, yksiExpression: expression}
              | Kaxig => {...state, kaxigExpression: expression}
              | Kolme => {...state, kolmeExpression: expression}
              },
              self => {
                self.send(ScriptAdvanced);
                None;
              },
            )
          | Choice(choices) =>
            ReactUpdate.Update({...state, displayedChoices: Some(choices)})
          | GoToScript(newScript) =>
            ReactUpdate.UpdateWithSideEffects(
              {...state, script: newScript},
              self => {
                self.send(ScriptAdvanced);
                None;
              },
            )
          };
        }
      )
  | ChoiceSelected(index) =>
    let selectedChoice =
      Belt.Option.flatMap(state.displayedChoices, choices =>
        Belt.Array.get(choices, index)
      );

    switch (selectedChoice) {
    | Some({result, _}) =>
      ReactUpdate.UpdateWithSideEffects(
        {...state, displayedChoices: None, script: result},
        self => {
          self.send(ScriptAdvanced);
          None;
        },
      )
    | None => ReactUpdate.NoUpdate
    };
  | HelpDialogOpened =>
    ReactUpdate.Update({...state, isShowingHelpDialog: true})
  | HelpDialogClosed =>
    ReactUpdate.Update({...state, isShowingHelpDialog: false})
  };
