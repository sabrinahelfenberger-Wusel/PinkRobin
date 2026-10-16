import { Component, inject } from "@angular/core";
import { RouterLink } from "@angular/router";
import { RobinComponent } from "./robin.component";
import { BackendStatusService } from "./backend-status.service";

@Component({
  selector: "pr-robin-page",
  standalone: true,
  imports: [RouterLink, RobinComponent],
  providers: [BackendStatusService],
  templateUrl: "./robin-page.component.html",
  styleUrl: "./robin-page.component.css",
})
export class RobinPageComponent {
  readonly backend = inject(BackendStatusService);
}
