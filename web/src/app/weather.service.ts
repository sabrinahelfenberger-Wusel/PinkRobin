import { HttpClient } from "@angular/common/http";
import {
  DestroyRef,
  Injectable,
  afterNextRender,
  inject,
  signal,
} from "@angular/core";
import { takeUntilDestroyed } from "@angular/core/rxjs-interop";
import {
  BehaviorSubject,
  catchError,
  exhaustMap,
  map,
  of,
  switchMap,
  tap,
  timeout,
  timer,
} from "rxjs";

import { RobinWeather, parseWeather } from "./weather";

@Injectable()
export class WeatherService {
  readonly weather = signal<RobinWeather | null>(null);
  readonly state = signal<"loading" | "ready" | "unavailable">("loading");
  readonly error = signal("");
  private readonly http = inject(HttpClient);
  private readonly destroyRef = inject(DestroyRef);
  private readonly location = new BehaviorSubject<string | null>(null);

  constructor() {
    afterNextRender(() => {
      this.location
        .pipe(
          switchMap((postalCode) =>
            timer(0, 60_000).pipe(
              exhaustMap(() => {
                // Clear expired values before polling so future reactions cannot use them.
                const previous = this.weather();
                if (previous && Date.parse(previous.validTo) <= Date.now()) {
                  this.weather.set(null);
                }
                return this.http
                  .get<unknown>("/api/weather", {
                    params: postalCode ? { postalCode } : {},
                  })
                  .pipe(
                    timeout(95_000),
                    map((body) => parseWeather(body)),
                    map((forecast) =>
                      forecast &&
                      (!postalCode || forecast.postalCode === postalCode)
                        ? forecast
                        : null,
                    ),
                    catchError(() => of(null)),
                  );
              }),
            ),
          ),
          tap((forecast) => {
            this.weather.set(forecast);
            this.state.set(forecast ? "ready" : "unavailable");
          }),
          takeUntilDestroyed(this.destroyRef),
        )
        .subscribe();
    });
  }

  selectPostalCode(value: string): void {
    const postalCode = value.trim();
    if (!/^\d{4}$/.test(postalCode)) {
      this.error.set("Bitte eine vierstellige Schweizer PLZ eingeben.");
      return;
    }
    this.error.set("");
    this.weather.set(null);
    this.state.set("loading");
    this.location.next(postalCode);
  }
}
