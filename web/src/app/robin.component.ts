import { Component, DestroyRef, afterNextRender, computed, inject, signal } from '@angular/core';

type RobinMood = 'sleeping' | 'waking' | 'curious' | 'happy';

const MOOD_LABELS: Record<RobinMood, string> = {
  sleeping: 'Robin träumt gerade',
  waking: 'Robin entdeckt die Welt',
  curious: 'Robin ist wach und neugierig',
  happy: 'Robin ist glücklich und freut sich über deine Nähe',
};

@Component({
  selector: 'pr-robin',
  standalone: true,
  templateUrl: './robin.component.html',
  styleUrl: './robin.component.css',
})
export class RobinComponent {
  readonly mood = signal<RobinMood>('sleeping');
  readonly awake = computed(() => this.mood() !== 'sleeping');
  readonly blinking = signal(false);
  readonly heartGlowing = signal(false);
  readonly look = signal<'left' | 'right' | null>(null);
  readonly speech = signal('Pssst … Robin schläft noch.');
  readonly state = computed(() => MOOD_LABELS[this.mood()]);

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
    this.heartGlowing.set(false);
    this.look.set(null);

    if (this.reducedMotion?.matches) {
      this.finishGreeting();
      return;
    }

    this.mood.set('sleeping');
    this.speech.set('Pssst … Robin schläft noch.');
    this.later(() => {
      this.speech.set('Oh! Da ist ja jemand. …');
      this.mood.set('waking');
    }, 4000);
    this.later(() => this.blink(), 4800);
    this.later(() => this.look.set('left'), 5300);
    this.later(() => this.look.set('right'), 6100);
    this.later(() => {
      this.look.set(null);
      this.finishGreeting();
      this.idle();
    }, 7100);
  }

  touchHeart(): void {
    if (this.destroyed) return;
    // Touch takes over the greeting and keeps just one idle loop.
    this.clearTimers();
    this.mood.set('happy');
    this.blinking.set(false);
    this.look.set(null);
    this.speech.set('Das tut gut. Schön, dass du da bist. ♡');
    this.heartGlowing.set(!this.reducedMotion?.matches);
    this.later(() => {
      this.mood.set('curious');
      this.speech.set('Was entdecken wir als Nächstes?');
    }, 6000);
    if (this.reducedMotion?.matches) return;
    this.blink();
    this.later(() => this.heartGlowing.set(false), 850);
    this.idle();
  }

  private finishGreeting(): void {
    this.speech.set('Hallo, ich bin Robin! Schön, dass du da bist. ♡');
    this.mood.set('curious');
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
