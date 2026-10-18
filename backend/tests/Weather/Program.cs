using System.Net;
using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.Extensions.Options;
using PinkRobin.Api.Weather;

static void Check(bool success, string message)
{
    if (!success) throw new Exception(message);
}
var clock = new TestClock();
var handler = new ForecastHandler();
var service = new WeatherService(new Factory(handler), Options.Create(new WeatherOptions { PostalCode = "8001" }),
    clock, NullLogger<WeatherService>.Instance);
var requests = await Task.WhenAll(Enumerable.Range(0, 8).Select(_ => service.GetAsync(default)));
Check(handler.Count == 6, "Concurrent cache misses must trigger only one refresh");
Check(requests.All(x => x.PointId == "42" && x.TemperatureC == 8.5 && x.PrecipitationMm == 1.2), "Point type, decimal parsing and parameter mapping");
Check(requests[0].ValidFrom == clock.Now.AddMinutes(-30), "Timestamp is the interval end");
var other = await service.GetAsync("3011", default);
Check(other.PointId == "43" && other.PostalCode == "3011" && handler.Count == 6, "A second location must share the national cache");
try { await service.GetAsync("9999", default); throw new Exception("Expected unknown location"); }
catch (WeatherUnavailableException) { }
Check(handler.Count == 6, "Unknown locations must not redownload forecasts");
clock.Now = clock.Now.AddMinutes(40);
var next = await service.GetAsync(default);
Check(next.TemperatureC == 9.5 && handler.Count == 6, "Select next interval from cached series");
clock.Now = clock.Now.AddMinutes(21);
await service.GetAsync(default);
Check(handler.Count == 12, "Refresh after one hour");
clock.Now = clock.Now.AddHours(1);
handler.Fail = true;
try { await service.GetAsync(default); throw new Exception("Expected unavailable"); }
catch (WeatherUnavailableException) { }
var failedCount = handler.Count;
try { await service.GetAsync(default); throw new Exception("Expected unavailable"); }
catch (WeatherUnavailableException) { }
Check(handler.Count == failedCount, "Failures have a retry cooldown");
foreach (var (postalCode, hours) in new[] { ("9999", 0), ("8001", 8) })
{
    var invalidClock = new TestClock();
    invalidClock.Now = invalidClock.Now.AddHours(hours);
    var unavailable = new WeatherService(new Factory(new ForecastHandler()),
        Options.Create(new WeatherOptions { PostalCode = postalCode }), invalidClock,
        NullLogger<WeatherService>.Instance);
    try { await unavailable.GetAsync(default); throw new Exception("Expected unavailable location or stale run"); }
    catch (WeatherUnavailableException) { }
}
var parsed = await WeatherService.ReadParameterAsync(new StringReader("point_id;point_type_id;Date;tre200h0\n42;2;202610181300;\n42;2;202610181400;NaN\n"), "42", "tre200h0", default);
Check(parsed.Count == 0, "Missing/nonfinite values must not become zero");
try
{
    await WeatherService.ReadParameterAsync(new StringReader("invalid"), "42", "tre200h0", default);
    throw new Exception("Expected invalid CSV");
}
catch (FormatException) { }
Console.WriteLine("Weather checks passed: concurrency, cache expiry, hour selection, units, point types, missing data and upstream failure.");

sealed class TestClock : TimeProvider
{
    public DateTimeOffset Now = new(2026, 10, 18, 12, 30, 0, TimeSpan.Zero);
    public override DateTimeOffset GetUtcNow() => Now;
}
sealed class Factory(ForecastHandler handler) : IHttpClientFactory
{
    public HttpClient CreateClient(string name) => new(handler, disposeHandler: false);
}
sealed class ForecastHandler : HttpMessageHandler
{
    public int Count;
    public bool Fail;
    protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken ct)
    {
        Interlocked.Increment(ref Count);
        if (Fail) return Task.FromResult(new HttpResponseMessage(HttpStatusCode.ServiceUnavailable));
        var path = request.RequestUri!.AbsolutePath;
        string content;
        if (path.EndsWith("meta_point.csv"))
            content = "point_id;point_type_id;postal_code\n42;1;8001\n42;2;8001\n43;2;3011\n";
        else if (path.EndsWith("/items"))
        {
            var assets = new[] { "tre200h0", "rre150h0", "fu3010h0", "fu3010h1" }
                .ToDictionary(p => $"vnut12.lssw.202610181200.{p}.csv", p => new { href = $"https://data.geo.admin.ch/{p}.csv" });
            content = JsonSerializer.Serialize(new { features = new[] { new { assets } }, links = Array.Empty<object>() });
        }
        else
        {
            var parameter = Path.GetFileNameWithoutExtension(path);
            var value = parameter switch { "tre200h0" => "8.5", "rre150h0" => "1.2", _ => "15.0" };
            content = $"point_id;point_type_id;Date;{parameter}\n42;1;202610181300;999\n42;2;202610181300;{value}\n42;2;202610181400;9.5\n42;2;202610181500;10.5\n43;2;202610181300;4.2\n43;2;202610181400;5.2\n43;2;202610181500;6.2\n";
        }
        return Task.FromResult(new HttpResponseMessage(HttpStatusCode.OK)
        { Content = new StringContent(content, Encoding.Latin1) });
    }
}
