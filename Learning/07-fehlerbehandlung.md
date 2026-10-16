# 07 – Fehlerbehandlung: Exceptions und Result-Typen

## Lernziel

Unterscheiden, wann ein Fehler den normalen Ablauf unterbricht und wann ein fehlgeschlagenes Ergebnis Teil des erwarteten Ablaufs ist. C# dient als Ausgangspunkt.

## A. Spickzettel

| Konzept | C# | TypeScript | C++ |
|---|---|---|---|
| Exception auslösen | `throw new Exception(...)` | `throw new Error(...)` | `throw std::runtime_error(...)` |
| Behandeln | `try/catch` | `try/catch` | `try/catch` |
| Aufräumen | `finally` / `using` | `finally` | RAII und Destruktoren |
| Fehler als Wert | eigener `Result<T>` | Discriminated Union | `std::expected<T,E>` (C++23) |
| Kein Ergebnis | `null` / `Try...` | `undefined` / Union | `std::optional<T>` |

## B. C# – bekannte Exceptions

```csharp
try
{
    ConnectToRobin();
}
catch (TimeoutException ex)
{
    Console.WriteLine(ex.Message);
}
finally
{
    Console.WriteLine("Verbindungsversuch abgeschlossen");
}
```

Nicht jeder unerwünschte Zustand ist eine Exception: „Person nicht gefunden“ kann ein normales Suchergebnis sein.

Für Ressourcen wie Streams gibt es in C# außerdem `using`.

## C. TypeScript – catch ist nicht automatisch Error

```typescript
try {
    connectToRobin();
} catch (error: unknown) {
    if (error instanceof Error) {
        console.error(error.message);
    } else {
        console.error("Unbekannter Fehler", error);
    }
}
```

JavaScript erlaubt auch `throw "Fehler"`; empfehlenswerter ist normalerweise ein `Error`-Objekt.

### Asynchrone Fehler

```typescript
async function loadRobinState(): Promise<void> {
    try {
        const response = await fetch("/api/robin/state");

        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        const state: unknown = await response.json();
        console.log(state);
    } catch (error) {
        console.error("Robin konnte nicht geladen werden", error);
    }
}
```

**Achtung, C#-Gehirn:** `fetch()` wirft bei HTTP 404 oder 500 normalerweise keine Exception. `response.ok` muss geprüft werden. Auch erfolgreich geparstes JSON ist nicht automatisch strukturell validiert.

Der Pfad `/api/robin/state` ist ein Lernentwurf, kein bereits implementierter Backend-Endpunkt. Den tatsächlich vorhandenen Statusendpunkt `/api/v1/status` behandelt Kapitel 17.

## D. C++ – Exceptions und RAII

```cpp
#include <iostream>
#include <stdexcept>

void connectToRobin()
{
    throw std::runtime_error("Verbindung fehlgeschlagen");
}

int main()
{
    try
    {
        connectToRobin();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
    }
}
```

C++ hat kein `finally`. Stattdessen ist **RAII** zentral: Ressourcen sind an die Lebensdauer von Objekten gebunden; deren Destruktoren übernehmen das Aufräumen, wenn ein Gültigkeitsbereich verlassen wird.

Auf Embedded-Plattformen können Exceptions aus Konfigurations- oder Ressourcengründen deaktiviert sein. Für den späteren ESP32-Code müssen wir die konkrete Toolchain-Konfiguration prüfen. Serverseitiger C++-Core und Embedded-Firmware können daher unterschiedliche Fehlerstrategien verwenden.

## E. Result-Typen

### TypeScript: Discriminated Union

```typescript
type Result<T> =
    | { success: true; value: T }
    | { success: false; error: string };

function processEvent(type: string): Result<string> {
    if (type === "touch") {
        return { success: true, value: "happy" };
    }

    return { success: false, error: "Unknown event type" };
}

const result = processEvent("banana");

if (result.success) {
    console.log(result.value);
} else {
    console.error(result.error);
}
```

Der boolesche Diskriminator `success` ermöglicht TypeScript, die passende Variante zu erkennen.

### C#: einfacher eigener Result-Typ

```csharp
public record Result<T>(bool Success, T? Value, string? Error);
```

Das ist ein **einfaches Beispiel**, kein vollständig abgesichertes Result-Modell: Die Kombinationen von `Success`, `Value` und `Error` sind hier nicht erzwungen. Für produktiven Code kann man getrennte Erfolgs-/Fehlervarianten oder geeignete Bibliotheken verwenden.

### C++23: std::expected

```cpp
#include <expected>
#include <string>

std::expected<std::string, std::string> processEvent(
    const std::string& type)
{
    if (type == "touch")
        return "happy";

    return std::unexpected(std::string{"Unknown event type"});
}
```

`std::expected<T,E>` enthält entweder einen Erfolgswert `T` oder einen Fehlerwert `E`. Voraussetzung ist C++23-Unterstützung durch Compiler und Standardbibliothek. Für ältere Standards braucht man eine andere Lösung.

## F. Pink Robin – Fehler bewusst einordnen

| Situation | Mögliche Strategie |
|---|---|
| Person noch nicht erkannt | Optionaler Wert, kein Fehler |
| Unbekannter Event-Typ | Validierungsfehler als Result |
| Backend vorübergehend nicht erreichbar | kontrollierte Fehleranzeige / Retry-Strategie |
| Unerwarteter Programmierfehler | Exception oder zentraler Fehlerpfad |
| Ressource muss freigegeben werden | `using` in C#, RAII in C++ |

Unsere geplante Pipeline:

```text
Angular → ASP.NET Core → C++-Core
```

Ein ungültiges Event sollte nicht den ganzen Robin-Prozess beenden. Das Backend muss Ergebnisse und Fehler in ein klares API-Verhalten übersetzen.

## G. Übung

Schreibe in TypeScript eine Funktion `parseTouchDuration(value: number): Result<number>`.

Regeln:

- `value` muss endlich sein.
- `value` muss mindestens `0` und höchstens `5000` sein.
- Bei Erfolg wird der Wert zurückgegeben.
- Bei Misserfolg eine verständliche Fehlermeldung.

Teste `250`, `-10`, `6000` und `NaN`.

Überlege: Warum ist ein Result hier sinnvoller als eine Exception für jeden ungültigen Wert?

## Merksätze

- Exceptions sind für außergewöhnliche Unterbrechungen nützlich; erwartbare Fehlschläge lassen sich oft besser als Werte modellieren.
- TypeScript-`catch` sollte unbekannte Fehlerwerte sicher prüfen.
- `fetch()` behandelt HTTP-Fehlerstatus nicht automatisch als geworfene Exception.
- C++ verwendet für zuverlässiges Ressourcenmanagement besonders RAII.
- `std::expected<T,E>` ist ein C++23-Werkzeug für Erfolg **oder** Fehler.
- API-Grenzen brauchen Validierung und eine definierte Fehlerübersetzung.

## Als Nächstes

Speicher und Lebensdauer: C#-Referenzen und Garbage Collection, JavaScript-Objekte sowie C++-Werte, Referenzen, Pointer und Ownership.
