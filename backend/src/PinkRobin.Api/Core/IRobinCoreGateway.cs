using PinkRobin.Api.Contracts;

namespace PinkRobin.Api.Core;

// Native ABI/process communication belongs behind this boundary.
// Rendering and SVG geometry remain responsibilities of the frontend/display.
public interface IRobinCoreGateway
{
    CoreConnectionStatus GetConnectionStatus();
}
