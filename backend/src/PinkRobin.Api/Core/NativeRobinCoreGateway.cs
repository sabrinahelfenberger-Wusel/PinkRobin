using PinkRobin.Api.Contracts;

namespace PinkRobin.Api.Core;

public sealed class NativeRobinCoreGateway(ILogger<NativeRobinCoreGateway> logger) : IRobinCoreGateway
{
    private readonly CoreConnectionStatus status = Probe(logger);

    // The gateway is a factory, not a shared Robin state for all visitors.
    public NativeRobinCore CreateInstance(ulong nowMs = 0) => new(nowMs);
    public CoreConnectionStatus GetConnectionStatus() => status;

    private static CoreConnectionStatus Probe(ILogger logger)
    {
        try
        {
            using var core = new NativeRobinCore();
            core.GetSnapshot();
            logger.LogInformation("Native PinkRobin Core connected (C ABI 1).");
            return new(true, "native-cabi-v1");
        }
        catch (Exception error) when (error is DllNotFoundException or EntryPointNotFoundException
            or BadImageFormatException or NotSupportedException or InvalidOperationException)
        {
            logger.LogError(error, "Native PinkRobin Core unavailable. Check the library, architecture and ABI; restart after repair.");
            return new(false, "not-connected");
        }
    }
}
