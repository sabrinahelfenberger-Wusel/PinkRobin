import { Component } from '@angular/core';
import { RouterLink } from '@angular/router';
import { RobinComponent } from './robin.component';

@Component({
  selector: 'pr-robin-page',
  standalone: true,
  imports: [RouterLink, RobinComponent],
  templateUrl: './robin-page.component.html',
  styleUrl: './robin-page.component.css',
})
export class RobinPageComponent {}
