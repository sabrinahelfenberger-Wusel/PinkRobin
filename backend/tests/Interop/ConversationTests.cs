using PinkRobin.Api.Core;
using static TestSupport;

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
}