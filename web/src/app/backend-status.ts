export interface BackendStatus {
  service: "PinkRobin.Api";
  apiVersion: "1.0";
  core: { connected: boolean; implementation: string };
}

export function parseBackendStatus(value: unknown): BackendStatus | null {
  if (!value || typeof value !== "object") return null;
  const candidate = value as Record<string, unknown>;
  if (
    candidate["service"] !== "PinkRobin.Api" ||
    candidate["apiVersion"] !== "1.0"
  )
    return null;
  const core = candidate["core"];
  if (!core || typeof core !== "object") return null;
  const fields = core as Record<string, unknown>;
  if (
    typeof fields["connected"] !== "boolean" ||
    typeof fields["implementation"] !== "string"
  )
    return null;
  return {
    service: "PinkRobin.Api",
    apiVersion: "1.0",
    core: {
      connected: fields["connected"],
      implementation: fields["implementation"],
    },
  };
}
