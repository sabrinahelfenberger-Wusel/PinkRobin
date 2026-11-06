namespace PinkRobin.Api.Core;

public enum CoreWeatherAnswerKind : uint
{
    Unknown = 0,
    Dry = 1,
    Rain = 2,
    Storm = 3
}

public sealed record CoreWeatherAnswerOutput(
    ulong Sequence,
    CoreWeatherAnswerKind Kind)
    : CoreOutput(Sequence);