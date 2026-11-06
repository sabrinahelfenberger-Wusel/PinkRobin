using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.Text;

namespace PinkRobin.Api.Core;

public enum RobinEvent : uint { Restart, HeartTouched, HeadStarted, HeadEnded, InteractionCancelled }
public enum RobinMode : uint { Sleeping, Waking, Awake }
public enum RobinMood : uint { Curious, Happy, Affectionate }
public enum RobinExpression : uint { Sleeping, Curious, Happy, HeartEyes }

public enum CoreTransformation : uint
{
    None = 0,
    Octopus = 1,
    raincoat = 2
}

public sealed record CoreWeatherForecast(
    double TemperatureC,
    double PrecipitationMm,
    double WindSpeedKmh,
    double WindGustKmh,
    ulong ValidFromMs,
    ulong ValidUntilMs);

public sealed record RobinSnapshot(RobinMode Mode, RobinMood Mood, RobinExpression Expression,
    bool HeadContact, ulong TimeMs, ulong Revision);

// One instance belongs to one future session. Callers supply monotonic milliseconds.
public sealed class NativeRobinCore : IDisposable
{
    private readonly CoreHandle handle;
    private readonly object sync = new();
    private bool disposed;
    private static readonly UTF8Encoding StrictUtf8 = new(
        encoderShouldEmitUTF8Identifier: false,
        throwOnInvalidBytes: true);

    public NativeRobinCore(
        ulong nowMs = 0,
        CorePlatformCapabilities capabilities = default)
    {
        if (NativeMethods.AbiVersion() != 1)
        {
            throw new NotSupportedException(
                "Unsupported PinkRobin Core ABI (expected 1).");
        }

        var nativeCapabilities = new NativePlatformCapabilities
        {
            TextOutput = capabilities.TextOutput ? 1u : 0u,
            OutfitDisplay = capabilities.OutfitDisplay ? 1u : 0u
        };

        handle = NativeMethods.CreateWithCapabilities(
            nowMs,
            in nativeCapabilities,
            (uint)Marshal.SizeOf<NativePlatformCapabilities>());

        if (handle.IsInvalid)
        {
            handle.Dispose();

            throw new InvalidOperationException(
                "PinkRobin Core allocation failed.");
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

    public RobinSnapshot Dispatch(
        RobinEvent input,
        CoreInputSource source,
        ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            var result = NativeMethods.ReceiveInteraction(
                handle,
                (uint)input,
                (uint)source,
                nowMs);

            if (result == 4)
            {
                AdvanceTo(nowMs);

                result = NativeMethods.ReceiveInteraction(
                    handle,
                    (uint)input,
                    (uint)source,
                    nowMs);
            }

            Check(result);

            return ReadSnapshot();
        }
    }

    public void PostInteraction(
        RobinEvent input,
        CoreInputSource source,
        ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            Check(NativeMethods.PostInteraction(
                handle,
                (uint)input,
                (uint)source,
                nowMs));
        }
    }

