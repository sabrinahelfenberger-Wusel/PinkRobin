import { Component, inject } from "@angular/core";
import { DecimalPipe } from "@angular/common";
import { WeatherService } from "./weather.service";
import { RouterLink } from "@angular/router";
import { RobinComponent } from "./robin.component";
import { BackendStatusService } from "./backend-status.service";

@Component({
  selector: "pr-robin-page",
  standalone: true,
  imports: [RouterLink, RobinComponent, DecimalPipe],
  providers: [BackendStatusService, WeatherService],
  templateUrl: "./robin-page.component.html",
  styleUrl: "./robin-page.component.css",
})
export class RobinPageComponent {
  readonly weather = inject(WeatherService);
  readonly backend = inject(BackendStatusService);
}
