using PinkRobin.Api.Contracts;
using PinkRobin.Api.Core;
using PinkRobin.Api.Sessions;
using System.Threading.RateLimiting;

using PinkRobin.Api.Weather;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddProblemDetails();
builder.Services.AddSingleton(TimeProvider.System);
builder.Services.AddSingleton<RobinSessions>();
builder.Services.AddHostedService<SessionCleanup>();
builder.Services.AddRateLimiter(options =>
{
    options.RejectionStatusCode = 429;
    options.AddPolicy("session-create", context => RateLimitPartition.GetFixedWindowLimiter(
        context.Connection.RemoteIpAddress?.ToString() ?? "unknown", _ => new FixedWindowRateLimiterOptions
        { PermitLimit = 30, Window = TimeSpan.FromMinutes(1), QueueLimit = 0 }));
});
builder.Services.AddSingleton<IRobinCoreGateway, NativeRobinCoreGateway>();

builder.Services.AddOptions<WeatherOptions>().BindConfiguration("Weather")
    .Validate(o => o.PostalCode.Length == 4 && o.PostalCode.All(char.IsAsciiDigit), "Weather:PostalCode must be a four-digit Swiss postal code.")
    .ValidateOnStart();
builder.Services.AddHttpClient("MeteoSwiss", client => client.Timeout = TimeSpan.FromSeconds(90));
builder.Services.AddSingleton<WeatherService>();

var origins = builder.Configuration.GetSection("Cors:AllowedOrigins").Get<string[]>() ?? [];
if (origins.Any(origin => origin.Contains('*') ||
    !Uri.TryCreate(origin, UriKind.Absolute, out var uri) ||
    (uri.Scheme != "https" && uri.Scheme != "http") ||
    uri.AbsolutePath != "/" || !string.IsNullOrEmpty(uri.Query) || !string.IsNullOrEmpty(uri.Fragment)))
{
    throw new InvalidOperationException("Cors:AllowedOrigins must contain explicit HTTP(S) origins without paths or wildcards.");
}
if (origins.Length > 0)
{
    builder.Services.AddCors(options => options.AddDefaultPolicy(policy =>
        policy.WithOrigins(origins).WithMethods("GET", "POST", "DELETE").WithHeaders("Content-Type", "X-Robin-Session")));
}

var app = builder.Build();
app.UseExceptionHandler();
app.UseStatusCodePages();
if (origins.Length > 0) app.UseCors();
app.UseRateLimiter();
app.Use(async (context, next) =>
{
    context.Response.Headers.CacheControl = "no-store";
    await next(context);
});

// Liveness is separate from readiness for Core-dependent work.
app.MapGet("/health/live", () => Results.Ok(new { status = "alive" }));
app.MapGet("/health/ready", IResult (IRobinCoreGateway core) =>
    core.GetConnectionStatus().Connected
        ? Results.Ok(new { status = "ready" })
        : Results.Problem(statusCode: StatusCodes.Status503ServiceUnavailable,
            title: "Core not connected", detail: "The native Robin Core could not be loaded. Check the backend logs."));

app.MapGet("/api/v1/status", (IRobinCoreGateway core) =>
    new BackendStatus("PinkRobin.Api", "1.0", "preparation", core.GetConnectionStatus(),
        SessionsAvailable: core.GetConnectionStatus().Connected, CommandsAvailable: core.GetConnectionStatus().Connected, RealtimeAvailable: false));

// Session tokens never appear in URLs or access logs. JSON/custom headers prevent
// cross-origin form submissions; CORS only permits configured development origins.
static IResult WithSession(HttpContext context, Func<string, IResult> action)
{
    var token = context.Request.Headers["X-Robin-Session"].ToString();
    if (token.Length != 64 || !token.All(Uri.IsHexDigit)) return Results.Problem(statusCode: 401, title: "Session token required");
    try { return action(token); }
    catch (SessionFailure error) { return Results.Problem(statusCode: error.Status, title: error.Message); }
}
app.MapPost("/api/v1/sessions", IResult (CreateSessionRequest request, RobinSessions sessions) =>
{
    try { return Results.Json(sessions.Create(request.ReducedMotion), statusCode: 201); }
    catch (SessionFailure error) { return Results.Problem(statusCode: error.Status, title: error.Message); }
}).RequireRateLimiting("session-create");
app.MapGet("/api/v1/session", (HttpContext context, RobinSessions sessions) =>
    WithSession(context, token => Results.Ok(sessions.Read(token))));
app.MapPost("/api/v1/session/events", (HttpContext context, RobinCommand command, RobinSessions sessions) =>
    WithSession(context, token => Results.Ok(sessions.Send(token, command))));
app.MapDelete("/api/v1/session", (HttpContext context, RobinSessions sessions) =>
    WithSession(context, token => { sessions.Remove(token); return Results.NoContent(); }));

app.MapGet("/api/weather", async Task<IResult> (string? postalCode, WeatherService weather, CancellationToken ct) =>
{
    if (postalCode is not null && (postalCode.Length != 4 || !postalCode.All(char.IsAsciiDigit)))
        return Results.Problem(statusCode: 400, title: "A four-digit Swiss postal code is required.");
    try { return Results.Ok(await weather.GetAsync(postalCode, ct)); }
    catch (WeatherUnavailableException)
    {
        return Results.Problem(statusCode: 503, title: "Weather unavailable",
            detail: "No current MeteoSwiss forecast is available. Robin should skip weather reactions.");
    }
});

app.Run();
