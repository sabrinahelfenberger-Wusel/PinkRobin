namespace PinkRobin.Api.Weather;

public interface IWeatherSource
{
    Task<WeatherSnapshot> GetAsync(
        string? postalCode,
        CancellationToken cancellationToken);
}