using System.Security.Cryptography;
using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;

namespace PinkRobin.Api.Sessions;

public sealed record CreateSessionRequest(bool ReducedMotion = false);
public sealed record RobinCommand(long Sequence, string Event);
public sealed record SessionState(
    string Mode,
    string Mood,
    string Expression,
    bool HeadContact,
    ulong TimeMs,
    ulong Revision,
    long Sequence,
    string Transformation);

public sealed record CreatedSession(string Token, SessionState State);
public sealed class SessionFailure(int status, string message) : Exception(message)
{
    public int Status { get; } = status;
}

// Tokens are bearer credentials, held only in browser memory. All native access and
// timestamps are serialized here; no native handle is shared between sessions.
public sealed class RobinSessions(IRobinCoreGateway gateway, TimeProvider clock) : IDisposable
{
    private sealed class Session(NativeRobinCore core, long started, bool reduced)
    {
        public NativeRobinCore Core { get; } = core;
        public long Started { get; } = started;
        public bool Reduced { get; } = reduced;
        public TimeSpan LastSeen;
        public TimeSpan LastContact;
        public ulong Offset;
        public long Sequence;
        public string? LastEvent;
        public int Commands;
        public long CommandSecond = -1;
    }
    private readonly object sync = new();
    private readonly Dictionary<string, Session> sessions = new(StringComparer.Ordinal);
    private static readonly TimeSpan IdleLimit = TimeSpan.FromMinutes(2);
    private static readonly TimeSpan AgeLimit = TimeSpan.FromMinutes(30);

    public CreatedSession Create(bool reduced)
    {
        lock (sync)
        {
            SweepLocked();
            if (!gateway.GetConnectionStatus().Connected) throw new SessionFailure(503, "Core unavailable");
            if (sessions.Count >= 128) throw new SessionFailure(503, "Session capacity reached");
            var session = new Session(gateway.CreateInstance(), clock.GetTimestamp(), reduced);
            try
            {
                if (reduced) session.Offset = 7100;
                session.Core.AdvanceTo(session.Offset);
                var token = Convert.ToHexString(RandomNumberGenerator.GetBytes(32));
                sessions.Add(token, session);
                return new(token, State(session));
            }
            catch { session.Core.Dispose(); throw; }
        }
    }

    public SessionState Read(string token) => Execute(token, null);
    public SessionState Send(string token, RobinCommand command) => Execute(token, command);

    private static CoreWeatherForecast? ToCoreWeather(
        WeatherSnapshot weather,
        DateTimeOffset utcNow,
        ulong coreNowMs)
    {
        if (weather.ValidFrom >= weather.ValidTo ||
            weather.ValidTo <= utcNow)
        {
            return null;
        }

        var remainingMs = Math.Floor(
            (weather.ValidTo - utcNow).TotalMilliseconds);

        if (remainingMs < 1)
        {
            return null;
        }

        var untilStartMs = Math.Max(
            0,
            Math.Ceiling(
                (weather.ValidFrom - utcNow).TotalMilliseconds));

        return new CoreWeatherForecast(
            weather.TemperatureC,
            weather.PrecipitationMm,
            weather.WindSpeedKmh,
            weather.WindGustKmh,
            checked(coreNowMs + (ulong)untilStartMs),
            checked(coreNowMs + (ulong)remainingMs));
    }

    public bool UpdateWeather(
        string token,
        WeatherSnapshot weather)
    {
        lock (sync)
        {
            SweepLocked();

            if (!sessions.TryGetValue(token, out var session))
            {
                throw new SessionFailure(404, "Session expired or unknown");
            }

            var currentState = Execute(token, null);

            var forecast = ToCoreWeather(
                weather,
                clock.GetUtcNow(),
                currentState.TimeMs);

            if (forecast is null)
            {
                return false;
            }

            session.Core.PostWeather(forecast, CoreInputSource.Web, currentState.TimeMs);

            return true;
        }
    }

