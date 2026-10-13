namespace PinkRobin.Api.Contracts;

public sealed record CoreConnectionStatus(bool Connected, string Implementation);

public sealed record BackendStatus(
    string Service,
    string ApiVersion,
    string Stage,
    CoreConnectionStatus Core,
    bool SessionsAvailable,
    bool CommandsAvailable,
    bool RealtimeAvailable);
