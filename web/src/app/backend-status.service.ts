import { HttpClient } from "@angular/common/http";
import {
  DestroyRef,
  Injectable,
  afterNextRender,
  inject,
  signal,
} from "@angular/core";
import { takeUntilDestroyed } from "@angular/core/rxjs-interop";
import { catchError, exhaustMap, map, of, timeout, timer } from "rxjs";
import { BackendStatus, parseBackendStatus } from "./backend-status";

type Connection = "checking" | "connected" | "unreachable" | "incompatible";

// Scoped to the Robin page: leaving the page stops polling and pending requests.
@Injectable()
export class BackendStatusService {
  readonly connection = signal<Connection>("checking");
  readonly status = signal<BackendStatus | null>(null);
  private readonly http = inject(HttpClient);
  private readonly destroyRef = inject(DestroyRef);

  constructor() {
    afterNextRender(() => {
      timer(0, 30_000)
        .pipe(
          exhaustMap(() =>
            this.http.get<unknown>("api/v1/status").pipe(
              timeout(5000),
              map((body) => ({
                reachable: true,
                status: parseBackendStatus(body),
              })),
              catchError(() => of({ reachable: false, status: null })),
            ),
          ),
          takeUntilDestroyed(this.destroyRef),
        )
        .subscribe((result) => {
          this.status.set(result.status);
          this.connection.set(
            !result.reachable
              ? "unreachable"
              : result.status
                ? "connected"
                : "incompatible",
          );
        });
    });
  }
}
