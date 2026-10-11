(() => {
  const stage = document.querySelector('.stage');
  const speech = document.querySelector('#speech');
  const state = document.querySelector('#state');
  const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
  const timers = new Set();
  function later(action, delay) {
    const timer = window.setTimeout(() => { timers.delete(timer); action(); }, delay);
    timers.add(timer);
  }
  function blink() {
    if (reducedMotion.matches || document.hidden) return;
    stage.classList.add('blinking');
    later(() => stage.classList.remove('blinking'), 160);
  }
  function idle() {
    later(() => { blink(); idle(); }, 2800 + Math.random() * 4200);
  }
  function greet() {
    timers.forEach(window.clearTimeout);
    timers.clear();
    stage.classList.remove('awake', 'blinking', 'look-left', 'look-right');
    speech.textContent = 'Pssst … Robin schläft noch.';
    state.textContent = 'Robin träumt gerade';
    later(() => {
      stage.classList.add('awake');
      speech.textContent = 'Oh! Da ist ja jemand. …';
      state.textContent = 'Robin entdeckt die Welt';
    }, reducedMotion.matches ? 0 : 1000);
    if (!reducedMotion.matches) {
      later(blink, 1800);
      later(() => stage.classList.add('look-left'), 2300);
      later(() => { stage.classList.remove('look-left'); stage.classList.add('look-right'); }, 3100);
    }
    later(() => {
      stage.classList.remove('look-left', 'look-right');
      speech.textContent = 'Hallo, ich bin Robin! Schön, dass du da bist. ♡';
      state.textContent = 'Robin ist wach und neugierig';
      idle();
    }, reducedMotion.matches ? 100 : 4100);
  }
  document.querySelector('#restart').addEventListener('click', greet);
  reducedMotion.addEventListener('change', greet);
  greet();
})();
