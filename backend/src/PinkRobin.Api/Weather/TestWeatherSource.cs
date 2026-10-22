using Microsoft.Extensions.Options;

namespace PinkRobin.Api.Weather;

public sealed class TestWeatherSource(
    TimeProvider clock,
    IOptions<WeatherOptions> options) : IWeatherSource
{
    public Task<WeatherSnapshot> GetAsync(
        string? postalCode,
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();

        var now = clock.GetUtcNow();

        var (precipitationMm, windSpeedKmh, windGustKmh) =
            options.Value.TestScenario switch
            {
                "Dry" => (0.0, 5.0, 10.0),
                "Rain" => (2.0, 10.0, 20.0),
                "Storm" => (12.0, 30.0, 70.0),

                _ => throw new InvalidOperationException(
                    $"Unknown test weather scenario: {options.Value.TestScenario}")
            };

        var forecast = new WeatherSnapshot(
            Source: "Test",
            PostalCode: postalCode ?? options.Value.PostalCode,
            PointId: "test-point",
            ForecastRunAt: now,
            RetrievedAt: now,
            ValidFrom: now,
            ValidTo: now.AddHours(1),
            TemperatureC: 8.5,
            PrecipitationMm: precipitationMm,
            WindSpeedKmh: windSpeedKmh,
            WindGustKmh: windGustKmh
            );

        return Task.FromResult(forecast);
    }

    public string Provider { get; set; } = "MeteoSwiss";
}