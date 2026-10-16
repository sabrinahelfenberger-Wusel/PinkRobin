import { HttpClient } from '@angular/common/http';
import { DestroyRef, Injectable, inject, signal } from '@angular/core';
import { takeUntilDestroyed } from '@angular/core/rxjs-interop';
import { firstValueFrom, timeout } from 'rxjs';

export type RobinEvent = 'restart' | 'heartTouched' | 'headStarted' | 'headEnded' | 'interactionCancelled';
export interface CoreState {
  mode: 'sleeping' | 'waking' | 'awake';
  mood: 'curious' | 'happy' | 'affectionate';
  expression: 'sleeping' | 'curious' | 'happy' | 'heartEyes';
  headContact: boolean; timeMs: number; revision: number; sequence: number;
}
export function parseCoreState(value: unknown): CoreState {
  const s = value as CoreState;
  if (!s || !['sleeping', 'waking', 'awake'].includes(s.mode) ||
      !['curious', 'happy', 'affectionate'].includes(s.mood) ||
      !['sleeping', 'curious', 'happy', 'heartEyes'].includes(s.expression) ||
      typeof s.headContact !== 'boolean' ||
      ![s.timeMs, s.revision, s.sequence].every(n => Number.isSafeInteger(n) && n >= 0))
    throw new Error('Incompatible Core state');
  return s;
}

@Injectable()
export class RobinSessionService {
  readonly state = signal<CoreState | null>(null);
  readonly connection = signal<'checking' | 'connected' | 'local'>('checking');
  private readonly http = inject(HttpClient);
  private readonly destroyRef = inject(DestroyRef);
  private token: string | null = null;
  private sequence = 0;
  private queue: { sequence: number; event: RobinEvent }[] = [];
  private timer?: ReturnType<typeof setTimeout>;
  private destroyed = false;
  private busy = false;
  private creating = false;

  constructor() {
    this.destroyRef.onDestroy(() => {
      this.destroyed = true;
      this.close();
    });
  }

  async connect(reducedMotion: boolean): Promise<void> {
    if (this.destroyed || this.creating || this.busy || this.token) return;
    this.creating = true;
    this.connection.set('checking');
    try {
      const created = await this.request<{ token: string; state: unknown }>('POST', 'api/v1/sessions', { reducedMotion });
      if (!/^[a-fA-F0-9]{64}$/.test(created.token)) throw new Error('Invalid session');
      if (this.destroyed) { this.release(created.token); return; }
      this.token = created.token;
      const state = parseCoreState(created.state);
      this.sequence = state.sequence;
      this.state.set(state);
      this.connection.set('connected');
      this.schedule();
    } catch {
      if (!this.destroyed) this.fail();
    } finally { this.creating = false; }
  }

  send(event: RobinEvent): void {
    if (!this.token || this.destroyed) return;
    if (this.queue.length >= 32) { this.fail(); return; }
    this.queue.push({ sequence: ++this.sequence, event });
    if (!this.busy) { clearTimeout(this.timer); void this.pump(); }
  }

  private async pump(): Promise<void> {
    if (!this.token || this.destroyed || this.busy) return;
    this.busy = true;
    const token = this.token;
    const command = this.queue.shift();
    try {
      // Exactly one request at a time: older poll responses cannot overwrite events.
      const state = await this.request<unknown>(command ? 'POST' : 'GET',
        command ? 'api/v1/session/events' : 'api/v1/session', command, token);
      if (!this.destroyed && this.token === token) this.state.set(parseCoreState(state));
    } catch {
      // No automatic replay/reconnect: an uncertain command must not affect a new session.
      if (!this.destroyed && this.token === token) this.fail();
    } finally {
      this.busy = false;
      if (this.token && !this.destroyed) this.schedule(this.queue.length ? 0 : 250);
    }
  }

  private request<T>(method: string, url: string, body?: unknown, token?: string): Promise<T> {
    return firstValueFrom(this.http.request<T>(method, url, {
      body, headers: token ? { 'X-Robin-Session': token } : {},
    }).pipe(timeout(5000), takeUntilDestroyed(this.destroyRef)));
  }
  private schedule(delay = 250): void { this.timer = setTimeout(() => void this.pump(), delay); }
  private fail(): void {
    this.close();
    this.state.set(null);
    this.connection.set('local');
  }
  private close(): void {
    clearTimeout(this.timer);
    this.queue = [];
    if (this.token) this.release(this.token);
    this.token = null;
  }
  private release(token: string): void {
    // Navigation can cancel Angular requests; keepalive is best effort, TTL is the backstop.
    void fetch(new URL('api/v1/session', document.baseURI), {
      method: 'DELETE', headers: { 'X-Robin-Session': token }, keepalive: true,
    }).catch(() => {});
  }
}
