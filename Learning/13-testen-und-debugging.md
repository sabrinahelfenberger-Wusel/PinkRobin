# 13 – Testen und Debugging

## Lernziel

Du kannst aus einer Verhaltensregel einen automatisierten Test ableiten, Grenzfälle prüfen und einen Fehler mit Debugger und Diagnosewerkzeugen eingrenzen. C# ist der Anker; TypeScript/JavaScript und C++17 zeigen die Unterschiede. Für Pink Robin testen wir Zeit als Eingabewert statt mit echten Wartezeiten.

Nach [Kapitel 12](12-serialisierung-json-und-api-grenzen.md) prüfen wir nicht nur erfolgreiche Nachrichten, sondern auch falsche Typen, fehlende Angaben und abgelehnte Operationen. **Tests liefern Evidenz für die geprüften Fälle; eine grüne Testsuite beweist nicht die allgemeine Fehlerfreiheit.**

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Testfälle organisieren | z. B. NUnit, xUnit oder MSTest | projektspezifischer Runner; hier `node:test` | hier vorhandene Programme unter CTest |
| Erwartung prüfen | Framework-Assertion | z. B. `node:assert/strict` | explizite Prüfungen mit Fehlerstatus |
| Mehrere Eingaben | z. B. NUnit-`TestCase` | Tabelle und Schleife im Test | Tabelle oder getrennte Szenarien |
| Async testen | Test gibt `Task` zurück und wartet | Test gibt Promise zurück / verwendet `await` | Arbeit kontrolliert abschliessen, Ergebnisse prüfen |
| Abhängigkeit ersetzen | Stub, Fake oder Mock | injizierte Funktion / Testobjekt | injizierte Komponente, Funktion oder Interface |
| Zeit kontrollieren | Uhr abstrahieren / Zeit explizit übergeben | Uhr / Scheduler ersetzen oder Zeit übergeben | Core erhält `now_ms` explizit |
| Speicherfehler finden | Debugger und .NET-Diagnose | DevTools und Laufzeitdiagnose | z. B. AddressSanitizer / UndefinedBehaviorSanitizer |

Die Frameworks sind Beispiele, keine neue Festlegung für alle Repository-Projekte. Die vorhandenen [Core-Tests](../core/tests/core_tests.cpp) und [C-ABI-Tests](../core/tests/c_api_tests.c) verwenden eigene Prüfungen und benötigen dafür kein zusätzliches Testframework.

## B. Verhalten prüfen: Arrange, Act, Assert

Ein guter Test hat einen nachvollziehbaren Ablauf:

1. **Arrange:** Ausgangszustand und benötigte Abhängigkeiten vorbereiten.
2. **Act:** Den interessierenden Vorgang auslösen.
3. **Assert:** Das beobachtbare Ergebnis mit der Anforderung vergleichen.

Beispiel: „Nach einer Herzberührung bleibt Robin 6000 ms glücklich.“ Daraus folgen mindestens ein Test kurz vor und einer an der Grenze. Nur „direkt nach der Berührung ist Robin glücklich“ prüft die Dauer nicht.

