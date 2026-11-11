import { HttpClient } from "@angular/common/http";
import { DestroyRef, Injectable, inject, signal } from "@angular/core";
import { takeUntilDestroyed } from "@angular/core/rxjs-interop";
import { firstValueFrom, timeout } from "rxjs";
import { CoreOutput, parseCoreOutput } from "./core-output";

export type RobinEvent =
  | "restart"
  | "heartTouched"
  | "headStarted"
  | "headEnded"
  | "interactionCancelled"
  | "weatherAsked"
  | "textReceived";

export interface CoreState {
  mode: "sleeping" | "waking" | "awake";
  mood: "curious" | "happy" | "affectionate";
  expression: "sleeping" | "curious" | "happy" | "heartEyes";
  headContact: boolean;
  timeMs: number;
  revision: number;
  sequence: number;
  transformation: "none" | "octopus" | "raincoat";
}
export function parseCoreState(value: unknown): CoreState {
  const s = value as CoreState;
  if (
    !s ||
    !["sleeping", "waking", "awake"].includes(s.mode) ||
    !["curious", "happy", "affectionate"].includes(s.mood) ||
    !["sleeping", "curious", "happy", "heartEyes"].includes(s.expression) ||
    !["none", "octopus", "raincoat"].includes(s.transformation) ||
    typeof s.headContact !== "boolean" ||
    ![s.timeMs, s.revision, s.sequence].every(
      (n) => Number.isSafeInteger(n) && n >= 0,
    )
  )
    throw new Error("Incompatible Core state");
  return s;
}

@Injectable()
export class RobinSessionService {
  readonly state = signal<CoreState | null>(null);
  readonly output = signal<CoreOutput | null>(null);
  readonly connection = signal<
    "checking" | "connected" | "reconnecting" | "local"
  >("checking");
  readonly textError = signal("");
  private readonly http = inject(HttpClient);
  private readonly destroyRef = inject(DestroyRef);
  private token: string | null = null;
  private sequence = 0;
  private timer?: ReturnType<typeof setTimeout>;
  private destroyed = false;
  private busy = false;
  private creating = false;
  private weatherBusy = false;
  private outputAckBusy = false;
  private outputReadToken: string | null = null;

  private recovering = false;
  private reconnectAttempt = 0;
  private readonly storageKey = "pinkrobin.session.v1";
  private restoring = false;

  private queue: {
    sequence: number;
    event: RobinEvent;
    text?: string;
  }[] = [];

  constructor() {
    this.destroyRef.onDestroy(() => {
      this.destroyed = true;
      this.close(true);
    });
  }

  private readStoredToken(): string | null {
    try {
      const token = sessionStorage.getItem(this.storageKey);

      if (token === null) return null;

      if (!/^[a-fA-F0-9]{64}$/.test(token)) {
        sessionStorage.removeItem(this.storageKey);
        return null;
      }

      return token;
    } catch {
      return null;
    }
  }

  private storeToken(token: string): void {
    try {
      sessionStorage.setItem(this.storageKey, token);
    } catch {
      // Ohne Browserspeicher bleibt die laufende Session nutzbar.
    }
  }

  private forgetStoredToken(): void {
    try {
      sessionStorage.removeItem(this.storageKey);
    } catch {
      // Nicht verfügbarer Browserspeicher verhindert keine Bereinigung.
    }
  }

  async connect(reducedMotion: boolean): Promise<void> {
    if (this.destroyed || this.creating || this.busy || this.token) return;
    this.creating = true;
    this.connection.set("checking");
    try {
      const storedToken = this.readStoredToken();

      if (storedToken) {
        this.token = storedToken;
        this.restoring = true;
        this.beginReconnect();
        await this.pump();
        return;
      }
      const created = await this.request<{ token: string; state: unknown }>(
        "POST",
        "api/v1/sessions",
        { reducedMotion },
      );
      if (!/^[a-fA-F0-9]{64}$/.test(created.token))
        throw new Error("Invalid session");
      if (this.destroyed) {
        this.release(created.token);
        return;
      }
      this.token = created.token;
      const state = parseCoreState(created.state);
      this.sequence = state.sequence;
      this.storeToken(created.token);
      this.state.set(state);
      this.connection.set("connected");
      this.schedule();
    } catch {
      if (!this.destroyed) this.fail();
    } finally {
      this.creating = false;
    }
  }

