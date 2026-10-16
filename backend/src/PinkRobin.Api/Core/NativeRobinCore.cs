using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace PinkRobin.Api.Core;

public enum RobinEvent : uint { Restart, HeartTouched, HeadStarted, HeadEnded, InteractionCancelled }
public enum RobinMode : uint { Sleeping, Waking, Awake }
public enum RobinMood : uint { Curious, Happy, Affectionate }
public enum RobinExpression : uint { Sleeping, Curious, Happy, HeartEyes }
public sealed record RobinSnapshot(RobinMode Mode, RobinMood Mood, RobinExpression Expression,
    bool HeadContact, ulong TimeMs, ulong Revision);

// One instance belongs to one future session. Callers supply monotonic milliseconds.
public sealed class NativeRobinCore : IDisposable
{
    private readonly CoreHandle handle;
    private readonly object sync = new();
    private bool disposed;

    public NativeRobinCore(ulong nowMs = 0)
    {
        if (NativeMethods.AbiVersion() != 1)
            throw new NotSupportedException("Unsupported PinkRobin Core ABI (expected 1).");
        handle = NativeMethods.Create(nowMs);
        if (handle.IsInvalid)
        {
            handle.Dispose();
            throw new InvalidOperationException("PinkRobin Core allocation failed.");
        }
    }

    public RobinSnapshot AdvanceTo(ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);
            Check(NativeMethods.Advance(handle, nowMs));
            return ReadSnapshot();
        }
    }

    public RobinSnapshot Dispatch(RobinEvent input, ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);
            Check(NativeMethods.Dispatch(handle, (uint)input, nowMs));
            return ReadSnapshot();
        }
    }

    public RobinSnapshot GetSnapshot()
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);
            return ReadSnapshot();
        }
    }

    private RobinSnapshot ReadSnapshot()
    {
        Check(NativeMethods.GetSnapshot(handle, out var state, (uint)Marshal.SizeOf<NativeSnapshot>()));
        if (state.AbiVersion != 1) throw new NotSupportedException("Unexpected Core snapshot ABI.");
        return new((RobinMode)state.Mode, (RobinMood)state.Mood, (RobinExpression)state.Expression,
            state.HeadContact != 0, state.TimeMs, state.Revision);
    }

    private static void Check(uint result)
    {
        if (result == 1) throw new ArgumentException("Invalid Core argument or event.");
        if (result == 2) throw new ArgumentOutOfRangeException("nowMs", "Core time must not move backwards.");
        if (result != 0) throw new InvalidOperationException($"Unknown Core result: {result}.");
    }

    public void Dispose()
    {
        lock (sync)
        {
            if (disposed) return;
            disposed = true;
            handle.Dispose();
        }
    }
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeSnapshot
{
    public uint AbiVersion, Mode, Mood, Expression, HeadContact, Reserved;
    public ulong TimeMs, Revision;
}

internal sealed class CoreHandle : SafeHandleZeroOrMinusOneIsInvalid
{
    public CoreHandle() : base(ownsHandle: true) { }
    protected override bool ReleaseHandle()
    {
        NativeMethods.Destroy(handle);
        return true;
    }
}

internal static class NativeMethods
{
    private const string Library = "pinkrobin_core";

    // Load only the library shipped beside the API, never an arbitrary PATH entry.
    static NativeMethods()
    {
        NativeLibrary.SetDllImportResolver(typeof(NativeMethods).Assembly, (name, assembly, searchPath) =>
        {
            if (name != Library) return IntPtr.Zero;
            var file = OperatingSystem.IsWindows() ? "pinkrobin_core.dll" :
                OperatingSystem.IsMacOS() ? "libpinkrobin_core.dylib" : "libpinkrobin_core.so";
            return NativeLibrary.Load(Path.Combine(AppContext.BaseDirectory, file));
        });
    }

    [DllImport(Library, EntryPoint = "robin_core_abi_version", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint AbiVersion();
    [DllImport(Library, EntryPoint = "robin_core_create", CallingConvention = CallingConvention.Cdecl)]
    internal static extern CoreHandle Create(ulong nowMs);
    [DllImport(Library, EntryPoint = "robin_core_destroy", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void Destroy(IntPtr handle);
    [DllImport(Library, EntryPoint = "robin_core_advance", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint Advance(CoreHandle handle, ulong nowMs);
    [DllImport(Library, EntryPoint = "robin_core_dispatch", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint Dispatch(CoreHandle handle, uint input, ulong nowMs);
    [DllImport(Library, EntryPoint = "robin_core_get_snapshot", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint GetSnapshot(CoreHandle handle, out NativeSnapshot state, uint size);
}
