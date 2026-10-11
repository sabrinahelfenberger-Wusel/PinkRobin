import { Component, provideZonelessChangeDetection, signal } from "@angular/core";
import { bootstrapApplication } from "@angular/platform-browser";
@Component({selector:"pr-root",standalone:true,template:`<main><div class="robin"><div class="eyes"><span></span><span></span></div><div class="mouth"></div></div><div class="body">♥</div><h1>Hallo, ich bin Robin! 🐦</h1><p>{{message()}}</p><button (click)="greet()">Sag Hallo</button><small>Angular-Prototyp · Backend folgt</small></main>`})
export class AppComponent { message=signal("Schön, dass du da bist!"); greet(){this.message.set("Pip pip! 💗");} }
bootstrapApplication(AppComponent, { providers: [provideZonelessChangeDetection()] }).catch(console.error);
