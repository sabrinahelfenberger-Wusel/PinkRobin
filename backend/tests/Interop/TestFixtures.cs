using PinkRobin.Api.Core;

sealed class ManualClock : TimeProvider
{
    private long ticks;

    private readonly DateTimeOffset initialUtc =
        new(2026, 1, 1, 12, 0, 0, TimeSpan.Zero);

    public override long TimestampFrequency => 1000;

    public override long GetTimestamp() => ticks;

    public override DateTimeOffset GetUtcNow() =>
        initialUtc.AddMilliseconds(ticks);

    public void Advance(long ms)
    {
        if (ms < 0)
        {
            throw new ArgumentOutOfRangeException(nameof(ms));
        }

        ticks = checked(ticks + ms);
    }
}

sealed class TestGateway : IRobinCoreGateway
{
    public PinkRobin.Api.Contracts.CoreConnectionStatus GetConnectionStatus() => new(true, "native-cabi-v1");
    public NativeRobinCore CreateInstance(
        ulong nowMs = 0,
        CorePlatformCapabilities capabilities = default)
    {
        return new NativeRobinCore(nowMs, capabilities);
    }
}

internal sealed class ControlledWeatherSource
    : PinkRobin.Api.Weather.IWeatherSource
{
    public int Calls { get; private set; }
    public string? PostalCode { get; private set; }

    public TaskCompletionSource<PinkRobin.Api.Weather.WeatherSnapshot>
        Completion
    { get; } = new(
        TaskCreationOptions.RunContinuationsAsynchronously);

    public Task<PinkRobin.Api.Weather.WeatherSnapshot> GetAsync(
        string? postalCode,
        CancellationToken cancellationToken)
    {
        ++Calls;
        PostalCode = postalCode;

        return Completion.Task.WaitAsync(cancellationToken);
    }
}