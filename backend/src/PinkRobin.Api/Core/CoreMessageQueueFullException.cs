namespace PinkRobin.Api.Core;

public sealed class CoreMessageQueueFullException
    : InvalidOperationException
{
    public CoreMessageQueueFullException()
        : base("The Core interaction queue is full.")
    {
    }
}