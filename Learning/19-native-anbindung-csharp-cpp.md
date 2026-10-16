# 19 – Native Anbindung: C# ↔ C++

## Lernziel

Du kannst die vorhandene C-Schnittstelle des Pink-Robin-Core aus C# aufrufen, ihren Snapshot korrekt abbilden und den nativen Handle mit `SafeHandle` verwalten. Du unterscheidest Ressourcenbesitz, Threadsicherheit und fachliche Fehler.

Wir verbinden Lebensdauer aus [Kapitel 11](11-speicher-und-lebensdauer.md), Nebenläufigkeit aus [Kapitel 15](15-asynchronitaet-und-nebenlaeufigkeit.md), API-Grenzen aus [Kapitel 16](16-serialisierung-json-und-api-grenzen.md) und Builds aus [Kapitel 13](13-module-projektstruktur-und-build-systeme.md).

**Das Kapitel liefert einen Lernadapter. Es integriert die native Bibliothek nicht automatisch ins bestehende Backend und ändert keinen Produktivcode.** Grundlage ist die aktuelle [C-ABI, Version 1](../core/include/pinkrobin/c_api.h).

## A. Spickzettel

| Native Seite | C#-Abbildung | Bedeutung |
|---|---|---|
| `uint32_t` | `uint` | genau 32 Bit ohne Vorzeichen |
| `uint64_t` | `ulong` | genau 64 Bit ohne Vorzeichen |
| `robin_core_handle*` | eigener `SafeHandle` | opaker Ressourcenbesitz |
| `robin_snapshot_v1* output` | `out NativeSnapshot` | Ausgabe in Speicher der Aufruferin |
| `uint32_t output_size` | `uint` | erwartete Puffergrösse |
| `ROBIN_CALL` / `__cdecl` | `CallingConvention.Cdecl` | Aufrufkonvention des Vertrags |
| `robin_core_destroy` | `ReleaseHandle()` | Freigabe mit derselben API |
| Status `0`, `1`, `2` | ausdrücklich prüfen | Erfolg, ungültiges Argument, Rückwärtszeit |

Ein `IntPtr` allein beschreibt eine Adresse, keinen Besitzer. Eine importierte Methode wird im selben Prozess ausgeführt; P/Invoke ist weder HTTP noch eine isolierte Sandbox.

## B. Warum eine C-Schnittstelle?

C++-Klassen können virtuelle Methoden, compilerabhängiges Layout, Standardbibliothekstypen und sprachspezifische Namenskodierung besitzen. Diese Details direkt aus C# nachzubauen wäre kein stabiler Vertrag.

Der Core veröffentlicht deshalb Funktionen mit C-Linkage und einen opaken Handle. C# kennt dessen Inhalt nicht und darf ihn nicht dereferenzieren. Erzeugen, Aufrufen und Zerstören erfolgen ausschliesslich über die vorgesehenen Funktionen.

`extern "C"` regelt die Sprachbindung, macht aber nicht beliebige Parameter automatisch ABI-stabil. Der Vertrag muss zusätzlich Feldbreiten, Layout, Aufrufkonvention, Fehler und Lebensdauer festlegen. Ein C++-`bool` oder `std::string` wird hier bewusst nicht übertragen.

## C. Das Snapshot-Layout verstehen

ABI v1 hat diese Felder in dieser Reihenfolge:

| Offset in Bytes | Feld | C#-Typ |
|---|---|---|
| 0 | `abi_version` | `uint` |
| 4 | `mode` | `uint` |
| 8 | `mood` | `uint` |
| 12 | `expression` | `uint` |
| 16 | `head_contact` | `uint`, nur 0 oder 1 |
| 20 | `reserved` | `uint`, derzeit 0 |
| 24 | `time_ms` | `ulong` |
| 32 | `revision` | `ulong` |

Gesamtgrösse: **40 Bytes**. `head_contact` nicht durch ein C#-`bool` ersetzen: Die C-ABI verwendet ausdrücklich ein 32-Bit-Integer. `StructLayout(LayoutKind.Sequential)` erhält die Feldreihenfolge nach den Layoutregeln. Kein geratenes `Pack = 1` ergänzen, um einen Fehler scheinbar zu beheben; Grösse und Offsets gegen den echten Vertrag prüfen.