  resume(): void {
    if (!this.token || this.destroyed) return;

    this.beginReconnect();
    this.reconnectAttempt = 0;
    clearTimeout(this.timer);

    if (!this.busy) {
      void this.pump();
    }
  }

  send(event: RobinEvent, text?: string): void {
    if (!this.token || this.destroyed) return;
    if (this.connection() !== "connected") return;
    if (this.queue.length >= 32) {
      this.fail();
      return;
    }

    this.queue.push({
      sequence: ++this.sequence,
      event,
      ...(text !== undefined ? { text } : {}),
    });

    if (!this.busy) {
      clearTimeout(this.timer);
      void this.pump();
    }
  }

  sendText(text: string): boolean {
    if (!this.token || this.destroyed || this.connection() !== "connected") {
      this.textError.set("Robin ist gerade nicht verbunden.");
      return false;
    }

    if (!text.trim()) {
      this.textError.set("Schreibe Robin zuerst eine Nachricht.");
      return false;
    }

    if (new TextEncoder().encode(text).length > 160) {
      this.textError.set("Deine Nachricht ist zu lang. Bitte kürze sie.");
      return false;
    }

    this.textError.set("");
    this.send("textReceived", text);

    return this.token !== null;
  }

  async updateWeather(postalCode: string): Promise<boolean> {
    if (!this.token || this.destroyed || this.weatherBusy) {
      return false;
    }

    this.weatherBusy = true;
    const token = this.token;

    try {
      const response = await this.request<{ accepted: unknown }>(
        "POST",
        "api/v1/session/weather",
        { postalCode },
        token,
        95_000,
      );

      if (this.destroyed || this.token !== token) {
        return false;
      }

      if (typeof response?.accepted !== "boolean") {
        throw new Error("Invalid weather response");
      }

      return response.accepted;
    } finally {
      this.weatherBusy = false;
    }
  }

  async acknowledgeOutput(sequence: string): Promise<boolean> {
    const token = this.token;

    if (
      !token ||
      this.destroyed ||
      this.outputAckBusy ||
      this.output()?.sequence !== sequence
    ) {
      return false;
    }

    this.outputAckBusy = true;

    try {
      await this.request<void>(
        "POST",
        "api/v1/session/outputs/ack",
        { sequence },
        token,
      );

      if (this.destroyed || this.token !== token) return false;

      if (this.output()?.sequence === sequence) {
        this.output.set(null);
      }

      return true;
    } catch {
      // Ausgabe behalten: Die Bestätigung darf wiederholt werden.
      return false;
    } finally {
      this.outputAckBusy = false;
    }
  }

  private async readOutput(token: string): Promise<void> {
    if (
      this.destroyed ||
      this.token !== token ||
      this.output() !== null ||
      this.outputReadToken === token
    ) {
      return;
    }

    this.outputReadToken = token;

    try {
      const response = await this.request<unknown>(
        "GET",
        "api/v1/session/outputs/next",
        undefined,
        token,
        95_000,
      );

      if (this.destroyed || this.token !== token) return;

      this.output.set(parseCoreOutput(response));
    } catch {
      // Lesen darf wiederholt werden. Wartende Core-Ausgaben
      // werden dadurch weder bestätigt noch entfernt.
    } finally {
      if (this.outputReadToken === token) {
        this.outputReadToken = null;
      }
    }
  }

