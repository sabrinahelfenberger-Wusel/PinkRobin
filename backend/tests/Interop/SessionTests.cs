using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;
using static TestSupport;

internal static class SessionTests
{
    public static void Lifecycle()
    {
        // Session time is server-owned and deterministic in tests.
        var clock = new ManualClock();
        using var sessions = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            clock
            );
        var a = sessions.Create(false);
        var b = sessions.Create(false);
        Require(a.Token != b.Token && a.Token.Length == 64, "Independent session tokens");
        Require(sessions.Send(a.Token, new(1, "heartTouched")).Mood == "happy", "Session heart command");
        Require(sessions.Read(b.Token).Mode == "sleeping", "Session isolation");
        sessions.Send(a.Token, new(2, "headStarted"));
        for (var i = 0; i < 5; i++) { clock.Advance(1000); sessions.Read(a.Token); }
        Require(sessions.Read(a.Token).Expression == "heartEyes", "Server time owns heart eyes");
        Require(sessions.Send(a.Token, new(3, "headEnded")).Expression == "curious", "Session head released");
        var revision = sessions.Read(a.Token).Revision;
        Require(sessions.Send(a.Token, new(3, "headEnded")).Revision == revision, "Retry must not replay event");
        Throws<PinkRobin.Api.Sessions.SessionFailure>(() => sessions.Send(a.Token, new(5, "restart")));
        Throws<PinkRobin.Api.Sessions.SessionFailure>(() => sessions.Send(a.Token, new(4, "unknown")));
        sessions.Send(a.Token, new(4, "headStarted"));
        clock.Advance(2001);
        sessions.Sweep();
        Require(!sessions.Read(a.Token).HeadContact, "Lost connection releases head");
        var reducedSession = sessions.Create(true);
        Require(reducedSession.State.Mode == "awake", "Reduced motion skips greeting");
        Require(sessions.Send(reducedSession.Token, new(1, "restart")).Mode == "awake", "Reduced motion restart");
        sessions.Remove(a.Token);
        Throws<PinkRobin.Api.Sessions.SessionFailure>(() => sessions.Read(a.Token));
        clock.Advance(120000);
        sessions.Sweep();
        Throws<PinkRobin.Api.Sessions.SessionFailure>(() => sessions.Read(b.Token));
    }

    public static void Capacity()
    {
        var limited = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            new ManualClock()
        );

        using (limited)
        {
            for (var i = 0; i < 128; i++) limited.Create(false);
            Throws<PinkRobin.Api.Sessions.SessionFailure>(() => limited.Create(false));
        }
        Console.WriteLine("Session isolation, sequence, clock, lease, expiry, reduced motion and capacity passed.");
    }
}
