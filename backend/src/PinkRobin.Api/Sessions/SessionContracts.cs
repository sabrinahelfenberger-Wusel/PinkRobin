namespace PinkRobin.Api.Sessions;

public sealed record CreateSessionRequest(bool ReducedMotion = false);
public sealed record RobinCommand(
    long Sequence,
    string Event,
    string? Text = null);
public sealed record SessionState(
    string Mode,
    string Mood,
    string Expression,
    bool HeadContact,
    ulong TimeMs,
    ulong Revision,
    long Sequence,
    string Transformation);

public sealed record CreatedSession(string Token, SessionState State);
public sealed class SessionFailure(int status, string message) : Exception(message)
{
    public int Status { get; } = status;
}

public sealed record SessionOutput(
    string Sequence,
    string Kind,
    string? WeatherAnswer = null,
    string? ConversationReply = null,
    string? RequestId = null,
    string? PostalCode = null);

public sealed record AcknowledgeOutputRequest(string Sequence);
