import {
  CoreOutput,
  ConversationReplyKind,
  WeatherAnswerKind,
} from "./core-output";

const WEATHER_ANSWERS: Record<WeatherAnswerKind, string> = {
  unknown:
    "Ich habe gerade keine gültigen Wetterdaten. Laden wir das Wetter für deinen Ort?",
  dry: "Die Prognose meldet trockenes Wetter. Was wir wohl draussen entdecken könnten? ♡",
  rain: "Die Prognose meldet Regen. Vergiss deinen Regenschutz nicht! ♡",
  storm:
    "Die Prognose meldet viel Regen und kräftige Böen. Machen wir es uns lieber drinnen gemütlich? ♡",
};

const CONVERSATION_REPLIES: Record<ConversationReplyKind, string> = {
  unknownInput:
    "Ich bin noch klein und verstehe noch nicht alles. Aber ich lerne dazu! ♡",

  askPostalCode:
    "Für welchen Ort möchtest du das Wetter wissen? Nenne mir bitte eine vierstellige Schweizer PLZ. ♡",

  invalidPostalCode:
    "Damit kann ich noch keinen Ort bestimmen. Nenne mir bitte eine vierstellige Schweizer PLZ.",

  weatherUnavailable:
    "Für diesen Ort konnte ich gerade keine gültige Prognose laden. Möchtest du eine andere Schweizer PLZ versuchen?",
};

export function coreOutputText(output: CoreOutput): string {
  return output.kind === "weatherAnswer"
    ? weatherAnswerText(output.weatherAnswer)
    : CONVERSATION_REPLIES[output.conversationReply];
}

export function weatherAnswerText(kind: WeatherAnswerKind): string {
  return WEATHER_ANSWERS[kind];
}
