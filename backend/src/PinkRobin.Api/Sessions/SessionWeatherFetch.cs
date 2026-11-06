using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;

namespace PinkRobin.Api.Sessions;

internal sealed class SessionWeatherFetch(
    CoreWeatherFetchRequestedOutput request)
{
    public CoreWeatherFetchRequestedOutput Request { get; } = request;

    public bool Running { get; set; }
    public bool Completed { get; set; }

    public WeatherSnapshot? Forecast { get; set; }
}