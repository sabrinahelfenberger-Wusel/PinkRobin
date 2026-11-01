export type WeatherAnswerKind = "unknown" | "dry" | "rain" | "storm";

export interface CoreOutput {
  sequence: string;
  kind: "weatherAnswer";
  weatherAnswer: WeatherAnswerKind;
}

export function parseCoreOutput(value: unknown): CoreOutput | null {
  // HTTP 204 liefert keinen Inhalt.
  if (value === null) return null;

  if (typeof value !== "object" || !value) {
    throw new Error("Invalid Core output");
  }

  const output = value as Record<string, unknown>;
  const sequence = output["sequence"];

  if (
    typeof sequence !== "string" ||
    !/^[1-9][0-9]{0,19}$/.test(sequence) ||
    (sequence.length === 20 && sequence > "18446744073709551615") ||
    output["kind"] !== "weatherAnswer" ||
    !["unknown", "dry", "rain", "storm"].includes(
      output["weatherAnswer"] as string,
    )
  ) {
    throw new Error("Invalid Core output");
  }

  return {
    sequence,
    kind: "weatherAnswer",
    weatherAnswer: output["weatherAnswer"] as WeatherAnswerKind,
  };
}