    public void StartTransformation(
        CoreTransformation transformation,
        ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            Check(NativeMethods.StartTransformation(
                handle,
                (uint)transformation,
                nowMs));
        }
    }

    public CoreTransformation GetTransformation()
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            Check(NativeMethods.GetTransformation(
                handle,
                out var transformation));

            if (transformation > (uint)CoreTransformation.raincoat)
            {
                throw new NotSupportedException(
                    "Unknown Core transformation.");
            }

            return (CoreTransformation)transformation;
        }
    }

    public void PostWeather(CoreWeatherForecast forecast, CoreInputSource source, ulong nowMs)
    {
        ArgumentNullException.ThrowIfNull(forecast);

        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            var weather = new NativeWeather
            {
                TemperatureC = forecast.TemperatureC,
                PrecipitationMm = forecast.PrecipitationMm,
                WindSpeedKmh = forecast.WindSpeedKmh,
                WindGustKmh = forecast.WindGustKmh,
                ValidFromMs = forecast.ValidFromMs,
                ValidUntilMs = forecast.ValidUntilMs
            };

            Check(NativeMethods.ReceiveWeather(
                handle,
                in weather,
                (uint)Marshal.SizeOf<NativeWeather>(),
                (uint)source,
                nowMs));
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

    public CoreOutput? GetNextOutput()
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            var result = NativeMethods.GetNextOutputV2(
                handle,
                out var output,
                (uint)Marshal.SizeOf<NativeOutputV2>());

            if (result == 5)
            {
                return null;
            }

            Check(result);

            if (output.AbiVersion != 1 ||
                output.Sequence == 0 ||
                output.Reserved != 0)
            {
                throw new InvalidOperationException(
                    "Invalid native Core output.");
            }

            switch (output.Kind)
            {
                case 1:
                    if (output.WeatherAnswerKind >
                        (uint)CoreWeatherAnswerKind.Storm)
                    {
                        throw new InvalidOperationException(
                            "Invalid native weather answer.");
                    }

                    return new CoreWeatherAnswerOutput(
                        output.Sequence,
                        (CoreWeatherAnswerKind)output.WeatherAnswerKind);

                case 2:
                    if (output.ConversationReplyKind >
                        (uint)CoreConversationReplyKind.WeatherUnavailable)
                    {
                        throw new InvalidOperationException(
                            "Invalid native conversation reply.");
                    }

                    return new CoreConversationReplyOutput(
                        output.Sequence,
                        (CoreConversationReplyKind)output.ConversationReplyKind);

                case 3:
                    var digits = new[]
                    {
                    output.PostalCode0,
                    output.PostalCode1,
                    output.PostalCode2,
                    output.PostalCode3
                };

                    if (output.RequestId == 0 ||
                        digits.Any(value => value < '0' || value > '9'))
                    {
                        throw new InvalidOperationException(
                            "Invalid native weather request.");
                    }

                    var postalCode = new string(
                        digits.Select(value => (char)value).ToArray());

                    return new CoreWeatherFetchRequestedOutput(
                        output.Sequence,
                        output.RequestId,
                        postalCode);

                default:
                    throw new NotSupportedException(
                        "Unknown native Core output kind.");
            }
        }
    }

    public void AcknowledgeOutput(ulong sequence)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            Check(NativeMethods.AcknowledgeOutput(
                handle,
                sequence));
        }
    }

    public void RequestWeatherAnswer(ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            var result = NativeMethods.RequestWeatherAnswer(
                handle,
                nowMs);

            if (result == 4)
            {
                throw new CoreOutputQueueFullException();
            }

            if (result == 6)
            {
                throw new InvalidOperationException(
                    "The Core output sequence is exhausted.");
            }

            Check(result);
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
        if (result == 1)
            throw new ArgumentException("Invalid Core argument or event.");

        if (result == 2)
            throw new ArgumentOutOfRangeException(
                "nowMs",
                "Core time must not move backwards.");

        if (result == 4)
            throw new CoreMessageQueueFullException();

        if (result == 7)
        {
            throw new NotSupportedException(
                "The platform does not support this Core action.");
        }

        if (result != 0)
            throw new InvalidOperationException(
                $"Unknown Core result: {result}.");
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

    public void ReceiveText(
        string text,
        CoreInputSource source,
        ulong nowMs)
    {
        ArgumentNullException.ThrowIfNull(text);

        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            var byteCount = StrictUtf8.GetByteCount(text);

            if (byteCount == 0 || byteCount > 160)
            {
                throw new ArgumentException(
                    "Text must contain between 1 and 160 UTF-8 bytes.",
                    nameof(text));
            }

            var bytes = StrictUtf8.GetBytes(text);

            Check(NativeMethods.ReceiveText(
                handle,
                bytes,
                (uint)bytes.Length,
                (uint)source,
                nowMs));
        }
    }

    public void ReceiveWeatherResult(
        ulong requestId,
        CoreWeatherForecast? forecast,
        CoreInputSource source,
        ulong nowMs)
    {
        lock (sync)
        {
            ObjectDisposedException.ThrowIf(disposed, this);

            if (forecast is null)
            {
                Check(NativeMethods.ReceiveWeatherFailure(
                    handle,
                    requestId,
                    IntPtr.Zero,
                    0,
                    (uint)source,
                    nowMs));

                return;
            }

            var weather = new NativeWeather
            {
                TemperatureC = forecast.TemperatureC,
                PrecipitationMm = forecast.PrecipitationMm,
                WindSpeedKmh = forecast.WindSpeedKmh,
                WindGustKmh = forecast.WindGustKmh,
                ValidFromMs = forecast.ValidFromMs,
                ValidUntilMs = forecast.ValidUntilMs
            };

            Check(NativeMethods.ReceiveWeatherResult(
                handle,
                requestId,
                in weather,
                (uint)Marshal.SizeOf<NativeWeather>(),
                (uint)source,
                nowMs));
        }
    }
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeSnapshot
{
    public uint AbiVersion, Mode, Mood, Expression, HeadContact, Reserved;
    public ulong TimeMs, Revision;
}
internal struct NativeWeather
{
    public double TemperatureC;
    public double PrecipitationMm;
    public double WindSpeedKmh;
    public double WindGustKmh;
    public ulong ValidFromMs;
    public ulong ValidUntilMs;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeOutput
{
    public uint AbiVersion;
    public uint Kind;
    public ulong Sequence;
    public uint WeatherAnswerKind;
    public uint Reserved;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeOutputV2
{
    public uint AbiVersion;
    public uint Kind;
    public ulong Sequence;

    public uint WeatherAnswerKind;
    public uint ConversationReplyKind;

    public ulong RequestId;

    public byte PostalCode0;
    public byte PostalCode1;
    public byte PostalCode2;
    public byte PostalCode3;

    public uint Reserved;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativePlatformCapabilities
{
    public uint TextOutput;
    public uint OutfitDisplay;
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

    [DllImport(Library, EntryPoint = "robin_core_post_weather", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint PostWeather(CoreHandle handle, in NativeWeather weather, uint weatherSize);
    [DllImport(
        Library,
        EntryPoint = "robin_core_receive_weather",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint ReceiveWeather(
        CoreHandle handle,
        in NativeWeather weather,
        uint weatherSize,
        uint source,
        ulong nowMs);

    [DllImport(
        Library,
        EntryPoint = "robin_core_receive_interaction",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint ReceiveInteraction(
        CoreHandle handle,
        uint eventValue,
        uint source,
        ulong nowMs);

    [DllImport(
        Library,
        EntryPoint = "robin_core_post_interaction",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint PostInteraction(
        CoreHandle handle,
        uint eventValue,
        uint source,
        ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_request_weather_answer",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint RequestWeatherAnswer(
        CoreHandle handle,
        ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_get_next_output",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint GetNextOutput(
        CoreHandle handle,
        out NativeOutput output,
        uint outputSize);

    [DllImport(
        Library,
        EntryPoint = "robin_core_get_next_output_v2",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint GetNextOutputV2(
        CoreHandle handle,
        out NativeOutputV2 output,
        uint outputSize);

    [DllImport(
        Library,
        EntryPoint = "robin_core_receive_text",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint ReceiveText(
        CoreHandle handle,
        [In] byte[] utf8,
        uint textSize,
        uint source,
        ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_acknowledge_output",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint AcknowledgeOutput(
        CoreHandle handle,
        ulong sequence);

    [DllImport(
        Library,
        EntryPoint = "robin_core_create_with_capabilities",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern CoreHandle CreateWithCapabilities(
        ulong nowMs,
        in NativePlatformCapabilities capabilities,
        uint capabilitiesSize);

    [DllImport(
        Library,
        EntryPoint = "robin_core_receive_weather_result",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint ReceiveWeatherResult(
        CoreHandle handle,
        ulong requestId,
        in NativeWeather weather,
        uint weatherSize,
        uint source,
        ulong nowMs);

    [DllImport(
        Library,
        EntryPoint = "robin_core_receive_weather_result",
        CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint ReceiveWeatherFailure(
        CoreHandle handle,
        ulong requestId,
        IntPtr weather,
        uint weatherSize,
        uint source,
        ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_start_transformation", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint StartTransformation(CoreHandle handle, uint transformation, ulong nowMs);
    [DllImport(Library, EntryPoint = "robin_core_get_transformation", CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint GetTransformation(CoreHandle handle, out uint transformation);
}
