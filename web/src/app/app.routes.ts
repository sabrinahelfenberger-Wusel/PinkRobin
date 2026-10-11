import { Routes } from '@angular/router';
import { HomeComponent } from './home.component';
import { RobinPageComponent } from './robin-page.component';

export const routes: Routes = [
  { path: '', component: HomeComponent, title: 'Pink Robin — Ein kleiner Roboter. Eine grosse Idee.' },
  { path: 'virtual-robin', component: RobinPageComponent, title: 'Virtual Robin · Pink Robin' },
  { path: '**', redirectTo: '' },
];
