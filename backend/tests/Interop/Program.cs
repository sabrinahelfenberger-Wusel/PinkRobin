using PinkRobin.Api.Core;

static void Require(bool condition, string message)
{
    if (!condition) throw new Exception(message);
}
static void Throws<T>(Action action) where T : Exception
{
    try { action(); } catch (T) { return; }
    throw new Exception($"Expected {typeof(T).Name}.");
}

using var first = new NativeRobinCore();
using var second = new NativeRobinCore(123);
Require(first.GetSnapshot() is { Mode: RobinMode.Sleeping, TimeMs: 0 }, "Initial snapshot/ABI layout");
Require(first.AdvanceTo(7100) is { Mode: RobinMode.Awake, Expression: RobinExpression.Curious, TimeMs: 7100 }, "Wake up");
Require(first.Dispatch(RobinEvent.HeartTouched, 7100).Expression == RobinExpression.Happy, "Heart reaction");
Require(second.GetSnapshot() is { Mode: RobinMode.Sleeping, TimeMs: 123 }, "Instance isolation");
Require(first.AdvanceTo(13100).Expression == RobinExpression.Curious, "Heart timeout");
Require(first.Dispatch(RobinEvent.HeadStarted, 13100) is { HeadContact: true, Expression: RobinExpression.Happy }, "Head contact");
Require(first.AdvanceTo(18100).Expression == RobinExpression.HeartEyes, "Five seconds of stroking");
Require(first.Dispatch(RobinEvent.HeadEnded, 18100) is { HeadContact: false, Expression: RobinExpression.Curious }, "Head released");
var before = first.GetSnapshot();
Throws<ArgumentOutOfRangeException>(() => first.AdvanceTo(18099));
Throws<ArgumentException>(() => first.Dispatch((RobinEvent)99, 18100));
Require(first.GetSnapshot() == before, "Rejected input changed state");
Require(first.Dispatch(RobinEvent.Restart, 19000).Mode == RobinMode.Sleeping, "Restart");
first.Dispose();
first.Dispose();
Throws<ObjectDisposedException>(() => first.GetSnapshot());
Throws<ObjectDisposedException>(() => first.Dispatch(RobinEvent.Restart, 0));
for (var i = 0; i < 1000; i++)
{
    using var core = new NativeRobinCore();
    core.AdvanceTo(7100);
}
Console.WriteLine("Native ABI, state changes, isolation, invalid inputs and lifetime passed.");

// Session time is server-owned and deterministic in tests.
var clock = new ManualClock();
using var sessions = new PinkRobin.Api.Sessions.RobinSessions(new TestGateway(), clock);
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
var limited = new PinkRobin.Api.Sessions.RobinSessions(new TestGateway(), new ManualClock());
using (limited)
{
    for (var i = 0; i < 128; i++) limited.Create(false);
    Throws<PinkRobin.Api.Sessions.SessionFailure>(() => limited.Create(false));
}
Console.WriteLine("Session isolation, sequence, clock, lease, expiry, reduced motion and capacity passed.");

sealed class ManualClock : TimeProvider
{
    private long ticks;
    public override long TimestampFrequency => 1000;
    public override long GetTimestamp() => ticks;
    public void Advance(long ms) => ticks += ms;
}
sealed class TestGateway : IRobinCoreGateway
{
    public PinkRobin.Api.Contracts.CoreConnectionStatus GetConnectionStatus() => new(true, "native-cabi-v1");
    public NativeRobinCore CreateInstance(ulong nowMs = 0) => new(nowMs);
}
