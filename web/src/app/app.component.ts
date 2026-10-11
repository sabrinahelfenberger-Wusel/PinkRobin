import { Component } from '@angular/core';
import { RobinComponent } from './robin.component';

@Component({
  selector: 'pr-root',
  standalone: true,
  imports: [RobinComponent],
  templateUrl: './app.component.html',
})
export class AppComponent {}
