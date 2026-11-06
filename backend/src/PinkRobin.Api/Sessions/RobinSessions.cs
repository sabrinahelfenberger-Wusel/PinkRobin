using System.Security.Cryptography;
using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;

namespace PinkRobin.Api.Sessions;

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
        public string? LastText;
        public int Commands;
        public long CommandSecond = -1;
        public SessionWeatherFetch? WeatherFetch;
        public bool ConversationWeatherSelected;
    }
    private readonly object sync = new();
    private readonly Dictionary<string, Session> sessions = new(StringComparer.Ordinal);
    private static readonly TimeSpan IdleLimit = TimeSpan.FromMinutes(2);
    private static readonly TimeSpan AgeLimit = TimeSpan.FromMinutes(30);

    private static readonly CorePlatformCapabilities WebCapabilities = new(
        TextOutput: true,
        OutfitDisplay: true);

    public CreatedSession Create(bool reduced)
    {
        lock (sync)
        {
            SweepLocked();
            if (!gateway.GetConnectionStatus().Connected) throw new SessionFailure(503, "Core unavailable");
            if (sessions.Count >= 128) throw new SessionFailure(503, "Session capacity reached");
            
            var session = new Session(
                gateway.CreateInstance(
                    capabilities: WebCapabilities),
                clock.GetTimestamp(),
                reduced);

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

    public SessionOutput? ReadOutput(string token)
    {
        lock (sync)
        {
            Execute(token, null);

            var output = sessions[token].Core.GetNextOutput();

            if (output is null)
            {
                return null;
            }

            var sequence = output.Sequence.ToString(
                System.Globalization.CultureInfo.InvariantCulture);

            return output switch
            {
                CoreWeatherAnswerOutput answer => new SessionOutput(
                    sequence,
                    "weatherAnswer",
                    WeatherAnswer: answer.Kind.ToString().ToLowerInvariant(),
                    Text: answer.Text),

                CoreConversationReplyOutput reply => new SessionOutput(
                    sequence,
                    "conversationReply",
                    ConversationReply: reply.Kind switch
                    {
                        CoreConversationReplyKind.UnknownInput =>
                            "unknownInput",
                        CoreConversationReplyKind.AskPostalCode =>
                            "askPostalCode",
                        CoreConversationReplyKind.InvalidPostalCode =>
                            "invalidPostalCode",
                        CoreConversationReplyKind.WeatherUnavailable =>
                            "weatherUnavailable",
                        CoreConversationReplyKind.Attention =>
                            "attention",
                        CoreConversationReplyKind.Greeting =>
                            "greeting",
                        CoreConversationReplyKind.Gratitude =>
                            "gratitude",
                        _ => throw new InvalidOperationException(
                            "Unknown conversation reply.")
                    },
                    Text: reply.Text),

                CoreWeatherFetchRequestedOutput request => new SessionOutput(
                    sequence,
                    "weatherFetchRequested",
                    RequestId: request.RequestId.ToString(
                        System.Globalization.CultureInfo.InvariantCulture),
                    PostalCode: request.PostalCode),

                _ => throw new InvalidOperationException(
                    "Unknown Core output.")
            };
        }
    }

    public void AcknowledgeOutput(string token, string sequence)
    {
        lock (sync)
        {
            Execute(token, null);

            if (!ulong.TryParse(
                    sequence,
                    System.Globalization.NumberStyles.None,
                    System.Globalization.CultureInfo.InvariantCulture,
                    out var parsedSequence) ||
                parsedSequence == 0)
            {
                throw new SessionFailure(
                    400,
                    "Invalid output sequence");
            }

            try
            {
                sessions[token].Core.AcknowledgeOutput(parsedSequence);
            }
            catch (ArgumentException)
            {
                throw new SessionFailure(
                    400,
                    "Output sequence cannot be acknowledged");
            }
        }
    }

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

            // Der Gesprächsort hat Vorrang vor der Standardversorgung.
            if (session.ConversationWeatherSelected)
            {
                return false;
            }

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

    private static SessionWeatherFetch? TakeWeatherFetch(Session session)
    {
        if (session.WeatherFetch is not null)
        {
            return session.WeatherFetch;
        }

        if (session.Core.GetNextOutput()
            is not CoreWeatherFetchRequestedOutput request)
        {
            return null;
        }

        var work = new SessionWeatherFetch(request);

        // Erst lokal übernehmen, danach den Core-Puffer freigeben.
        session.WeatherFetch = work;

        try
        {
            session.Core.AcknowledgeOutput(request.Sequence);
        }
        catch
        {
            session.WeatherFetch = null;
            throw;
        }

        session.ConversationWeatherSelected = true;

        return work;
    }

    private bool TryDeliverWeatherFetch(
        Session session,
        ulong nowMs)
    {
        var work = session.WeatherFetch;

        if (work is null || !work.Completed)
        {
            return false;
        }

        CoreWeatherForecast? forecast = null;

        if (work.Forecast is not null &&
            work.Forecast.PostalCode == work.Request.PostalCode)
        {
            forecast = ToCoreWeather(
                work.Forecast,
                clock.GetUtcNow(),
                nowMs);
        }

        try
        {
            session.Core.ReceiveWeatherResult(
                work.Request.RequestId,
                forecast,
                CoreInputSource.Backend,
                nowMs);
        }
        catch (CoreMessageQueueFullException)
        {
            // Das Ergebnis bleibt für den nächsten Versuch erhalten.
            return false;
        }

        session.WeatherFetch = null;
        return true;
    }

    private SessionOutput? ReadVisibleOutput(string token)
    {
        var output = ReadOutput(token);

        return output?.Kind == "weatherFetchRequested"
            ? null
            : output;
    }

    public async Task<SessionOutput?> ReadOutputAsync(
        string token,
        IWeatherSource weather,
        CancellationToken ct)
    {
        Session session;
        SessionWeatherFetch work;

        lock (sync)
        {
            var state = Execute(token, null);
            session = sessions[token];

            var pending = TakeWeatherFetch(session);

            if (pending is null)
            {
                return ReadVisibleOutput(token);
            }

            work = pending;

            if (work.Completed)
            {
                TryDeliverWeatherFetch(session, state.TimeMs);
                return ReadVisibleOutput(token);
            }

            if (work.Running)
            {
                return ReadVisibleOutput(token);
            }

            work.Running = true;
        }

        WeatherSnapshot? forecast;

        try
        {
            try
            {
                forecast = await weather.GetAsync(
                    work.Request.PostalCode,
                    ct);
            }
            catch (WeatherUnavailableException)
            {
                forecast = null;
            }
        }
        catch
        {
            lock (sync)
            {
                work.Running = false;
            }

            throw;
        }

        lock (sync)
        {
            work.Running = false;

            if (!sessions.TryGetValue(token, out var current) ||
                !ReferenceEquals(current, session))
            {
                throw new SessionFailure(
                    404,
                    "Session expired or unknown");
            }

            work.Forecast = forecast;
            work.Completed = true;

            var state = Execute(token, null);

            TryDeliverWeatherFetch(session, state.TimeMs);

            return ReadVisibleOutput(token);
        }
    }

    private SessionState Execute(string token, RobinCommand? command)
    {
        lock (sync)
        {
            SweepLocked();

            if (!sessions.TryGetValue(token, out var session))
            {
                throw new SessionFailure(404, "Session expired or unknown");
            }

            var elapsed = clock.GetElapsedTime(session.Started);
            session.LastSeen = elapsed;
            var now = (ulong)elapsed.TotalMilliseconds + session.Offset;

            // Eine verlorene Verbindung darf Streicheln nicht dauerhaft festhalten.
            if (session.Core.GetSnapshot().HeadContact &&
                elapsed - session.LastContact > TimeSpan.FromSeconds(2))
            {
                session.Core.Dispatch(
                    RobinEvent.HeadEnded,
                    CoreInputSource.Backend,
                    now);
            }

            session.Core.AdvanceTo(now);

            if (command is not null)
            {
                if (command.Sequence == session.Sequence &&
                    command.Event == session.LastEvent &&
                    command.Text == session.LastText)
                {
                    return State(session);
                }

                if (command.Sequence != session.Sequence + 1)
                {
                    throw new SessionFailure(409, "Unexpected command sequence");
                }

                if (command.Event == "textReceived")
                {
                    if (command.Text is null)
                    {
                        throw new SessionFailure(400, "Text is required");
                    }
                }
                else if (command.Text is not null)
                {
                    throw new SessionFailure(
                        400,
                        "Text is only allowed for textReceived");
                }

                RobinEvent? input = command.Event switch
                {
                    "weatherAsked" => null,
                    "restart" => RobinEvent.Restart,
                    "heartTouched" => RobinEvent.HeartTouched,
                    "headStarted" => RobinEvent.HeadStarted,
                    "headEnded" => RobinEvent.HeadEnded,
                    "textReceived" => null,
                    "interactionCancelled" => RobinEvent.InteractionCancelled,
                    _ => throw new SessionFailure(400, "Unknown Robin event")
                };

                var second = (long)elapsed.TotalSeconds;

                if (second != session.CommandSecond)
                {
                    session.CommandSecond = second;
                    session.Commands = 0;
                }

                if (++session.Commands > 30)
                {
                    throw new SessionFailure(429, "Too many commands");
                }

                if (input is RobinEvent interaction)
                {
                    session.Core.Dispatch(
                        interaction,
                        CoreInputSource.Web,
                        now);
                }
                else if (command.Event == "textReceived")
                {
                    try
                    {
                        session.Core.ReceiveText(
                            command.Text!,
                            CoreInputSource.Web,
                            now);
                    }
                    catch (ArgumentException)
                    {
                        throw new SessionFailure(400, "Invalid text input");
                    }
                    catch (CoreMessageQueueFullException)
                    {
                        throw new SessionFailure(
                            503,
                            "Core input queue is full");
                    }
                }
                else
                {
                    try
                    {
                        session.Core.RequestWeatherAnswer(now);
                    }
                    catch (CoreOutputQueueFullException)
                    {
                        throw new SessionFailure(
                            503,
                            "Core output queue is full");
                    }
                }

                if (input == RobinEvent.Restart && session.Reduced)
                {
                    session.Offset += 7100;
                    session.Core.AdvanceTo(now + 7100);
                }

                session.Sequence = command.Sequence;
                session.LastEvent = command.Event;
                session.LastText = command.Text;
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
                pair.Value.Core.Dispatch(RobinEvent.HeadEnded, CoreInputSource.Web, (ulong)elapsed.TotalMilliseconds + pair.Value.Offset);
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
