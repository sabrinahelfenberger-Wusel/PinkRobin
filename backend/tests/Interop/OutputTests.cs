using PinkRobin.Api.Core;
using PinkRobin.Api.Weather;
using static TestSupport;

internal static class OutputTests
{
    public static void ReadingAndAcknowledgement()
    {
        using (var outputCore = new NativeRobinCore(
                   capabilities: new CorePlatformCapabilities(
                       TextOutput: true,
                       OutfitDisplay: false)))
        {
            Require(
                outputCore.GetNextOutput() is null,
                "Output queue starts empty");

            outputCore.RequestWeatherAnswer(100);

            var firstOutput = outputCore.GetNextOutput();

            Require(
                firstOutput is not null &&
                firstOutput.Sequence == 1 &&
                firstOutput.Kind == CoreWeatherAnswerKind.Unknown,
                "Weather answer without data is unknown");

            Require(
                outputCore.GetNextOutput() == firstOutput,
                "Reading preserves the pending output");

            outputCore.AcknowledgeOutput(firstOutput!.Sequence);

            Require(
                outputCore.GetNextOutput() is null,
                "Acknowledged output is removed");

            outputCore.AcknowledgeOutput(firstOutput.Sequence);
        }

        Console.WriteLine(
            "Core output reading and acknowledgement passed.");
    }

    public static void CapacityAndRecovery()
    {
        using (var fullOutputCore = new NativeRobinCore(
                   capabilities: new CorePlatformCapabilities(
                       TextOutput: true,
                       OutfitDisplay: false)))
        {
            for (ulong nowMs = 1; nowMs <= 8; ++nowMs)
            {
                fullOutputCore.RequestWeatherAnswer(nowMs);
            }

            Throws<CoreOutputQueueFullException>(() =>
                fullOutputCore.RequestWeatherAnswer(9));

            fullOutputCore.AcknowledgeOutput(1);

            // Nach der Bestätigung ist wieder Platz.
            fullOutputCore.RequestWeatherAnswer(10);

            for (ulong sequence = 2; sequence <= 9; ++sequence)
            {
                var queuedOutput = fullOutputCore.GetNextOutput();

                Require(
                    queuedOutput is not null &&
                    queuedOutput.Sequence == sequence,
                    "Outputs retain their sequence after overflow");

                fullOutputCore.AcknowledgeOutput(sequence);
            }

            Require(
                fullOutputCore.GetNextOutput() is null,
                "Output queue is empty after all acknowledgements");
        }

        Console.WriteLine(
            "Core output capacity and recovery passed.");
    }

    public static void WeatherValidity()
    {
        using (var rainyOutputCore = new NativeRobinCore(
                   capabilities: new CorePlatformCapabilities(
                       TextOutput: true,
                       OutfitDisplay: false)))
        {
            rainyOutputCore.PostWeather(
                new CoreWeatherForecast(
                    TemperatureC: 8.5,
                    PrecipitationMm: 2.0,
                    WindSpeedKmh: 10.0,
                    WindGustKmh: 20.0,
                    ValidFromMs: 0,
                    ValidUntilMs: 1000),
                CoreInputSource.Backend,
                100);

            rainyOutputCore.RequestWeatherAnswer(100);

            var rainOutput = rainyOutputCore.GetNextOutput();

            Require(
                rainOutput is not null &&
                rainOutput.Kind == CoreWeatherAnswerKind.Rain,
                "Weather answer uses current world knowledge");

            rainyOutputCore.AcknowledgeOutput(rainOutput!.Sequence);

            // An der Gültigkeitsgrenze ist das Wetterwissen abgelaufen.
            rainyOutputCore.RequestWeatherAnswer(1000);

            var expiredOutput = rainyOutputCore.GetNextOutput();

            Require(
                expiredOutput is not null &&
                expiredOutput.Kind == CoreWeatherAnswerKind.Unknown,
                "Expired weather produces an unknown answer");
        }
    }

    public static void SessionDelivery()
    {
        using (var outputSessions = new PinkRobin.Api.Sessions.RobinSessions(
                   new TestGateway(),
                   new ManualClock()))
        {
            var outputSession = outputSessions.Create(false);

            var otherOutputSession = outputSessions.Create(false);

            var question = new PinkRobin.Api.Sessions.RobinCommand(
                1,
                "weatherAsked");

            outputSessions.Send(outputSession.Token, question);
            outputSessions.Send(outputSession.Token, question);

            Require(
                outputSessions.ReadOutput(otherOutputSession.Token) is null,
                "Outputs remain isolated per session");

            var sessionOutput = outputSessions.ReadOutput(outputSession.Token);

            Require(
                sessionOutput is not null &&
                sessionOutput.Sequence == "1" &&
                sessionOutput.WeatherAnswer == "unknown",
                "Session weather question produces an output");

            Require(
                outputSessions.ReadOutput(outputSession.Token) == sessionOutput,
                "Session reading preserves the output");

            outputSessions.AcknowledgeOutput(
                outputSession.Token,
                sessionOutput!.Sequence);

            Require(
                outputSessions.ReadOutput(outputSession.Token) is null,
                "Repeated command does not create a duplicate output");
        }
    }

    public static void UnsupportedActions()
    {
        using var core = new NativeRobinCore();

        core.AdvanceTo(8000);
        var before = core.GetSnapshot();

        Throws<NotSupportedException>(() =>
            core.RequestWeatherAnswer(8000));

        Throws<NotSupportedException>(() =>
            core.StartTransformation(
                CoreTransformation.Octopus,
                8000));

        Require(
            core.GetNextOutput() is null,
            "Unsupported answer must not create output");

        Require(
            core.GetTransformation() == CoreTransformation.None,
            "Unsupported transformation must not start");

        Require(
            core.GetSnapshot() == before,
            "Unsupported actions must preserve the snapshot");
    }
}
