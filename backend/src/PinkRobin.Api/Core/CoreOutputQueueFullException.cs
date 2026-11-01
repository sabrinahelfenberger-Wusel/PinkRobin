namespace PinkRobin.Api.Core;

public sealed class CoreOutputQueueFullException : InvalidOperationException
{
    public CoreOutputQueueFullException()
        : base("The Core output queue is full.")
    {
    }
}