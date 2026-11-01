using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;
using static TestSupport;

internal static class NativeCoreTests
{
    public static void StateAndLifetime()
    {
        using var first = new NativeRobinCore();
        using var second = new NativeRobinCore(123);
        Require(first.GetSnapshot() is { Mode: RobinMode.Sleeping, TimeMs: 0 }, "Initial snapshot/ABI layout");
        Require(first.AdvanceTo(7100) is { Mode: RobinMode.Awake, Expression: RobinExpression.Curious, TimeMs: 7100 }, "Wake up");
        Require(first.Dispatch(RobinEvent.HeartTouched, CoreInputSource.Web, 7100).Expression == RobinExpression.Happy, "Heart reaction");
        Require(second.GetSnapshot() is { Mode: RobinMode.Sleeping, TimeMs: 123 }, "Instance isolation");
        Require(first.AdvanceTo(13100).Expression == RobinExpression.Curious, "Heart timeout");
        Require(first.Dispatch(RobinEvent.HeadStarted, CoreInputSource.Web, 13100) is { HeadContact: true, Expression: RobinExpression.Happy }, "Head contact");
        Require(first.AdvanceTo(18100).Expression == RobinExpression.HeartEyes, "Five seconds of stroking");
        Require(first.Dispatch(RobinEvent.HeadEnded, CoreInputSource.Web, 18100) is { HeadContact: false, Expression: RobinExpression.Curious }, "Head released");
        var before = first.GetSnapshot();
        Throws<ArgumentOutOfRangeException>(() => first.AdvanceTo(18099));
        Throws<ArgumentException>(() =>
            first.Dispatch(
                RobinEvent.HeadStarted,
                (CoreInputSource)999,
                18100));
        Throws<ArgumentException>(() => first.Dispatch((RobinEvent)99, CoreInputSource.Web, 18100));
        Require(first.GetSnapshot() == before, "Rejected input changed state");
        Require(first.Dispatch(RobinEvent.Restart, CoreInputSource.Web, 19000).Mode == RobinMode.Sleeping, "Restart");
        first.Dispose();
        first.Dispose();
        Throws<ObjectDisposedException>(() => first.GetSnapshot());
        Throws<ObjectDisposedException>(() => first.Dispatch(RobinEvent.Restart, CoreInputSource.Web, 0));
        for (var i = 0; i < 1000; i++)
        {
            using var core = new NativeRobinCore();
            core.AdvanceTo(7100);
        }
        Console.WriteLine("Native ABI, state changes, isolation, invalid inputs and lifetime passed.");
    }

    public static void InteractionQueue()
    {
        using (var queuedCore = new NativeRobinCore())
        {
            for (ulong time = 1; time <= 8; ++time)
            {
                queuedCore.PostInteraction(
                    RobinEvent.HeadStarted,
                    CoreInputSource.Web,
                    time);
            }

            // Einreihen allein verändert den sichtbaren Zustand nicht.
            Require(
                queuedCore.GetSnapshot() is
                    { TimeMs: 0, HeadContact: false },
                "Queued interactions must not execute immediately");

            // Der neunte Eintrag passt nicht mehr hinein.
            Throws<CoreMessageQueueFullException>(() =>
                queuedCore.PostInteraction(
                    RobinEvent.HeadEnded,
                    CoreInputSource.Web,
                    9));

            // Dispatch muss den vollen Puffer verarbeiten und
            // das zunächst abgelehnte Loslassen erneut übergeben.
            var released = queuedCore.Dispatch(
                RobinEvent.HeadEnded,
                CoreInputSource.Web,
                9);

            Require(
                released is { TimeMs: 9, HeadContact: false } &&
                released.Expression == RobinExpression.Curious,
                "Dispatch must drain the full queue and retry head release");
        }

        Console.WriteLine(
            "Interaction queue capacity and dispatch retry passed.");
    }
}
