namespace PinkRobin.Api.Core;

public enum CoreConversationReplyKind : uint
{
    UnknownInput = 0,
    AskPostalCode = 1,
    InvalidPostalCode = 2,
    WeatherUnavailable = 3
}

public abstract record CoreOutput(ulong Sequence);

public sealed record CoreConversationReplyOutput(
    ulong Sequence,
    CoreConversationReplyKind Kind,
    string Text)
    : CoreOutput(Sequence);

public sealed record CoreWeatherFetchRequestedOutput(
    ulong Sequence,
    ulong RequestId,
    string PostalCode)
    : CoreOutput(Sequence);