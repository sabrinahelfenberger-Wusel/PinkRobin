namespace PinkRobin.Api.Sessions;

public sealed class RobinSessionOptions
{
    public const string SectionName = "RobinSessions";

    public TimeSpan IdleTimeout { get; set; } =
        TimeSpan.FromMinutes(15);
}