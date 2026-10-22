export interface RobinWeather {
  source: "MeteoSwiss";
  postalCode: string;
  pointId: string;
  forecastRunAt: string;
  retrievedAt: string;
  validFrom: string;
  validTo: string;
  temperatureC: number;
  precipitationMm: number;
  windSpeedKmh: number;
  windGustKmh: number;
}

export function parseWeather(
  value: unknown,
  now = Date.now(),
): RobinWeather | null {
  if (!value || typeof value !== "object") return null;
  const w = value as Record<string, unknown>;
  if (
    w["source"] !== "MeteoSwiss" ||
    typeof w["postalCode"] !== "string" ||
    !/^\d{4}$/.test(w["postalCode"]) ||
    typeof w["pointId"] !== "string"
  )
    return null;
  for (const key of [
    "temperatureC",
    "precipitationMm",
    "windSpeedKmh",
    "windGustKmh",
  ]) {
    if (typeof w[key] !== "number" || !Number.isFinite(w[key])) return null;
  }
  for (const key of ["forecastRunAt", "retrievedAt", "validFrom", "validTo"]) {
    if (typeof w[key] !== "string" || !Number.isFinite(Date.parse(w[key])))
      return null;
  }
  if (
    Date.parse(w["validFrom"] as string) > now ||
    Date.parse(w["validTo"] as string) <= now
  )
    return null;
  return value as RobinWeather;
}

export enum RobinTransformation {
  None = 0,
  Octopus = 1,
  Sailor = 2,
  Snowman = 3,
  Raincoat = 4,
  Kite = 5,
}
