import { Component, effect, inject, untracked } from "@angular/core";
import { DecimalPipe } from "@angular/common";
import { RouterLink } from "@angular/router";
import { RobinComponent } from "./robin.component";
import { BackendStatusService } from "./backend-status.service";
import { WeatherService } from "./weather.service";
import { RobinSessionService } from "./robin-session.service";

@Component({
  selector: "pr-robin-page",
  standalone: true,
  imports: [RouterLink, RobinComponent, DecimalPipe],
  providers: [BackendStatusService, WeatherService, RobinSessionService],
  templateUrl: "./robin-page.component.html",
  styleUrl: "./robin-page.component.css",
})
export class RobinPageComponent {
  readonly backend = inject(BackendStatusService);
  readonly weather = inject(WeatherService);
  readonly session = inject(RobinSessionService);

  constructor() {
    effect(() => {
      const forecast = this.weather.weather();
      const connected = this.session.connection() === "connected";

      if (!forecast || !connected) {
        return;
      }

      if (Date.parse(forecast.validTo) <= Date.now()) {
        return;
      }

      untracked(() => {
        void this.updateSessionWeather(forecast.postalCode);
      });
    });
  }

  // Die Methode steht innerhalb der Klasse,
  // aber ausserhalb anderer Methoden.
  async updateSessionWeather(postalCode: string): Promise<void> {
    try {
      await this.session.updateWeather(postalCode);
    } catch {
      // Keine automatische Wiederholung bei einem unklaren Netzwerkfehler.
    }
  }
}