    private SessionState Execute(string token, RobinCommand? command)
    {
        lock (sync)
        {
            SweepLocked();
            if (!sessions.TryGetValue(token, out var session)) throw new SessionFailure(404, "Session expired or unknown");
            var elapsed = clock.GetElapsedTime(session.Started);
            session.LastSeen = elapsed;
            var now = (ulong)elapsed.TotalMilliseconds + session.Offset;
            // A lost connection must not leave Robin being stroked indefinitely.
            if (session.Core.GetSnapshot().HeadContact && elapsed - session.LastContact > TimeSpan.FromSeconds(2))
                session.Core.Dispatch(RobinEvent.HeadEnded, now);
            session.Core.AdvanceTo(now);
            if (command is not null)
            {
                if (command.Sequence == session.Sequence && command.Event == session.LastEvent)
                    return State(session); // Retry: return current state without replaying the event.
                if (command.Sequence != session.Sequence + 1)
                    throw new SessionFailure(409, "Unexpected command sequence");
                var input = command.Event switch
                {
                    "restart" => RobinEvent.Restart,
                    "heartTouched" => RobinEvent.HeartTouched,
                    "headStarted" => RobinEvent.HeadStarted,
                    "headEnded" => RobinEvent.HeadEnded,
                    "interactionCancelled" => RobinEvent.InteractionCancelled,
                    _ => throw new SessionFailure(400, "Unknown Robin event")
                };
                var second = (long)elapsed.TotalSeconds;
                if (second != session.CommandSecond) { session.CommandSecond = second; session.Commands = 0; }
                if (++session.Commands > 30) throw new SessionFailure(429, "Too many commands");
                session.Core.Dispatch(input, now);
                if (input == RobinEvent.Restart && session.Reduced)
                {
                    session.Offset += 7100;
                    session.Core.AdvanceTo(now + 7100);
                }
                session.Sequence = command.Sequence;
                session.LastEvent = command.Event;
            }
            session.LastContact = elapsed;
            return State(session);
        }
    }

    private static SessionState State(Session session)
    {
        var s = session.Core.GetSnapshot();

        return new(
            s.Mode.ToString().ToLowerInvariant(),
            s.Mood.ToString().ToLowerInvariant(),
            s.Expression == RobinExpression.HeartEyes
                ? "heartEyes" 
                : s.Expression.ToString().ToLowerInvariant(),
            s.HeadContact,
            s.TimeMs,
            s.Revision,
            session.Sequence,
            session.Core.GetTransformation().ToString().ToLowerInvariant());
    }

    public void Remove(string token)
    {
        lock (sync) { if (sessions.Remove(token, out var session)) session.Core.Dispose(); }
    }
    public void Sweep() { lock (sync) SweepLocked(); }
    private void SweepLocked()
    {
        foreach (var pair in sessions.ToArray())
        {
            var elapsed = clock.GetElapsedTime(pair.Value.Started);
            if (elapsed >= AgeLimit || elapsed - pair.Value.LastSeen >= IdleLimit)
            {
                pair.Value.Core.Dispose();
                sessions.Remove(pair.Key);
            }
            else if (pair.Value.Core.GetSnapshot().HeadContact && elapsed - pair.Value.LastContact > TimeSpan.FromSeconds(2))
                pair.Value.Core.Dispatch(RobinEvent.HeadEnded, (ulong)elapsed.TotalMilliseconds + pair.Value.Offset);
        }
    }
    public void Dispose()
    {
        lock (sync) { foreach (var session in sessions.Values) session.Core.Dispose(); sessions.Clear(); }
    }
}

public sealed class SessionCleanup(RobinSessions sessions) : BackgroundService
{
    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        using var timer = new PeriodicTimer(TimeSpan.FromSeconds(1));
        try { while (await timer.WaitForNextTickAsync(stoppingToken)) sessions.Sweep(); }
        catch (OperationCanceledException) when (stoppingToken.IsCancellationRequested) { }
    }
}
