import {
  Component,
  DestroyRef,
  afterNextRender,
  computed,
  effect,
  inject,
  signal,
} from "@angular/core";

import { CoreState, RobinSessionService } from "./robin-session.service";

type RobinMood = "sleeping" | "waking" | "curious" | "happy" | "affectionate";

const MOOD_LABELS: Record<RobinMood, string> = {
  sleeping: "Robin träumt gerade",
  waking: "Robin entdeckt die Welt",
  curious: "Robin ist wach und neugierig",
  happy: "Robin ist glücklich und freut sich über deine Nähe",
  affectionate: "Robin geniesst das Streicheln mit Herzaugen",
};

@Component({
  selector: "pr-robin",
  standalone: true,
  templateUrl: "./robin.component.html",
  styleUrl: "./robin.component.css",
})
export class RobinComponent {
  readonly session = inject(RobinSessionService);
  private coreRevision: number | null = null;
  private coreActive = false;
  private coreGreeting = true;
  readonly mood = signal<RobinMood>("sleeping");
  readonly petting = signal(false);
  readonly smiling = computed(
    () => this.mood() === "happy" || this.mood() === "affectionate",
  );
  private headHeld = false;
  private activePointer: number | null = null;
  readonly awake = computed(() => this.mood() !== "sleeping");
  readonly blinking = signal(false);
  readonly heartGlowing = signal(false);
  readonly look = signal<"left" | "right" | null>(null);
  readonly speech = signal("Pssst … Robin schläft noch.");
  readonly state = computed(() => MOOD_LABELS[this.mood()]);
  readonly transformation = signal<"none" | "octopus" | "raincoat">("none");

  private readonly timers = new Set<ReturnType<typeof setTimeout>>();
  private reducedMotion?: MediaQueryList;
  private destroyed = false;

  constructor() {
    const destroyRef = inject(DestroyRef);
    afterNextRender(() => {
      this.reducedMotion = window.matchMedia(
        "(prefers-reduced-motion: reduce)",
      );
      this.reducedMotion.addEventListener("change", this.onMotionChange);
      this.greet();
      void this.session.connect(this.reducedMotion.matches);
    });
    effect(() => {
      const state = this.session.state();
      if (state) {
        this.coreActive = true;
        if (state.revision !== this.coreRevision) {
          this.coreRevision = state.revision;
          this.applyCoreState(state);
        }
      } else if (this.coreActive) {
        this.transformation.set("none");
        this.coreGreeting = true;
        this.headHeld = false;
        this.coreActive = false;
        this.coreRevision = null;
        this.greet();
      }
    });
    destroyRef.onDestroy(() => {
      this.destroyed = true;
      this.clearTimers();
      this.reducedMotion?.removeEventListener("change", this.onMotionChange);
    });
  }

  private readonly onMotionChange = () => {
    if (this.session.connection() === "connected") {
      this.clearTimers();
      this.blinking.set(false);
      this.look.set(null);
      this.heartGlowing.set(false);
      if (!this.reducedMotion?.matches) this.idle();
    } else this.greet();
  };

  reconnect(): void {
    void this.session.connect(this.reducedMotion?.matches ?? false);
  }

  private applyCoreState(state: CoreState): void {
    this.transformation.set(state.transformation);
    this.clearTimers();
    this.blinking.set(false);
    this.look.set(null);
    this.heartGlowing.set(false);
    this.petting.set(state.headContact);
    if (!state.headContact && !this.headHeld) this.activePointer = null;
    const mood: RobinMood =
      state.mode === "sleeping"
        ? "sleeping"
        : state.mode === "waking"
          ? "waking"
          : state.mood;
    this.mood.set(mood);
    if (state.mode !== "awake") this.coreGreeting = true;
    this.speech.set(
      mood === "sleeping"
        ? "Pssst … Robin schläft noch."
        : mood === "waking"
          ? "Oh! Da ist ja jemand. …"
          : mood === "affectionate"
            ? "Bei dir fühle ich mich wohl. ♡"
            : mood === "happy"
              ? state.headContact
                ? "Oh, das fühlt sich schön an. ♡"
                : "Das tut gut. Schön, dass du da bist. ♡"
              : this.coreGreeting
                ? "Hallo, ich bin Robin! Schön, dass du da bist. ♡"
                : "Was entdecken wir als Nächstes?",
    );
    if (
      state.mode === "awake" &&
      state.mood === "curious" &&
      !state.headContact
    ) {
      switch (state.transformation) {
        case "raincoat":
          this.speech.set("Gut, dass ich meinen Regenmantel dabei habe!");
          break;

        case "octopus":
          this.speech.set(
            "Bei dem Wetter Verwandle ich mich besser in einen Tintenfisch!",
          );
          break;
      }
    }
    if (mood !== "sleeping") this.blink();
    if (mood === "waking" && !this.reducedMotion?.matches) {
      this.later(() => this.look.set("left"), 1300);
      this.later(() => this.look.set("right"), 2100);
    }
    if (
      mood === "happy" &&
      !state.headContact &&
      !this.reducedMotion?.matches
    ) {
      this.heartGlowing.set(true);
      this.later(() => this.heartGlowing.set(false), 850);
    }
    this.idle();
  }

