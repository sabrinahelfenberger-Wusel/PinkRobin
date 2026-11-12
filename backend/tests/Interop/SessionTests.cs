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
        clock.Advance(900000);
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

    public static void LifetimePolicy()
    {
        var clock = new ManualClock();

        using var sessions = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            clock);

        var paused = sessions.Create(false);

        // Eine Hintergrundpause unter 15 Minuten erhält die Session.
        clock.Advance(899999);
        sessions.Sweep();

        Require(
            sessions.Read(paused.Token).Mode == "awake",
            "Session survives a pause shorter than 15 minutes");

        // Lesen erneuert die Inaktivitätsfrist.
        clock.Advance(899999);
        sessions.Sweep();

        Require(
            sessions.Read(paused.Token).Mode == "awake",
            "Successful read renews session lifetime");

        // Genau 15 Minuten ohne Anfrage beenden die Session.
        clock.Advance(900000);
        sessions.Sweep();

        Throws<PinkRobin.Api.Sessions.SessionFailure>(
            () => sessions.Read(paused.Token));

        var active = sessions.Create(false);

        for (var minute = 0; minute < 31; minute++)
        {
            clock.Advance(60000);
            sessions.Read(active.Token);
        }

        Require(
            sessions.Read(active.Token).Mode == "awake",
            "Active session survives more than 30 minutes");

        var customClock = new ManualClock();

        var options = Microsoft.Extensions.Options.Options.Create(
            new PinkRobin.Api.Sessions.RobinSessionOptions
            {
                IdleTimeout = TimeSpan.FromSeconds(3)
            });

        using var custom = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            customClock,
            options);

        var shortSession = custom.Create(false);

        customClock.Advance(3000);
        custom.Sweep();

        Throws<PinkRobin.Api.Sessions.SessionFailure>(
            () => custom.Read(shortSession.Token));

        Console.WriteLine(
            "Session pause, renewal, expiry and configured timeout passed.");
    }

    public static async Task WeatherRequiresExistingSession()
    {
        var clock = new ManualClock();
        var weather = new ControlledWeatherSource();

        using var sessions = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            clock);

        var removed = sessions.Create(false);
        sessions.Remove(removed.Token);

        var expired = sessions.Create(false);
        clock.Advance(900000);
        sessions.Sweep();

        foreach (var token in new[]
        {
            new string('0', 64),
            removed.Token,
            expired.Token
        })
        {
            var attempt = sessions.UpdateWeatherAsync(
                token,
                "9000",
                weather,
                CancellationToken.None);

            Require(
                weather.Calls == 0,
                "Rejected session must not invoke the weather source");

            try
            {
                await attempt;

                throw new InvalidOperationException(
                    "Weather update unexpectedly accepted a rejected session.");
            }
            catch (PinkRobin.Api.Sessions.SessionFailure error)
            {
                Require(
                    error.Status == 404,
                    "Unknown, removed and expired sessions return 404");
            }
        }

        // Positive Kontrolle: Eine gültige Session erreicht die Quelle.
        var valid = sessions.Create(false);
        var now = clock.GetUtcNow();

        weather.Completion.SetResult(new WeatherSnapshot(
            Source: "Test",
            PostalCode: "9000",
            PointId: "900000",
            ForecastRunAt: now,
            RetrievedAt: now,
            ValidFrom: now,
            ValidTo: now.AddMinutes(10),
            TemperatureC: 8.5,
            PrecipitationMm: 2.0,
            WindSpeedKmh: 10.0,
            WindGustKmh: 20.0));

        var accepted = await sessions.UpdateWeatherAsync(
            valid.Token,
            "9000",
            weather,
            CancellationToken.None);

        Require(accepted, "Valid session accepts weather");
        Require(weather.Calls == 1, "Valid session invokes weather once");

        Console.WriteLine(
            "Weather access rejects missing sessions before external work.");
    }
}