Die C-API kopiert den Snapshot und behält den Ausgabepuffer nicht. `out` ist deshalb passend. Ein API-Vertrag, der einen Pointer nach dem Aufruf speichert, bräuchte eine andere Lebensdauerstrategie.

## D. Vollständiger C#-Lernadapter

Das folgende Programm ist für ein normales **.NET-8-oder-neuer-Konsolenprojekt** gedacht. Der Lernadapter verwendet `DllImport` mit dem üblichen Laufzeit-Marshalling. Für neue Interop-Schichten empfiehlt Microsoft, wo passend, quellgeneriertes `LibraryImport`; dessen Details zu partiellen Methoden, Calling Convention und Buildkonfiguration sind gesondert zu behandeln. [Microsoft: Native Interop](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/best-practices)

```csharp
using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeSnapshot
{
    public uint AbiVersion;
    public uint Mode;
    public uint Mood;
    public uint Expression;
    public uint HeadContact;
    public uint Reserved;
    public ulong TimeMs;
    public ulong Revision;
}

internal sealed class CoreHandle : SafeHandle
{
    public CoreHandle() : base(IntPtr.Zero, ownsHandle: true) { }
    public override bool IsInvalid => handle == IntPtr.Zero;

    protected override bool ReleaseHandle()
    {
        Native.Destroy(handle);
        return true;
    }
}

internal static class Native
{
    private const string Library = "pinkrobin_core";

    [DllImport(Library, EntryPoint = "robin_core_abi_version",
        ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint AbiVersion();

    [DllImport(Library, EntryPoint = "robin_core_create",
        ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
    internal static extern CoreHandle Create(ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_destroy",
        ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
    internal static extern void Destroy(IntPtr handle);

    [DllImport(Library, EntryPoint = "robin_core_dispatch",
        ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint Dispatch(CoreHandle handle, uint input, ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_advance",
        ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint Advance(CoreHandle handle, ulong nowMs);

    [DllImport(Library, EntryPoint = "robin_core_get_snapshot",
        ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint GetSnapshot(
        CoreHandle handle, out NativeSnapshot output, uint outputSize);
}

internal sealed class RobinSession : IDisposable
{
    private readonly object gate = new();
    private readonly CoreHandle handle;
    private bool disposed;

    public RobinSession(ulong nowMs)
    {
        if (Marshal.SizeOf<NativeSnapshot>() != 40
            || Marshal.OffsetOf<NativeSnapshot>(nameof(NativeSnapshot.TimeMs)).ToInt32() != 24
            || Marshal.OffsetOf<NativeSnapshot>(nameof(NativeSnapshot.Revision)).ToInt32() != 32)
            throw new InvalidOperationException("Snapshot-Layout passt nicht zu ABI v1");

        if (Native.AbiVersion() != 1)
            throw new NotSupportedException("Core-ABI wird nicht unterstützt");

        handle = Native.Create(nowMs);
        if (handle.IsInvalid)
        {
            handle.Dispose();
            throw new OutOfMemoryException("Core konnte nicht erstellt werden");
        }
    }

    public NativeSnapshot TouchHeart(ulong nowMs)
    {
        lock (gate)
        {
            EnsureOpen();
            Check(Native.Dispatch(handle, 1u, nowMs)); // ROBIN_EVENT_HEART_TOUCHED
            return ReadSnapshot();
        }
    }

    public NativeSnapshot AdvanceTo(ulong nowMs)
    {
        lock (gate)
        {
            EnsureOpen();
            Check(Native.Advance(handle, nowMs));
            return ReadSnapshot();
        }
    }

    private NativeSnapshot ReadSnapshot()
    {
        Check(Native.GetSnapshot(handle, out var state,
            checked((uint)Marshal.SizeOf<NativeSnapshot>())));
        if (state.AbiVersion != 1 || state.HeadContact > 1 || state.Reserved != 0)
            throw new InvalidOperationException("Ungültiger ABI-v1-Snapshot");
        return state;
    }

    private void EnsureOpen()
    {
        if (disposed)
            throw new ObjectDisposedException(nameof(RobinSession));
    }

    private static void Check(uint status)
    {
        switch (status)
        {
            case 0: return;
            case 1: throw new ArgumentException("Native API: ungültiges Argument");
            case 2: throw new InvalidOperationException("Core-Zeit läuft rückwärts");
            default: throw new InvalidOperationException($"Unbekannter Core-Status: {status}");
        }
    }

    public void Dispose()
    {
        lock (gate)
        {
            if (disposed) return;
            disposed = true;
            handle.Dispose();
        }
    }
}

internal static class Program
{
    public static void Main()
    {
        using var session = new RobinSession(0);
        NativeSnapshot touched = session.TouchHeart(100);
        Console.WriteLine(touched.Mood); // 1 = happy
        NativeSnapshot expired = session.AdvanceTo(6100);
        Console.WriteLine(expired.Mood); // 0 = curious
    }
}
```

