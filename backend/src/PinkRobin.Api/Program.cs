using PinkRobin.Api.Contracts;
using PinkRobin.Api.Core;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddProblemDetails();
builder.Services.AddSingleton<IRobinCoreGateway, UnconnectedRobinCoreGateway>();

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
        policy.WithOrigins(origins).WithMethods("GET").WithHeaders("Content-Type")));
}

var app = builder.Build();
app.UseExceptionHandler();
app.UseStatusCodePages();
if (origins.Length > 0) app.UseCors();
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
            title: "Core not connected", detail: "The native Robin Core has not been integrated yet."));

app.MapGet("/api/v1/status", (IRobinCoreGateway core) =>
    new BackendStatus("PinkRobin.Api", "1.0", "preparation", core.GetConnectionStatus(),
        SessionsAvailable: false, CommandsAvailable: false, RealtimeAvailable: false));

app.Run();