Ein Testname wie `HeartTouch_At6000Milliseconds_BecomesCurious` beschreibt Auslöser, Zeitpunkt und Erwartung. Ein Name wie `Test1` hilft bei einem Fehler kaum. Pro Test einen zusammenhängenden Verhaltensaspekt prüfen; dafür dürfen mehrere passende Assertions nötig sein. [Microsoft: Gute Unit-Tests](https://learn.microsoft.com/en-us/dotnet/core/testing/unit-testing-best-practices)

Die Erwartung kommt aus dem Vertrag, nicht aus einer Kopie des Implementierungsalgorithmus. Wenn Test und Anwendung dieselbe falsche Formel enthalten, bestätigen sie sich sonst gegenseitig.

## C. C# als Anker: Grenzfälle als Testtabelle

Das folgende **Lernbeispiel** benötigt ein NUnit-Testprojekt mit passendem Testadapter. Es ist keine bereits vorhandene Pink-Robin-Klasse:

```csharp
using NUnit.Framework;

public static class TouchValidation
{
    public static bool IsValidDuration(double milliseconds) =>
        double.IsFinite(milliseconds)
        && milliseconds >= 0
        && milliseconds <= 5000;
}

[TestFixture]
public sealed class TouchValidationTests
{
    [TestCase(0.0, true)]
    [TestCase(250.0, true)]
    [TestCase(5000.0, true)]
    [TestCase(-1.0, false)]
    [TestCase(5001.0, false)]
    [TestCase(double.NaN, false)]
    [TestCase(double.PositiveInfinity, false)]
    public void Duration_RespectsAllowedRange(double input, bool expected)
    {
        // Act
        bool actual = TouchValidation.IsValidDuration(input);

        // Assert
        Assert.That(actual, Is.EqualTo(expected));
    }
}
```

Die Tabelle deckt erlaubte Grenzen, typische Werte und ungültige Zahlen ab. Die Regel „0 bis 5000 ms“ ist hier ein eigenständiger Validierungsvertrag aus dem Lernbuch und keine Obergrenze für die tatsächliche Dauer eines Core-Kopfkontakts.

Für asynchronen Code gibt eine NUnit-Testmethode beispielsweise `async Task` zurück. Innerhalb des Tests auf den relevanten Task warten und erst danach prüfen. Ein gestarteter, aber nicht beobachteter Task kann erst nach vermeintlichem Testende fehlschlagen. Normale asynchrone Tests nicht als `async void` schreiben. Für ein passend eingerichtetes .NET-Testprojekt führt `dotnet test` die Tests aus. [Microsoft: NUnit mit .NET](https://learn.microsoft.com/dotnet/core/testing/unit-testing-csharp-with-nunit)

**Achtung, C#-Gehirn:** `Debug.Assert` ist kein Ersatz für eine Testframework-Assertion, deren Fehler der Testlauf zuverlässig als Fehlschlag meldet.

## D. Test-Doubles: nur die nötige Umgebung ersetzen

Ein **Stub** liefert vorbereitete Antworten, etwa eine festgelegte Sensorlesung. Ein **Fake** hat eine vereinfachte funktionsfähige Umsetzung, etwa eine Queue im Speicher. Ein **Mock** prüft vereinbarte Interaktionen. Die Begriffe werden je nach Bibliothek unterschiedlich verwendet; wichtig ist die Aufgabe im konkreten Test.

Bei Pink Robin brauchen wir für einen Core-Verhaltenstest weder einen echten Sensor noch HTTP. Ein Ereignis und ein Zeitwert reichen. Ein Adaptertest kann dagegen einen künstlichen Empfänger nutzen, der die ausgegebenen Snapshots sammelt.

Nicht jede interne Methode mocken. Sonst scheitern Tests bei sinnvollen Umstrukturierungen, obwohl das beobachtbare Verhalten gleich bleibt. Interaktionen gezielt prüfen, wenn sie selbst Teil des Vertrags sind, beispielsweise „Handle genau einmal freigeben“ oder „nach Abmeldung keine weitere neue Benachrichtigung starten“.

## E. TypeScript/JavaScript: Nachrichten und Promise-Fehler testen

Die folgende Datei verwendet nur JavaScript-Syntax, die auch in TypeScript nutzbar ist. Als `touch.test.mjs` läuft sie mit einem aktuellen Node.js über `node --test touch.test.mjs`, ohne zusätzliche Pakete. In einem streng typgeprüften TypeScript-Projekt ergänzen wir passende Typen und verwenden dessen eingerichteten Build- und Testweg.

Die kleine Prüfung ist ein **Lernbeispiel für einen bereits geparsten Wert**, kein Ersatz für die vollständige JSON-Grenze aus Kapitel 12:

```javascript
import test from 'node:test';
import assert from 'node:assert/strict';

function isHeartTouch(value) {
    return typeof value === 'object'
        && value !== null
        && !Array.isArray(value)
        && Object.keys(value).length === 2
        && Object.hasOwn(value, 'type')
        && Object.hasOwn(value, 'target')
        && value.type === 'touch'
        && value.target === 'heart';
}

test('Herzberührung erfüllt den Nachrichtenvertrag', () => {
    assert.equal(isHeartTouch({ type: 'touch', target: 'heart' }), true);
});

test('Ungültige Nachrichten werden abgelehnt', () => {
    const invalid = [
        null, [], true,
        { type: 'touch' },
        { type: 'touch', target: 42 },
        { type: 'touch', target: 'head' },
        { type: 'touch', target: 'heart', extra: 1 }
    ];
    for (const value of invalid) {
        assert.equal(isHeartTouch(value), false, JSON.stringify(value));
    }
});

async function loadMood(load) {
    const state = await load();
    return state.mood;
}

test('Laden liefert den Mood aus dem Ergebnis', async () => {
    const result = await loadMood(async () => ({ mood: 'curious' }));
    assert.equal(result, 'curious');
});

test('Ladefehler bleiben für die Aufruferin beobachtbar', async () => {
    const expected = new Error('Backend nicht erreichbar');
    await assert.rejects(
        loadMood(async () => { throw expected; }),
        error => error === expected
    );
});
```

Der asynchrone Test wartet auf die Fehlerprüfung selbst. Ohne `await` oder Rückgabe des entsprechenden Promise kann die Prüfung ausserhalb des gewünschten Testablaufs liegen. Der [Node-Test-Runner](https://nodejs.org/api/test.html) beobachtet zurückgegebene Promises. Browser-DOM und echte UI-Komponenten benötigen zusätzlich eine passende Browser- oder DOM-Testumgebung; Node allein simuliert keine vollständige Oberfläche.

Bei konkurrierenden UI-Anfragen die Reihenfolge kontrolliert vorgeben: Anfrage A starten, B starten, B zuerst abschliessen, danach A. Dann prüfen, dass A die Anzeige nicht überschreibt. Das ist zuverlässiger als zufällige Netzwerkverzögerungen und ein anschliessendes langes Warten.

## F. C++17: Zeitgrenzen ohne sleep testen

Dieses vollständige Programm verwendet die echte [Core-API](../core/include/pinkrobin/core.hpp). Die Prüfungen bleiben auch mit `NDEBUG` aktiv:

```cpp
#include "pinkrobin/core.hpp"
#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error{message};
}

int main()
{
    try {
        // Arrange
        pinkrobin::Core core{0};

        // Act: Berührung bei 100 ms
        require(core.dispatch(pinkrobin::Event::heart_touched, 100)
                    == pinkrobin::Result::ok, "Herzereignis abgelehnt");

        // Assert: unmittelbar vor und genau an der Ablaufgrenze
        require(core.advance_to(6099) == pinkrobin::Result::ok,
                "Zeitfortschritt abgelehnt");
        require(core.snapshot().mood == pinkrobin::Mood::happy,
                "Freude endete zu frueh");

        require(core.advance_to(6100) == pinkrobin::Result::ok,
                "Grenzzeit abgelehnt");
        require(core.snapshot().mood == pinkrobin::Mood::curious,
                "Freude endete nicht nach 6000 ms");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
```

Es vergehen keine echten sechs Sekunden. `advance_to` erhält direkt den gewünschten Zeitpunkt. Ein Snapshot allein bewegt die Core-Zeit nicht weiter. Die relevante Grenze liegt hier bei `100 + 6000 = 6100`, nicht bei absolut 6000 ms.

**Achtung, C#-Gehirn:** C++-`assert` kann durch `NDEBUG` vollständig entfallen, inklusive der Auswertung seines Ausdrucks. Darin deshalb keine für den Ablauf nötigen Aufrufe verstecken. Ein selbst gebauter Test muss Fehler über einen von null verschiedenen Exitcode an den Runner melden. Die vorhandenen Core-Tests verwenden dafür explizite `CHECK`-Prüfungen.

## G. Die vorhandenen Pink-Robin-Tests

Das Repository hat bereits native Tests, die über [CMake und CTest](../core/CMakeLists.txt) eingebunden sind. Vom Repository-Stamm:

```sh
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build --config Release
ctest --test-dir core/build -C Release --output-on-failure
```

Die [Verhaltenstests](../core/tests/core_tests.cpp) prüfen unter anderem Aufwachen, Herz- und Kopfreaktionen, Unterbrechung, Neustart, unabhängige Instanzen und rückwärts laufende Zeit. Die [C-ABI-Tests](../core/tests/c_api_tests.c) prüfen die native Grenze aus einem echten C-Aufrufer, darunter Layout, Nullargumente, Puffergrösse und Statuscodes.

Diese Prüfungen ersetzen keine spätere C#-P/Invoke-Integration: Auch die tatsächlich verwendete Bibliothek, Aufrufkonvention, Struct-Abbildung und Handle-Lebensdauer müssen zusammen getestet werden. Eine funktionierende ABI allein beweist ebenso wenig, dass JSON richtig validiert oder die Oberfläche korrekt aktualisiert wird.

| Testebene | Konkrete Pink-Robin-Frage |
|---|---|
| Unit-/Verhaltenstest | Endet die Herzreaktion an der richtigen Zeitgrenze? |
| Adaptertest | Wird ein gültiger Heart-Request genau auf das gewünschte Core-Ereignis abgebildet? |
| Integrationstest | Funktionieren Backend und native Bibliothek mit echtem Layout und Ownership zusammen? |
| Durchgängiger UI-Test | Führt eine Berührung über die angebundenen Schichten zur erwarteten Anzeige? |

Viele gezielte kleine Tests und wenige wichtige durchgängige Abläufe ergänzen sich. Ein UI-Test soll ausdrücklich erkennen lassen, ob er nur die lokale Demo oder die echte Backend-Core-Verbindung prüft.

## H. Debugging: eine Hypothese nach der anderen

Wenn ein Test fehlschlägt, zuerst den kleinsten reproduzierbaren Ablauf festhalten: Anfangszustand, Ereignis, Zeitpunkt, erwartetes und tatsächliches Ergebnis. Den Test einzeln im Debugger starten.

- **Breakpoint:** Vor dem fehlerhaften Übergang anhalten.
- **Step over / Step into / Step out:** Im aktuellen Ablauf weitergehen, in einen Aufruf hinein oder aus ihm heraus.
- **Watch und Locals:** Zustand und Eingaben prüfen, etwa `time_ms`, `revision`, `head_contact` und Ergebniscode.
- **Call Stack:** Nachvollziehen, welcher Aufrufer diesen Pfad erreicht hat.
- **Exception-Breakpoint:** Möglichst am Entstehungsort eines unerwarteten Fehlers anhalten.

Eine einfache Frage ist oft besonders wirksam: „Welche letzte Eingabe erklärt die erste falsche Zustandsänderung?“ Bei asynchronen Abläufen zusätzlich Sitzung, Anfragekennung und Reihenfolge notieren. Ein Breakpoint verändert das Timing; verschwindet der Fehler dadurch, ist das keine Reparatur und kein Beleg für korrekte Synchronisierung.

Logs ergänzen den Debugger. Für Robin helfen Ereigniskennung, Sitzungsbezug, monotone Zeit, Ergebnis und Revision. Unnötige persönliche Daten oder rohe vertrauliche Nachrichten gehören nicht in Diagnoselogs.

## I. C++-Sanitizer: Fehler finden, die ein Ergebnisvergleich übersieht

**AddressSanitizer** findet bei ausgeführten Pfaden unter anderem Zugriffe ausserhalb von Puffern und auf bereits freigegebenen Speicher. **UndefinedBehaviorSanitizer** erkennt ausgewählte Formen undefinierten Verhaltens, beispielsweise bestimmte ungültige Operationen oder vorzeichenbehaftete Überläufe. Sie benötigen passende Compiler-/Laufzeitunterstützung und Instrumentierung. [Clang: AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html), [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)

Der Core bietet dafür bereits `ROBIN_ENABLE_SANITIZERS`. Beispiel für eine unterstützte Clang-/GCC-Umgebung:

```sh
cmake -S core -B core/build-sanitized -DCMAKE_BUILD_TYPE=Debug -DROBIN_ENABLE_SANITIZERS=ON
cmake --build core/build-sanitized
ctest --test-dir core/build-sanitized --output-on-failure
```

Dieses Beispiel nimmt einen Generator mit einer Build-Konfiguration an. Bei einem Multi-Config-Generator beim Bauen und Testen zusätzlich `--config Debug` beziehungsweise `-C Debug` verwenden. Die Repository-Option aktiviert diese Flags nicht für MSVC.

Ein fehlerfreier Sanitizer-Lauf beweist nicht die Gültigkeit aller Referenzen. Nicht ausgeführte Pfade bleiben ungeprüft; fachlich falsche Reaktionen erkennt ein Speicherwerkzeug nicht. Data Races benötigen geeignete separate Werkzeuge wie ThreadSanitizer und einen dafür unterstützten Build; ASan/UBSan allein prüfen nicht die gesamte Nebenläufigkeit.

## J. Miniübung

Erweitere das C++-Lernbeispiel gedanklich oder als eigenes Testprogramm:

1. Herzberührung bei `100` ms, zweite Berührung bei `6000` ms: Bei `6100` ms soll Robin weiterhin glücklich sein. Wann endet die verlängerte Reaktion?
2. Kopfkontakt bei `100` ms starten: Welche Ausdrücke erwartest du bei `5099` und `5100` ms?
3. Nach einem gültigen Schritt zu `1000` ms einen Schritt zu `999` ms versuchen. Prüfe Ergebniscode, Zeit und Revision vor und nach dem Versuch.
4. Ersetze gedanklich die Ablaufbedingung `>= 6000` durch `> 6000`. Welcher Grenztest muss dann fehlschlagen?

<details>
<summary>Lösung anzeigen</summary>

1. Die zweite Berührung startet die 6000 ms erneut: Ablauf bei `12000` ms. Prüfe unmittelbar davor und genau dort.
2. Vor der Fünf-Sekunden-Grenze `happy`, ab `5100` ms `heart_eyes`, solange der Kontakt weiter besteht.
3. Der Ergebniscode ist `time_reversed`. Die gespeicherte Zeit und Revision bleiben unverändert. Weitere fachlich relevante Snapshot-Felder können ebenfalls auf unveränderten Zustand geprüft werden.
4. Der Test bei exakt `6100` ms nach der ersten Berührung bei `100` ms: Mit der falschen Bedingung bliebe Robin dort noch glücklich. Das ist ein Beispiel dafür, wie eine kleine absichtliche Mutation zeigt, ob der Test die Grenze tatsächlich schützt.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Erwartungen aus dem fachlichen Vertrag ableiten, nicht die Implementierung im Test nachbauen.
- Grenzen, abgelehnte Eingaben und unveränderten Zustand nach Fehlern ausdrücklich prüfen.
- Zeit und Abschlussreihenfolge kontrollieren, statt auf echte Verzögerungen zu hoffen.
- Asynchrone Tests müssen den relevanten Abschluss und dessen Fehler beobachten.
- C++-Prüfungen müssen auch im gewünschten Build aktiv bleiben und einen Fehlerstatus liefern.
- Unit-Tests, Integration, Debugger und Sanitizer beantworten unterschiedliche Fragen.
- Ein gefundener Fehler bekommt einen reproduzierbaren Regressionstest, damit er nicht unbemerkt zurückkommt.

[Zurück zu Kapitel 12](12-serialisierung-json-und-api-grenzen.md) · [Inhaltsverzeichnis](README.md)
