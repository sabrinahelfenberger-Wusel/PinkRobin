export type WeatherAnswerKind = "unknown" | "dry" | "rain" | "storm";

export type ConversationReplyKind =
  "unknownInput" | "askPostalCode" | "invalidPostalCode" | "weatherUnavailable";

export type CoreOutput =
  | {
      sequence: string;
      kind: "weatherAnswer";
      weatherAnswer: WeatherAnswerKind;
      text: string;
    }
  | {
      sequence: string;
      kind: "conversationReply";
      conversationReply: ConversationReplyKind;
      text: string;
    };

export function parseCoreOutput(value: unknown): CoreOutput | null {
  if (value === null) return null;

  if (!value || typeof value !== "object") {
    throw new Error("Invalid Core output");
  }

  const output = value as Record<string, unknown>;
  const sequence = output["sequence"];

  if (
    typeof sequence !== "string" ||
    !/^[1-9][0-9]{0,19}$/.test(sequence) ||
    (sequence.length === 20 && sequence > "18446744073709551615")
  ) {
    throw new Error("Invalid Core output sequence");
  }

  const text = output["text"];

  if (
    typeof text !== "string" ||
    text.trim().length === 0 ||
    new TextEncoder().encode(text).length > 512
  ) {
    throw new Error("Invalid Core output text");
  }

  if (
    output["kind"] === "weatherAnswer" &&
    typeof output["weatherAnswer"] === "string" &&
    ["unknown", "dry", "rain", "storm"].includes(output["weatherAnswer"])
  ) {
    return {
      sequence,
      kind: "weatherAnswer",
      weatherAnswer: output["weatherAnswer"] as WeatherAnswerKind,
      text,
    };
  }

  if (
    output["kind"] === "conversationReply" &&
    typeof output["conversationReply"] === "string" &&
    [
      "unknownInput",
      "askPostalCode",
      "invalidPostalCode",
      "weatherUnavailable",
    ].includes(output["conversationReply"])
  ) {
    return {
      sequence,
      kind: "conversationReply",
      conversationReply: output["conversationReply"] as ConversationReplyKind,
      text,
    };
  }

  throw new Error("Unsupported Core output");
}