  private async pump(): Promise<void> {
    if (!this.token || this.destroyed || this.busy) return;

    this.busy = true;

    const token = this.token;
    const resuming = this.recovering;

    // Erst nach bestätigter Verarbeitung aus der Queue entfernen.
    const command = resuming ? undefined : this.queue[0];

    let responseReceived = false;

    try {
      const response = await this.request<unknown>(
        command ? "POST" : "GET",
        command ? "api/v1/session/events" : "api/v1/session",
        command,
        token,
      );

      if (this.destroyed || this.token !== token) return;

      responseReceived = true;
      const state = parseCoreState(response);

      if (resuming) {
        if (this.restoring) {
          // Nach Neuladen gilt die bestätigte Serversequenz.
          // Nicht gespeicherte Eingaben werden nicht nachgespielt.
          this.sequence = state.sequence;
          this.queue = [
            {
              sequence: ++this.sequence,
              event: "interactionCancelled",
            },
          ];
          this.restoring = false;
        }
        const firstPending = this.queue[0];

        const minimumSequence = firstPending
          ? firstPending.sequence - 1
          : this.sequence;

        if (
          state.sequence < minimumSequence ||
          state.sequence > this.sequence
        ) {
          throw new Error("Incompatible session command sequence");
        }

        // Bereits angenommene Befehle nicht erneut senden.
        while (
          this.queue.length > 0 &&
          this.queue[0].sequence <= state.sequence
        ) {
          this.queue.shift();
        }

        // Alte Berührungen oder Neustarts nach einer Pause nicht nachspielen.
        // Die Befehlsnummern bleiben dabei lückenlos.
        this.queue = this.queue.map((pending) =>
          pending.event === "textReceived" || pending.event === "weatherAsked"
            ? pending
            : {
                sequence: pending.sequence,
                event: "interactionCancelled" as RobinEvent,
              },
        );

        if (
          state.headContact &&
          !this.queue.some(
            (pending) => pending.event === "interactionCancelled",
          )
        ) {
          this.queue.push({
            sequence: ++this.sequence,
            event: "interactionCancelled",
          });
        }

        this.recovering = false;
        this.reconnectAttempt = 0;
        this.connection.set("connected");
      } else if (command) {
        if (state.sequence !== command.sequence) {
          throw new Error("Unexpected command acknowledgement");
        }

        this.queue.shift();
      }

      this.state.set(state);
      void this.readOutput(token);
    } catch (error) {
      if (this.destroyed || this.token !== token) return;

      const status =
        typeof error === "object" &&
        error !== null &&
        "status" in error &&
        typeof error.status === "number"
          ? error.status
          : undefined;

      const terminalHttpError =
        status !== undefined &&
        status >= 400 &&
        status < 500 &&
        status !== 408 &&
        status !== 409 &&
        status !== 429;

      if (responseReceived || terminalHttpError) {
        this.fail();
      } else {
        this.beginReconnect();
      }
    } finally {
      this.busy = false;

      if (this.token && !this.destroyed) {
        this.schedule(this.queue.length ? 0 : 250);
      }
    }
  }

  private request<T>(
    method: string,
    url: string,
    body?: unknown,
    token?: string,
    timeoutMs = 5000,
  ): Promise<T> {
    return firstValueFrom(
      this.http
        .request<T>(method, url, {
          body,
          headers: token ? { "X-Robin-Session": token } : {},
        })
        .pipe(timeout(timeoutMs), takeUntilDestroyed(this.destroyRef)),
    );
  }

  private beginReconnect(): void {
    if (!this.token || this.destroyed) return;

    this.recovering = true;
    this.connection.set("reconnecting");
  }

  private schedule(delay = 250): void {
    clearTimeout(this.timer);

    if (!this.token || this.destroyed) return;

    if (this.recovering) {
      const delays = [1000, 2000, 4000, 8000, 15000];
      const index = Math.min(this.reconnectAttempt, delays.length - 1);

      delay = delays[index];
      this.reconnectAttempt = Math.min(
        this.reconnectAttempt + 1,
        delays.length - 1,
      );
    }

    this.timer = setTimeout(() => void this.pump(), delay);
  }

  private fail(): void {
    this.close();
    this.state.set(null);
    this.connection.set("local");
  }

  private close(preserveSession = false): void {
    clearTimeout(this.timer);

    this.recovering = false;
    this.reconnectAttempt = 0;
    this.restoring = false;

    this.queue = [];
    this.output.set(null);

    if (!preserveSession) {
      this.forgetStoredToken();

      if (this.token) {
        this.release(this.token);
      }
    }

    this.token = null;
    this.outputReadToken = null;
  }

  private release(token: string): void {
    // Navigation can cancel Angular requests; keepalive is best effort, TTL is the backstop.
    void fetch(new URL("api/v1/session", document.baseURI), {
      method: "DELETE",
      headers: { "X-Robin-Session": token },
      keepalive: true,
    }).catch(() => {});
  }
}
