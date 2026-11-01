import { WeatherAnswerKind } from "./core-output";

const WEATHER_ANSWERS: Record<WeatherAnswerKind, string> = {
  unknown:
    "Ich habe gerade keine gültigen Wetterdaten. Laden wir das Wetter für deinen Ort?",
  dry: "Die Prognose meldet trockenes Wetter. Was wir wohl draussen entdecken könnten? ♡",
  rain: "Die Prognose meldet Regen. Vergiss deinen Regenschutz nicht! ♡",
  storm:
    "Die Prognose meldet viel Regen und kräftige Böen. Machen wir es uns lieber drinnen gemütlich? ♡",
};

export function weatherAnswerText(kind: WeatherAnswerKind): string {
  return WEATHER_ANSWERS[kind];
}
