using PinkRobin.Api.Core;
using static TestSupport;
using PinkRobin.Api.Weather;

internal static class ConversationTests
{
    public static void WeatherLocationConversation()
    {
        using var core = new NativeRobinCore(
            capabilities: new CorePlatformCapabilities(
                TextOutput: true,
                OutfitDisplay: false));

        core.ReceiveText(
            "Wie wird das Wetter?",
            CoreInputSource.Web,
            100);

        var question = core.GetNextOutput();

        Require(
            question is CoreConversationReplyOutput
            {
                Kind: CoreConversationReplyKind.AskPostalCode
            },
            "Weather question asks for a postal code");

        core.AcknowledgeOutput(question!.Sequence);

        core.ReceiveText(
            "9000",
            CoreInputSource.Web,
            200);

        var request = core.GetNextOutput()
            as CoreWeatherFetchRequestedOutput
            ?? throw new Exception("Expected weather fetch request.");

        Require(
            request.RequestId == 1 &&
            request.PostalCode == "9000",
            "Postal code reaches the managed adapter");

        core.AcknowledgeOutput(request.Sequence);

        core.ReceiveWeatherResult(
            request.RequestId,
            new CoreWeatherForecast(
                TemperatureC: 8.5,
                PrecipitationMm: 2.0,
                WindSpeedKmh: 10.0,
                WindGustKmh: 20.0,
                ValidFromMs: 300,
                ValidUntilMs: 60000),
            CoreInputSource.Backend,
            300);

        var answer = core.GetNextOutput();

        Require(
            answer is CoreWeatherAnswerOutput
            {
                Kind: CoreWeatherAnswerKind.Rain
            },
            "Fetched weather produces a rain answer");

        core.AcknowledgeOutput(answer!.Sequence);

        Require(
            core.GetNextOutput() is null,
            "Conversation outputs are acknowledged");
    }

    public static void SessionTextCommands()
    {
        using var sessions = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            new ManualClock());

        var session = sessions.Create(false);

        var question = new PinkRobin.Api.Sessions.RobinCommand(
            1,
            "textReceived",
            "Wie wird das Wetter?");

        sessions.Send(session.Token, question);
        sessions.Send(session.Token, question);

        // Gleiche Nummer mit anderem Text ist keine Wiederholung.
        Throws<PinkRobin.Api.Sessions.SessionFailure>(() =>
            sessions.Send(
                session.Token,
                question with { Text = "Hallo Robin" }));

        var output = sessions.ReadOutput(session.Token);

        Require(
            output is
            {
                Sequence: "1",
                Kind: "conversationReply",
                ConversationReply: "askPostalCode"
            },
            "Session text produces a postal code question");

        sessions.AcknowledgeOutput(
            session.Token,
            output!.Sequence);

        Require(
            sessions.ReadOutput(session.Token) is null,
            "Repeated text command does not duplicate output");

        // Eine ungültige Eingabe darf die Befehlsnummer nicht verbrauchen.
        Throws<PinkRobin.Api.Sessions.SessionFailure>(() =>
            sessions.Send(
                session.Token,
                new(2, "textReceived", "   ")));

        var accepted = sessions.Send(
            session.Token,
            new(2, "textReceived", "9000"));

        Require(
            accepted.Sequence == 2,
            "Valid text can reuse a rejected command sequence");
    }

    public static async Task SessionWeatherFetch()
    {
        var clock = new ManualClock();
        var weather = new ControlledWeatherSource();

        using var sessions = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            clock);

        var session = sessions.Create(false);

        sessions.Send(
            session.Token,
            new(1, "textReceived", "Wie wird das Wetter?"));

        var question = sessions.ReadOutput(session.Token);
        Require(question is not null, "Postal code question exists");

        sessions.AcknowledgeOutput(session.Token, question!.Sequence);

        sessions.Send(
            session.Token,
            new(2, "textReceived", "9000"));

        var pending = sessions.ReadOutputAsync(
            session.Token,
            weather,
            CancellationToken.None);

        Require(weather.Calls == 1, "Weather fetch starts once");
        Require(weather.PostalCode == "9000", "Requested postal code is used");

        var concurrent = await sessions.ReadOutputAsync(
            session.Token,
            weather,
            CancellationToken.None);

        Require(concurrent is null, "Internal fetch stays out of UI output");
        Require(weather.Calls == 1, "Concurrent poll does not duplicate fetch");

        var now = clock.GetUtcNow();

        var standardForecast = new WeatherSnapshot(
            Source: "Test",
            PostalCode: "8001",
            PointId: "800100",
            ForecastRunAt: now,
            RetrievedAt: now,
            ValidFrom: now,
            ValidTo: now.AddMinutes(10),
            TemperatureC: 20.0,
            PrecipitationMm: 0.0,
            WindSpeedKmh: 5.0,
            WindGustKmh: 10.0);

        Require(
            !sessions.UpdateWeather(session.Token, standardForecast),
            "Standard weather is blocked while conversation fetch is running");

        weather.Completion.SetResult(new WeatherSnapshot(
            Source: "Test",
            PostalCode: "9000",
            PointId: "900000",
            ForecastRunAt: now,
            RetrievedAt: now,
            ValidFrom: now,
            ValidTo: now.AddMinutes(10),
            TemperatureC: 8.5,
            PrecipitationMm: 2.0,
            WindSpeedKmh: 10.0,
            WindGustKmh: 20.0));

        var answer = await pending;

        Require(
            answer is { Kind: "weatherAnswer", WeatherAnswer: "rain" },
            "Backend fetch returns the Core weather answer");

        Require(
            !sessions.UpdateWeather(session.Token, standardForecast),
            "Standard weather remains blocked after conversation fetch");

        sessions.AcknowledgeOutput(
            session.Token,
            answer!.Sequence);

        sessions.Send(
            session.Token,
            new(3, "textReceived", "Wie ist das Wetter?"));

        var repeated = sessions.ReadOutput(session.Token);

        Require(
            repeated is { Kind: "weatherAnswer", WeatherAnswer: "rain" },
            "Selected location and rain forecast survive standard update");
    }

    public static async Task SessionWeatherFailure()
    {
        var weather = new ControlledWeatherSource();

        using var sessions = new PinkRobin.Api.Sessions.RobinSessions(
            new TestGateway(),
            new ManualClock());

        var session = sessions.Create(false);

        sessions.Send(
            session.Token,
            new(1, "textReceived", "Wie wird das Wetter?"));

        var question = sessions.ReadOutput(session.Token);
        Require(question is not null, "Postal code question exists");

        sessions.AcknowledgeOutput(session.Token, question!.Sequence);

        sessions.Send(
            session.Token,
            new(2, "textReceived", "9000"));

        var pending = sessions.ReadOutputAsync(
            session.Token,
            weather,
            CancellationToken.None);

        weather.Completion.SetException(
            new WeatherUnavailableException("Test weather unavailable"));

        var reply = await pending;

        Require(
            reply is
            {
                Kind: "conversationReply",
                ConversationReply: "weatherUnavailable"
            },
            "Weather failure produces a conversation reply");

        var repeated = await sessions.ReadOutputAsync(
            session.Token,
            weather,
            CancellationToken.None);

        Require(repeated == reply, "Failure reply remains until acknowledged");
        Require(weather.Calls == 1, "Reading the reply does not fetch again");
    }
}