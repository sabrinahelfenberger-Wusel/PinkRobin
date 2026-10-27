using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;

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

using (var weatherCore = new NativeRobinCore())
{
    var forecast = new CoreWeatherForecast(
        TemperatureC: 8.5,
        PrecipitationMm: 12.0,
        WindSpeedKmh: 30.0,
        WindGustKmh: 70.0,
        ValidFromMs: 8000,
        ValidUntilMs: 18000);

    // Gültige Nachricht wird angenommen.
    weatherCore.PostWeather(forecast, CoreInputSource.Web,0);

    Throws<ArgumentException>(() =>
        weatherCore.PostWeather(
            forecast,
            (CoreInputSource)999,
            0));

    // Negativer Niederschlag wird vom Core abgewiesen.
    var invalid = forecast with { PrecipitationMm = -1.0 };

    Throws<ArgumentException>(() =>
        weatherCore.PostWeather(invalid, CoreInputSource.Web, 0));

    // Zeit bis zum Ende der Gültigkeit fortschalten.
    weatherCore.AdvanceTo(18000);

    // Dieselbe Prognose ist jetzt abgelaufen.
    Throws<ArgumentException>(() =>
        weatherCore.PostWeather(forecast, CoreInputSource.Web, 18000));
}

Console.WriteLine(
    "Wetterübergabe und native Validierung erfolgreich geprüft.");

using (var core = new NativeRobinCore())
{
    core.AdvanceTo(8000);

    core.StartTransformation(CoreTransformation.Octopus, 8000);

    if (core.GetTransformation() != CoreTransformation.Octopus)
    {
        throw new Exception("Octopus wurde nicht gestartet.");
    }

    core.AdvanceTo(17999);

    if (core.GetTransformation() != CoreTransformation.Octopus)
    {
        throw new Exception("Octopus wurde zu früh beendet.");
    }

    core.AdvanceTo(18000);

    if (core.GetTransformation() != CoreTransformation.None)
    {
        throw new Exception("Octopus wurde nicht beendet.");
    }
}

Console.WriteLine("Verwandlung: Start und Ablauf erfolgreich geprüft.");

var weatherClock = new ManualClock();

using (var weatherSessions = new PinkRobin.Api.Sessions.RobinSessions(
           new TestGateway(),
           weatherClock))
{
    var robinA = weatherSessions.Create(false);
    var robinB = weatherSessions.Create(false);

    // Beide Robins vollständig aufwecken.
    weatherClock.Advance(8000);

    var utcNow = weatherClock.GetUtcNow();

    var forecast = new WeatherSnapshot(
        Source: "MeteoSwiss",
        PostalCode: "8001",
        PointId: "800100",
        ForecastRunAt: utcNow,
        RetrievedAt: utcNow,
        ValidFrom: utcNow.AddSeconds(2),
        ValidTo: utcNow.AddSeconds(20),
        TemperatureC: 8.5,
        PrecipitationMm: 12.0,
        WindSpeedKmh: 30.0,
        WindGustKmh: 70.0);

    Require(
        weatherSessions.UpdateWeather(robinA.Token, forecast),
        "Weather message accepted");

    // Die Prognose beginnt erst in zwei Sekunden.
    weatherClock.Advance(1999);

    Require(
        weatherSessions.Read(robinA.Token).Transformation == "none",
        "Future forecast must not trigger early");

    weatherClock.Advance(1);

    Require(
        weatherSessions.Read(robinA.Token).Transformation == "octopus",
        "Weather reaction starts at forecast validity");

    Require(
        weatherSessions.Read(robinB.Token).Transformation == "none",
        "Weather must remain isolated per session");

    // Nach genau zehn Sekunden endet die Verwandlung.
    weatherClock.Advance(10000);

    Require(
        weatherSessions.Read(robinA.Token).Transformation == "none",
        "Weather transformation ends after ten seconds");

    // Jetzt endet auch die Gültigkeit der Prognose.
    weatherClock.Advance(8000);

    Require(
        !weatherSessions.UpdateWeather(robinA.Token, forecast),
        "Expired forecast must be rejected");
}

Console.WriteLine(
    "Session weather, UTC conversion, isolation and expiry passed.");

sealed class ManualClock : TimeProvider
{
    private long ticks;

    private readonly DateTimeOffset initialUtc =
        new(2026, 1, 1, 12, 0, 0, TimeSpan.Zero);

    public override long TimestampFrequency => 1000;

    public override long GetTimestamp() => ticks;

    public override DateTimeOffset GetUtcNow() =>
        initialUtc.AddMilliseconds(ticks);

    public void Advance(long ms)
    {
        if (ms < 0)
        {
            throw new ArgumentOutOfRangeException(nameof(ms));
        }

        ticks = checked(ticks + ms);
    }
}
sealed class TestGateway : IRobinCoreGateway
{
    public PinkRobin.Api.Contracts.CoreConnectionStatus GetConnectionStatus() => new(true, "native-cabi-v1");
    public NativeRobinCore CreateInstance(ulong nowMs = 0) => new(nowMs);
}
