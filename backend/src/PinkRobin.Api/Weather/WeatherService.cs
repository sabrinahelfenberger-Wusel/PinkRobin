using System.Globalization;
using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Options;
using Microsoft.Extensions.Logging;

namespace PinkRobin.Api.Weather;

public sealed class WeatherOptions
{
    public string PostalCode { get; set; } = "";
    public string? PointId { get; set; }
}

public sealed record WeatherSnapshot(string Source, string PostalCode, string PointId,
    DateTimeOffset ForecastRunAt, DateTimeOffset RetrievedAt, DateTimeOffset ValidFrom,
    DateTimeOffset ValidTo, double TemperatureC, double PrecipitationMm,
    double WindSpeedKmh, double WindGustKmh);

public sealed class WeatherUnavailableException(string message) : Exception(message);

// One configured location per backend; the singleton owns the cache and refresh lock.
public sealed class WeatherService(IHttpClientFactory clients, IOptions<WeatherOptions> options,
    TimeProvider clock, ILogger<WeatherService> logger)
{
    private const string Collection = "https://data.geo.admin.ch/api/stac/v1/collections/ch.meteoschweiz.ogd-local-forecasting";
    private const string Points = "https://data.geo.admin.ch/ch.meteoschweiz.ogd-local-forecasting/ogd-local-forecasting_meta_point.csv";
    private static readonly string[] Parameters = ["tre200h0", "rre150h0", "fu3010h0", "fu3010h1"];
    private readonly SemaphoreSlim gate = new(1, 1);
    private WeatherSnapshot[] cache = [];
    private DateTimeOffset expires;
    private DateTimeOffset retryAfter;

    public async Task<WeatherSnapshot> GetAsync(CancellationToken cancellationToken)
    {
        await gate.WaitAsync(cancellationToken);
        try
        {
            var now = clock.GetUtcNow();
            if (now >= expires)
            {
                if (now < retryAfter) throw new WeatherUnavailableException("Weather refresh temporarily unavailable.");
                using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
                timeout.CancelAfter(TimeSpan.FromSeconds(90));
                try
                {
                    cache = await RefreshAsync(timeout.Token);
                    expires = clock.GetUtcNow().AddHours(1);
                }
                catch (Exception error) when (error is HttpRequestException or IOException or JsonException or
                    FormatException or InvalidOperationException or KeyNotFoundException or WeatherUnavailableException or OperationCanceledException or
                    Microsoft.VisualBasic.FileIO.MalformedLineException)
                {
                    if (cancellationToken.IsCancellationRequested) throw;
                    retryAfter = clock.GetUtcNow().AddMinutes(1);
                    logger.LogWarning(error, "MeteoSwiss weather refresh failed");
                    throw new WeatherUnavailableException("MeteoSwiss forecast temporarily unavailable.");
                }
            }
            now = clock.GetUtcNow();
            return cache.FirstOrDefault(x => x.ValidFrom <= now && now < x.ValidTo)
                ?? throw new WeatherUnavailableException("No forecast covers the current hour.");
        }
        finally { gate.Release(); }
    }

    private async Task<WeatherSnapshot[]> RefreshAsync(CancellationToken ct)
    {
        using var client = clients.CreateClient("MeteoSwiss");
        string pointId;
        using (var reader = await OpenCsvAsync(client, Points, ct))
        {
            using var csv = new Microsoft.VisualBasic.FileIO.TextFieldParser(reader);
            csv.SetDelimiters(";");
            var header = csv.ReadFields() ?? throw new FormatException("Missing point header");
            foreach (var name in new[] { "point_id", "point_type_id", "postal_code" })
                if (!header.Contains(name)) throw new FormatException("Unexpected point CSV header");
            var matches = new List<string>();
            while (!csv.EndOfData)
            {
                ct.ThrowIfCancellationRequested();
                var row = csv.ReadFields()!;
                if (row.Length != header.Length) throw new FormatException("Unexpected point CSV row");
                string Field(string name) => row[Array.IndexOf(header, name)];
                if (Field("point_type_id") == "2" && Field("postal_code") == options.Value.PostalCode)
                    matches.Add(Field("point_id"));
            }
            var selected = matches.Where(id => options.Value.PointId is null || id == options.Value.PointId).Distinct().ToArray();
            if (selected.Length != 1) throw new WeatherUnavailableException("Postal code missing or ambiguous; configure Weather:PointId if necessary.");
            pointId = selected[0];
        }

        var runs = new Dictionary<string, Dictionary<string, string>>();
        string? next = Collection + "/items?limit=100";
        var visited = new HashSet<string>();
        while (next is not null)
        {
            if (!visited.Add(next)) throw new FormatException("STAC pagination cycle");
            using var response = await client.GetAsync(Trusted(next), ct);
            response.EnsureSuccessStatusCode();
            using var document = await JsonDocument.ParseAsync(await response.Content.ReadAsStreamAsync(ct), cancellationToken: ct);
            foreach (var feature in document.RootElement.GetProperty("features").EnumerateArray())
            foreach (var asset in feature.GetProperty("assets").EnumerateObject())
            {
                var parts = asset.Name.Split('.');
                if (parts.Length != 5 || !Parameters.Contains(parts[3]) || parts[4] != "csv") continue;
                if (!DateTimeOffset.TryParseExact(parts[2], "yyyyMMddHHmm", CultureInfo.InvariantCulture,
                    DateTimeStyles.AssumeUniversal, out _)) continue;
                if (!runs.TryGetValue(parts[2], out var parameters)) runs[parts[2]] = parameters = [];
                parameters[parts[3]] = asset.Value.GetProperty("href").GetString()!;
            }
            next = document.RootElement.GetProperty("links").EnumerateArray()
                .Where(link => link.GetProperty("rel").GetString() == "next")
                .Select(link => link.GetProperty("href").GetString()).FirstOrDefault();
        }
        var run = runs.Where(x => Parameters.All(x.Value.ContainsKey)).OrderByDescending(x => x.Key).FirstOrDefault();
        if (run.Value is null) throw new WeatherUnavailableException("No complete forecast run in STAC.");
        var data = new Dictionary<string, Dictionary<DateTimeOffset, double>>();
        foreach (var parameter in Parameters)
        {
            using var reader = await OpenCsvAsync(client, run.Value[parameter], ct);
            data[parameter] = await ReadParameterAsync(reader, pointId, parameter, ct);
        }
        var retrieved = clock.GetUtcNow();
        var issued = DateTimeOffset.ParseExact(run.Key, "yyyyMMddHHmm", CultureInfo.InvariantCulture, DateTimeStyles.AssumeUniversal);
        if (issued > retrieved || retrieved - issued > TimeSpan.FromHours(6))
            throw new WeatherUnavailableException("Forecast run is outdated.");
        return data[Parameters[0]].Keys.Order().Where(time => Parameters.All(p => data[p].ContainsKey(time)))
            .Select(time => new WeatherSnapshot("MeteoSwiss", options.Value.PostalCode, pointId, issued,
                retrieved, time.AddHours(-1), time, data[Parameters[0]][time], data[Parameters[1]][time],
                data[Parameters[2]][time], data[Parameters[3]][time])).ToArray();
    }

    internal static async Task<Dictionary<DateTimeOffset, double>> ReadParameterAsync(
        TextReader reader, string pointId, string parameter, CancellationToken ct)
    {
        var header = (await reader.ReadLineAsync(ct))?.TrimStart('\uFEFF').Split(';');
        if (header is null || !header.SequenceEqual(new[] { "point_id", "point_type_id", "Date", parameter }))
            throw new FormatException("Unexpected forecast CSV header");
        var values = new Dictionary<DateTimeOffset, double>();
        while (await reader.ReadLineAsync(ct) is { } line)
        {
            var row = line.Split(';');
            if (row.Length != 4) throw new FormatException("Unexpected forecast CSV row");
            if (row[0] != pointId || row[1] != "2") continue;
            var time = DateTimeOffset.ParseExact(row[2], "yyyyMMddHHmm", CultureInfo.InvariantCulture, DateTimeStyles.AssumeUniversal);
            if (double.TryParse(row[3], NumberStyles.Float, CultureInfo.InvariantCulture, out var value) && double.IsFinite(value))
                values[time] = value;
        }
        return values;
    }

    private static Uri Trusted(string url)
    {
        var uri = new Uri(url);
        if (uri.Scheme != "https" || uri.Host != "data.geo.admin.ch" || !uri.IsDefaultPort)
            throw new FormatException("Unexpected STAC URL");
        return uri;
    }

    private static async Task<StreamReader> OpenCsvAsync(HttpClient client, string url, CancellationToken ct) =>
        new(await client.GetStreamAsync(Trusted(url), ct), Encoding.Latin1);
}
