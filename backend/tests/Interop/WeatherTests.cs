using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;
using static TestSupport;

internal static class WeatherTests
{
    public static void Validation()
    {
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
    }

    public static void Transformation()
    {
        using (var core = new NativeRobinCore(
                   capabilities: new CorePlatformCapabilities(
                       TextOutput: false,
                       OutfitDisplay: true)))
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
    }

    public static void SessionValidityAndIsolation()
    {
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
    }
}