Alle Abschnitte dieses Codeblocks gehören in dieselbe Beispieldatei. Der Wrapper zeigt gezielt Herzberührung und Zeitfortschritt. Weitere Ereignisse und die Übersetzung numerischer Mode-/Mood-/Expression-Werte in eigene C#-Modelle wären ausdrückliche Erweiterungen, keine ungeprüften Casts. Unbekannte externe Werte nach dem Vertrag behandeln.

## E. SafeHandle: genau eine zuständige Besitzerin

`CoreHandle` besitzt das native Objekt. `ReleaseHandle()` ruft ausschliesslich den passenden `robin_core_destroy` auf; `Marshal.FreeHGlobal`, `delete` aus einer anderen Bibliothek oder ein zweiter unabhängiger Besitzer wären falsch.

Der öffentliche parameterlose Konstruktor ermöglicht die Erzeugung des Handle-Wrappers durch den Marshaller. Als ungültig gilt in dieser API der Nullpointer. Nicht pauschal Regeln für Betriebssystem-Handles übernehmen, bei denen auch `-1` ungültig sein kann. [Microsoft: SafeHandle](https://learn.microsoft.com/en-us/dotnet/api/system.runtime.interopservices.safehandle)

Bei importierten Aufrufen wird der `SafeHandle` als solcher übergeben. Dadurch kann die Interop-Schicht ihn für die Dauer des Aufrufs schützen. Nicht bequem überall `DangerousGetHandle()` aufrufen und den Schutz durch rohe Adressen umgehen. Die Freigabefunktion erhält innerhalb von `ReleaseHandle` dagegen bewusst den rohen gespeicherten Handle.

`using` löst die kontrollierte Freigabe aus. Die SafeHandle-Finalisierung ist ein Rückfallmechanismus, kein Ersatz für einen geordneten Sitzungsabschluss. Die native Bibliothek muss auch beim Freigeben noch verfügbar sein; nicht manuell entladen, solange Handles existieren. `ReleaseHandle` darf keine normalen fachlichen Exceptions nach aussen werfen.

**Achtung, C#-Gehirn:** Der GC weiss nicht, wie ein beliebiger `IntPtr` zerstört werden muss. Und ein lebender SafeHandle macht das enthaltene C++-Objekt nicht threadsicher.

## F. Serialisierung und Lebensdauer getrennt absichern

Das `lock` im Beispiel schützt Dispatch, Advance, Snapshot und Dispose mit derselben Sperre. Der zusammengehörige Core-Aufruf und sein Snapshot bleiben so eine logische Operation; ein zweiter Aufruf kann sich nicht dazwischen schieben. Es gibt keinen öffentlichen Zugriff auf den Handle.

Die Methoden führen nur kurze synchrone native Arbeit aus. Keine HTTP-Aufrufe, `await` oder fremde Callbacks innerhalb dieser Sperre. Einen zurückgegebenen Snapshot kann die Aufruferin anschliessend unabhängig verwenden.

Das Lock garantiert gegenseitigen Ausschluss, aber keine richtige zeitliche Reihenfolge konkurrierender Requests. Zwei vor dem Eintritt ermittelte Zeitwerte können in umgekehrter Reihenfolge ankommen. Ein produktiver Sitzungsadapter benötigt daher zusätzlich die geordnete Verarbeitung und vertrauenswürdige monotone Zeit aus Kapitel 15 und 18.

Beim Sitzungsende zunächst neue Arbeit verhindern, eingereihte und laufende Zugriffe nach Vertrag abschliessen und danach entsorgen. Unser kleiner Wrapper blockiert einen gleichzeitig eintretenden Aufruf mit derselben Sperre und lehnt spätere Nutzung nach Dispose ab. Er implementiert keine vollständige Server-Queue.

## G. Statuscodes sind keine fremden Exceptions

Die aktuelle C-ABI meldet Fehler als Zahlen. Nur `robin_core_create` verwendet den Nullpointer als Fehlerergebnis. `SetLastError = true` wäre hier kein Ersatz: Die Schnittstelle verspricht weder Win32-LastError noch `errno` als Fehlerkanal.

Der Lernadapter übersetzt Statuscodes in C#-Exceptions. Bei einer erwartbaren fachlichen Ablehnung kann ein produktiver Adapter stattdessen ein Result-Modell verwenden. Wichtig ist, jeden Rückgabewert vor der Verwendung des Ergebnisses zu prüfen. Ein fehlgeschlagener Snapshot-Aufruf darf nicht als gültiger Zustand ausgegeben werden.

Eine C#-Exception macht bereits erfolgte native Zustandsänderungen nicht rückgängig. Falls Dispatch erfolgreich ist, aber eine nachfolgende Ausgabe scheitert, darf das Ereignis nicht blind wiederholt werden. An der ABI bleiben C++-Exceptions laut Core-Vertrag innerhalb der nativen Implementierung. Ein beliebiger Speicherfehler in nativem Code lässt sich dennoch nicht verlässlich durch ein normales `catch` reparieren.

## H. Bibliothek bauen und bereitstellen

Den Core wie in seinem [README](../core/README.md) bauen und testen:

```sh
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build --config Release
ctest --test-dir core/build -C Release --output-on-failure
```

Der logische Importname `pinkrobin_core` kann von .NET abhängig von Plattform und Suchregeln um Präfix und Endung ergänzt werden. Übliche Artefaktnamen sind `pinkrobin_core.dll`, `libpinkrobin_core.so` und `libpinkrobin_core.dylib`. [Microsoft: Native Bibliotheken laden](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/cross-platform)

Für einen lokalen Lernlauf die passende Bibliothek in das Ausgabeverzeichnis der C#-Anwendung bereitstellen und von dort starten. Bei Multi-Config-Builds liegt sie oft unter `core/build/Release`. CPU-Architektur, Betriebssystem und native Abhängigkeiten müssen zum .NET-Prozess passen. Ein ARM64-Prozess lädt keine beliebige x64-Bibliothek.

Für die Auslieferung eine reproduzierbare Kopier-/Paketstrategie vorsehen, statt auf zufällige globale Suchpfade zu vertrauen. Ein vollständiges Publish-Paket muss die erforderliche native Datei und deren Abhängigkeiten enthalten. Erfolgreiches `dotnet build` allein lädt die Bibliothek noch nicht und beweist deshalb keine funktionierende P/Invoke-Anbindung.

| Symptom | Erste Prüfung |
|---|---|
| `DllNotFoundException` | Datei oder eine ihrer nativen Abhängigkeiten fehlt / Suchpfad passt nicht |
| `EntryPointNotFoundException` | falscher Exportname oder inkompatible Bibliotheksversion |
| `BadImageFormatException` | u. a. falsche Architektur oder ungeeignetes Binärformat |
| Falsche Snapshot-Werte | Feldbreiten, Layout, Version und Signatur prüfen |
| Absturz bei Freigabe | falscher Besitzer, doppelte Freigabe oder Nutzung nach Lebensende |

Die Symptome können mehrere Ursachen haben. Die erste Aufgabe ist, die tatsächlich geladene Datei und Architektur festzustellen.

## I. Integrationstests vor der Backend-Anbindung

Die vorhandenen [C-ABI-Tests](../core/tests/c_api_tests.c) prüfen die Grenze aus einem C-Aufrufer. Zusätzlich muss der reale C#-Adapter mit der realen Bibliothek getestet werden:

1. ABI-Version, Struct-Grösse und Offsets prüfen.
2. Instanz erzeugen, Herz bei `100` ms auslösen, `Mood == 1` prüfen.
3. Bei `6099` ms weiterhin glücklich, bei `6100` ms neugierig prüfen.
4. Rückwärtszeit ablehnen und unveränderten Zustand nachweisen.
5. Zwei Sitzungen auf Isolation prüfen.
6. Doppelte Dispose-Aufrufe zulassen, weitere Nutzung kontrolliert ablehnen.
7. Verpackung und Ladeverhalten auf den Zielplattformen testen.

Freigabe „genau einmal“ lässt sich gezielt mit einer instrumentierten Testbibliothek untersuchen. Einen bereits zerstörten rohen Handle absichtlich nochmals an die echte Bibliothek zu übergeben wäre kein sauberer Testfall, sondern undefinierte Nutzung ausserhalb des Vertrags.

Der aktuelle [Core-Status](../core/README.md) beschreibt die C#-Anbindung noch als nächsten Integrationsschritt. Dieses Kapitel macht den Entwurf nachvollziehbar; es meldet keine bereits hergestellte Backend-Verbindung. TypeScript bleibt auf der anderen Seite der Backend-API und importiert keine native Serverbibliothek in den Browser.

## J. Miniübung

1. Warum bildet `HeadContact` ein `uint` ab, obwohl der fachliche Wert nur ja/nein ist?
2. Warum reicht ein SafeHandle nicht aus, um zwei gleichzeitige Dispatch-Aufrufe sicher zu machen?
3. Warum darf `ReleaseHandle` nicht `Marshal.FreeHGlobal` verwenden?
4. Erweitere `Main`: Rufe `AdvanceTo(6099)` vor `AdvanceTo(6100)` auf und prüfe die beiden Mood-Werte.
5. Was unterscheidet eine fehlende native Datei von einem Fehlerstatus `2`?

<details>
<summary>Lösung anzeigen</summary>

1. Die ABI legt ausdrücklich ein 32-Bit-Feld fest. Erst nach dem Lesen wird daraus fachlich ein boolescher Wert.
2. SafeHandle verwaltet die Ressource; die Synchronisierung des veränderlichen Core-Zustands bleibt eine eigene Aufgabe.
3. Allokation und Freigabe gehören zum Vertrag derselben nativen API. Nur `robin_core_destroy` kennt das Objekt und seine richtige Zerstörung.
4. Bei `6099` ist Mood `1`, bei `6100` Mood `0`, bezogen auf die Herzberührung bei `100`.
5. Eine fehlende Datei verhindert bereits das Laden beziehungsweise den Aufruf. Status `2` stammt aus einem erfolgreich erreichten Core-Aufruf und bedeutet rückwärts laufende Zeit.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Die native C-ABI ist ein ausdrücklicher binärer Vertrag, keine automatisch gemeinsame C++-/C#-Klassenwelt.
- Signaturen, Aufrufkonvention, Layout und Version müssen zusammenpassen.
- SafeHandle besitzt; die richtige API gibt frei; Synchronisierung ordnet die Zugriffe.
- Ein Snapshot ist eine Kopie und kein Pointer in den laufenden Core.
- Rückgabewerte prüfen und Fehler an der Grenze bewusst übersetzen.
- Ein verwalteter Build ist noch kein erfolgreicher nativer Integrationstest.
- Pro Sitzung eine Instanz, geordnete Zugriffe und Freigabe nach Ende aller Arbeit.

[← 18 – Zustandsmaschinen und zeitgesteuertes Verhalten](18-zustandsmaschinen-und-zeitgesteuertes-verhalten.md) · [Inhaltsverzeichnis](README.md) · [20 – HTTP, WebSockets und Verbindungsmanagement →](20-http-websockets-und-verbindungsmanagement.md)
