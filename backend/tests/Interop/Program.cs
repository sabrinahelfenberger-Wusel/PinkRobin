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
