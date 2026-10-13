using PinkRobin.Api.Contracts;

namespace PinkRobin.Api.Core;

public sealed class UnconnectedRobinCoreGateway : IRobinCoreGateway
{
    public CoreConnectionStatus GetConnectionStatus() => new(false, "not-connected");
}