  greet(): void {
    if (this.destroyed) return;
    if (this.session.connection() === "connected") {
      this.coreGreeting = true;
      this.headHeld = false;
      this.session.send("restart");
      return;
    }
    this.clearTimers();
    this.petting.set(false);
    this.activePointer = null;
    this.blinking.set(false);
    this.heartGlowing.set(false);
    this.look.set(null);

    if (this.reducedMotion?.matches) {
      this.finishGreeting();
      return;
    }

    this.mood.set("sleeping");
    this.speech.set("Pssst … Robin schläft noch.");
    this.later(() => {
      this.speech.set("Oh! Da ist ja jemand. …");
      this.mood.set("waking");
    }, 4000);
    this.later(() => this.blink(), 4800);
    this.later(() => this.look.set("left"), 5300);
    this.later(() => this.look.set("right"), 6100);
    this.later(() => {
      this.look.set(null);
      this.finishGreeting();
      this.idle();
    }, 7100);
  }

  touchHeart(): void {
    if (this.destroyed) return;
    if (this.session.connection() === "connected") {
      this.headHeld = false;
      this.session.send("heartTouched");
      return;
    }
    // Touch takes over the greeting and keeps just one idle loop.
    this.clearTimers();
    this.petting.set(false);
    this.activePointer = null;
    this.mood.set("happy");
    this.blinking.set(false);
    this.look.set(null);
    this.speech.set("Das tut gut. Schön, dass du da bist. ♡");
    this.heartGlowing.set(!this.reducedMotion?.matches);
    this.later(() => {
      this.mood.set("curious");
      this.speech.set("Was entdecken wir als Nächstes?");
    }, 6000);
    if (this.reducedMotion?.matches) return;
    this.blink();
    this.later(() => this.heartGlowing.set(false), 850);
    this.idle();
  }

  onHeadPointerDown(event: PointerEvent): void {
    if (!event.isPrimary || event.button !== 0) return;
    this.activePointer = event.pointerId;
    (event.currentTarget as HTMLElement).setPointerCapture(event.pointerId);
    this.startPetting();
  }

  onHeadPointerMove(event: PointerEvent): void {
    if (event.pointerId !== this.activePointer) return;
    const bounds = (event.currentTarget as HTMLElement).getBoundingClientRect();
    if (
      event.clientX < bounds.left ||
      event.clientX > bounds.right ||
      event.clientY < bounds.top ||
      event.clientY > bounds.bottom
    )
      this.stopPetting();
  }

  onPointerEnd(event: PointerEvent): void {
    if (event.pointerId === this.activePointer) this.stopPetting();
  }

  onHeadKeyDown(event: KeyboardEvent): void {
    if (event.key !== " " && event.key !== "Enter") return;
    event.preventDefault();
    this.startPetting();
  }

  onHeadKeyUp(event: KeyboardEvent): void {
    if (event.key === " " || event.key === "Enter") this.stopPetting();
  }

  onVisibilityChange(): void {
    if (document.hidden) this.stopPetting();
  }

  startPetting(): void {
    if (this.destroyed) return;
    if (this.session.connection() === "connected") {
      if (!this.headHeld) {
        this.headHeld = true;
        this.session.send("headStarted");
      }
      return;
    }
    if (this.petting()) return;
    this.clearTimers();
    this.petting.set(true);
    this.mood.set("happy");
    this.look.set(null);
    this.blinking.set(false);
    this.heartGlowing.set(false);
    this.speech.set("Oh, das fühlt sich schön an. ♡");
    this.blink();
    this.later(() => {
      this.mood.set("affectionate");
      this.speech.set("Bei dir fühle ich mich wohl. ♡");
    }, 5000);
  }

  stopPetting(): void {
    if (this.destroyed) return;
    if (this.session.connection() === "connected") {
      if (this.headHeld) {
        this.headHeld = false;
        this.activePointer = null;
        this.session.send("headEnded");
      }
      return;
    }
    if (!this.petting()) return;
    this.clearTimers();
    this.petting.set(false);
    this.activePointer = null;
    this.mood.set("curious");
    this.blinking.set(false);
    this.heartGlowing.set(false);
    this.look.set(null);
    this.speech.set("Was entdecken wir als Nächstes?");
    this.idle();
  }

  private finishGreeting(): void {
    this.speech.set("Hallo, ich bin Robin! Schön, dass du da bist. ♡");
    this.mood.set("curious");
  }

  private blink(): void {
    if (this.reducedMotion?.matches || document.hidden) return;
    this.blinking.set(true);
    this.later(() => this.blinking.set(false), 160);
  }

  private idle(): void {
    if (this.reducedMotion?.matches || this.destroyed) return;
    this.later(
      () => {
        this.blink();
        this.idle();
      },
      2800 + Math.random() * 4200,
    );
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
    this.timers.forEach((timer) => clearTimeout(timer));
    this.timers.clear();
  }
}
