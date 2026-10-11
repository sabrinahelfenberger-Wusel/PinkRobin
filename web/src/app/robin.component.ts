import { Component, DestroyRef, afterNextRender, inject, signal } from '@angular/core';

@Component({
  selector: 'pr-robin',
  standalone: true,
  templateUrl: './robin.component.html',
  styleUrl: './robin.component.css',
})
export class RobinComponent {
  readonly awake = signal(false);
  readonly blinking = signal(false);
  readonly look = signal<'left' | 'right' | null>(null);
  readonly speech = signal('Pssst … Robin schläft noch.');
  readonly state = signal('Robin träumt gerade');

  private readonly timers = new Set<ReturnType<typeof setTimeout>>();
  private reducedMotion?: MediaQueryList;
  private destroyed = false;

  constructor() {
    const destroyRef = inject(DestroyRef);
    afterNextRender(() => {
      this.reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
      this.reducedMotion.addEventListener('change', this.onMotionChange);
      this.greet();
    });
    destroyRef.onDestroy(() => {
      this.destroyed = true;
      this.clearTimers();
      this.reducedMotion?.removeEventListener('change', this.onMotionChange);
    });
  }

  private readonly onMotionChange = () => this.greet();

  greet(): void {
    if (this.destroyed) return;
    this.clearTimers();
    this.blinking.set(false);
    this.look.set(null);

    if (this.reducedMotion?.matches) {
      this.awake.set(true);
      this.finishGreeting();
      return;
    }

    this.awake.set(false);
    this.speech.set('Pssst … Robin schläft noch.');
    this.state.set('Robin träumt gerade');
    this.later(() => {
      this.awake.set(true);
      this.speech.set('Oh! Da ist ja jemand. …');
      this.state.set('Robin entdeckt die Welt');
    }, 1000);
    this.later(() => this.blink(), 1800);
    this.later(() => this.look.set('left'), 2300);
    this.later(() => this.look.set('right'), 3100);
    this.later(() => {
      this.look.set(null);
      this.finishGreeting();
      this.idle();
    }, 4100);
  }

  private finishGreeting(): void {
    this.speech.set('Hallo, ich bin Robin! Schön, dass du da bist. ♡');
    this.state.set('Robin ist wach und neugierig');
  }

  private blink(): void {
    if (this.reducedMotion?.matches || document.hidden) return;
    this.blinking.set(true);
    this.later(() => this.blinking.set(false), 160);
  }

  private idle(): void {
    if (this.reducedMotion?.matches || this.destroyed) return;
    this.later(() => {
      this.blink();
      this.idle();
    }, 2800 + Math.random() * 4200);
  }

  private later(action: () => void, delay: number): void {
    if (this.destroyed) return;
    const timer = setTimeout(() => {
      this.timers.delete(timer);
      if (!this.destroyed) action();
    }, delay);
    this.timers.add(timer);
  }

  private clearTimers(): void {
    this.timers.forEach(timer => clearTimeout(timer));
    this.timers.clear();
  }
}
