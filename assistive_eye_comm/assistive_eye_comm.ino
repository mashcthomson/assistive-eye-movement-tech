/*
 * ============================================================================
 *  Assistive Eye Movement Communication Device
 *  Bachelor of IT Capstone Project - Sri Sairam Engineering College, 2020
 *
 *  Low-cost AAC (Augmentative and Alternative Communication) device for
 *  patients with ALS / locked-in syndrome / severe paralysis. Four IR
 *  proximity sensors positioned around one eye detect gaze direction and
 *  blinks. The patient navigates a phrase bank shown on a 16x2 LCD and
 *  "speaks" a phrase as an audible caregiver-call tone pattern.
 *
 *  NOTE: this file is a reconstruction of the original 2020 capstone code,
 *  rewritten from the project's design documentation after the original
 *  source was lost. See NOTES.md in the repo root.
 *
 *  ---------------------------------------------------------------------
 *  WIRING TABLE (Arduino Uno)
 *  ---------------------------------------------------------------------
 *   Component            Module pin     Arduino pin
 *   -------------------  -------------  -----------
 *   IR sensor LEFT       OUT            A0  (used as digital input)
 *   IR sensor RIGHT      OUT            A1  (used as digital input)
 *   IR sensor UP         OUT            A2  (used as digital input)
 *   IR sensor DOWN/BLINK OUT            A3  (used as digital input)
 *   (all IR sensors)     VCC / GND      5V / GND
 *
 *   16x2 LCD (parallel, 4-bit mode, HD44780 compatible):
 *   LCD RS               pin 4          D12
 *   LCD EN               pin 6          D11
 *   LCD D4               pin 11         D5
 *   LCD D5               pin 12         D4
 *   LCD D6               pin 13         D3
 *   LCD D7               pin 14         D2
 *   LCD RW               pin 5          GND
 *   LCD VO (contrast)    pin 3          10k pot wiper (ends to 5V/GND)
 *   LCD VDD/VSS          pins 2/1       5V / GND
 *   LCD backlight A/K    pins 15/16     5V via 220R / GND
 *
 *   Buzzer / speaker (+)                D9  (tone output)
 *   Buzzer / speaker (-)                GND
 *
 *   Power: regulated 5V supply (or USB during development).
 *  ---------------------------------------------------------------------
 *
 *  GESTURE MAPPING (matches the design documentation):
 *    Look LEFT        -> previous phrase
 *    Look RIGHT       -> next phrase
 *    Look UP          -> confirm / select current phrase
 *    BLINK-HOLD       -> "speak" the selected phrase (LCD banner +
 *                        distinct per-phrase call tone pattern)
 *
 *  The DOWN sensor doubles as the blink sensor: it faces the eyelid, so a
 *  closed eye held for longer than BLINK_HOLD_MS registers as a deliberate
 *  blink-hold. Short activations (natural blinks) are ignored.
 * ============================================================================
 */

#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

/* ---------------------------------------------------------------------------
 * PIN DEFINITIONS
 * ------------------------------------------------------------------------ */
const uint8_t PIN_IR_LEFT  = A0;   // IR sensor: patient looks LEFT
const uint8_t PIN_IR_RIGHT = A1;   // IR sensor: patient looks RIGHT
const uint8_t PIN_IR_UP    = A2;   // IR sensor: patient looks UP
const uint8_t PIN_IR_BLINK = A3;   // IR sensor: eyelid / DOWN position

const uint8_t PIN_BUZZER   = 9;    // passive buzzer, driven with tone()

// LCD on the standard 6-pin parallel hookup: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/*
 * Most common IR proximity modules (e.g. FC-51 style) pull their OUT pin
 * LOW when they detect a reflection. If your modules are active-HIGH,
 * change this one constant to HIGH.
 */
const int SENSOR_ACTIVE_LEVEL = LOW;

/* ---------------------------------------------------------------------------
 * TIMING CONSTANTS (all in milliseconds)
 *
 * Sensible starting values; they will need per-patient tuning on real
 * hardware (see NOTES.md).
 * ------------------------------------------------------------------------ */
const unsigned long GESTURE_HOLD_MS   = 400;    // gaze must be held this long
                                                // to count as a gesture (this
                                                // is the debounce: rejects
                                                // sensor flicker + passing
                                                // glances)
const unsigned long BLINK_HOLD_MS     = 1500;   // eyelid closed this long =
                                                // deliberate blink-hold; a
                                                // natural blink (~100-300 ms)
                                                // never gets close
const unsigned long REFRACTORY_MS     = 700;    // dead time after any gesture
                                                // so one glance = one action
const unsigned long INACTIVITY_MS     = 30000;  // no gestures for this long
                                                // -> return to idle screen
const unsigned long SCROLL_STEP_MS    = 350;    // marquee speed for phrases
                                                // longer than 16 characters
const unsigned long SCROLL_PAUSE_MS   = 1200;   // pause at the start of a
                                                // scrolling phrase so it can
                                                // be read from the beginning

/* ---------------------------------------------------------------------------
 * PHRASE BANK  --  CAREGIVER CUSTOMISATION SECTION
 *
 * To change what the patient can say, edit ONLY this section:
 *
 *   1. Edit the PHRASE_n text (keep it short; longer phrases scroll on
 *      the LCD automatically, but shorter is faster to read).
 *   2. Edit the matching TONE_n pattern. Patterns are strings of
 *      '.' (short beep) and '-' (long beep). Give each phrase a pattern
 *      the care team can learn to recognise from another room, like a
 *      call-bell code. Keep them distinct from each other.
 *   3. To add or remove a phrase, add/remove a PHRASE_n + TONE_n pair
 *      AND add/remove its entry in the PHRASES[] and TONES[] tables
 *      below. The phrase count updates automatically.
 *
 * Strings live in PROGMEM (flash) so they do not eat into the Uno's
 * 2 KB of RAM.
 * ------------------------------------------------------------------------ */
const char PHRASE_0[] PROGMEM = "I need water";
const char PHRASE_1[] PROGMEM = "I am in pain";
const char PHRASE_2[] PROGMEM = "Please call the nurse";
const char PHRASE_3[] PROGMEM = "Yes";
const char PHRASE_4[] PROGMEM = "No";
const char PHRASE_5[] PROGMEM = "I need help";
// --- custom per-patient phrases: caregivers edit/add below ---
const char PHRASE_6[] PROGMEM = "Thank you";
const char PHRASE_7[] PROGMEM = "I feel cold";

const char TONE_0[] PROGMEM = ".";      // I need water
const char TONE_1[] PROGMEM = "---";    // I am in pain      (urgent)
const char TONE_2[] PROGMEM = "-.-.";   // Please call nurse (urgent)
const char TONE_3[] PROGMEM = "..";     // Yes
const char TONE_4[] PROGMEM = "-";      // No
const char TONE_5[] PROGMEM = "...-";   // I need help       (urgent)
const char TONE_6[] PROGMEM = ".-";     // Thank you
const char TONE_7[] PROGMEM = "..-";    // I feel cold

const char* const PHRASES[] PROGMEM = {
  PHRASE_0, PHRASE_1, PHRASE_2, PHRASE_3,
  PHRASE_4, PHRASE_5, PHRASE_6, PHRASE_7
};
const char* const TONES[] PROGMEM = {
  TONE_0, TONE_1, TONE_2, TONE_3,
  TONE_4, TONE_5, TONE_6, TONE_7
};

const uint8_t PHRASE_COUNT = sizeof(PHRASES) / sizeof(PHRASES[0]);

const uint8_t MAX_PHRASE_LEN = 40;      // buffer size for copying a phrase
                                        // out of PROGMEM; keep phrases
                                        // shorter than this

/* ---------------------------------------------------------------------------
 * TONE / BEEP PARAMETERS
 *
 * Honest limitation: a passive buzzer cannot produce speech. "Speaking" a
 * phrase means showing it full-screen on the LCD and playing that phrase's
 * call pattern loudly enough to summon a caregiver, who then reads the
 * LCD. See NOTES.md for the DFPlayer/TTS extension point.
 * ------------------------------------------------------------------------ */
const unsigned int  TONE_CALL_HZ    = 2400;  // main call-pattern pitch
const unsigned int  TONE_NAV_HZ     = 1800;  // navigation tick
const unsigned int  TONE_ERR_HZ     = 400;   // low warning tone
const unsigned long DOT_MS          = 130;   // '.' beep length
const unsigned long DASH_MS         = 390;   // '-' beep length
const unsigned long GAP_MS          = 130;   // silence between beeps
const uint8_t       CALL_REPEATS    = 2;     // pattern is played this many
                                             // times per blink-hold

/* ---------------------------------------------------------------------------
 * GESTURES
 * ------------------------------------------------------------------------ */
enum Gesture : uint8_t {
  GESTURE_NONE = 0,
  GESTURE_LEFT,        // previous phrase
  GESTURE_RIGHT,       // next phrase
  GESTURE_UP,          // confirm selection
  GESTURE_BLINK_HOLD   // speak selected phrase
};

/*
 * Per-sensor debounce state. A gesture fires once when the sensor has been
 * continuously active for its hold time; it cannot fire again until the
 * sensor is released (so staring left does not scroll forever - the patient
 * must glance away and back, which matches how the device was demoed).
 */
struct SensorState {
  uint8_t       pin;
  unsigned long holdMs;       // required continuous-active time
  Gesture       gesture;      // gesture this sensor produces
  unsigned long activeSince;  // millis() when it last became active
  bool          wasActive;    // active on the previous scan
  bool          fired;        // gesture already emitted for this activation
};

SensorState sensors[4] = {
  { PIN_IR_LEFT,  GESTURE_HOLD_MS, GESTURE_LEFT,       0, false, false },
  { PIN_IR_RIGHT, GESTURE_HOLD_MS, GESTURE_RIGHT,      0, false, false },
  { PIN_IR_UP,    GESTURE_HOLD_MS, GESTURE_UP,         0, false, false },
  { PIN_IR_BLINK, BLINK_HOLD_MS,   GESTURE_BLINK_HOLD, 0, false, false }
};

/* ---------------------------------------------------------------------------
 * STATE MACHINE
 *
 *   IDLE ---any gesture---> NAVIGATE (waking gesture is consumed, not acted
 *                                     on, so the screen never jumps)
 *   NAVIGATE --left/right-> NAVIGATE (prev/next phrase, wraps around)
 *   NAVIGATE --up---------> SELECTED (confirmation beep)
 *   SELECTED --blink-hold-> speak: LCD banner + call pattern -> SELECTED
 *   SELECTED --left/right-> NAVIGATE (browse again from the same spot)
 *   any state --30 s idle-> IDLE
 * ------------------------------------------------------------------------ */
enum DeviceState : uint8_t {
  STATE_IDLE,
  STATE_NAVIGATE,
  STATE_SELECTED
};

DeviceState state = STATE_IDLE;
uint8_t phraseIndex = 0;                 // currently highlighted phrase

unsigned long lastGestureAt   = 0;       // for refractory + inactivity
unsigned long lastActivityAt  = 0;

// Marquee scrolling for phrases longer than 16 characters
char          scrollBuf[MAX_PHRASE_LEN];
uint8_t       scrollLen      = 0;
uint8_t       scrollOffset   = 0;
unsigned long lastScrollAt   = 0;
bool          scrollNeeded   = false;

/* ===========================================================================
 * SETUP
 * ======================================================================== */
void setup() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(sensors[i].pin, INPUT);
  }
  pinMode(PIN_BUZZER, OUTPUT);

  lcd.begin(16, 2);

  // --- startup splash ---
  lcd.clear();
  lcd.print(F("Eye Comm Device"));
  lcd.setCursor(0, 1);
  lcd.print(F("SSEC 2020  v1.0"));
  beepStartup();
  delay(1500);

  // --- sensor self-check ---
  // At power-on the patient should not yet be positioned, so every sensor
  // ought to read inactive. A sensor already active means it is blocked,
  // misaligned, or aimed too close to the skin - flag it so the caregiver
  // can fix the fitting before handing the device over.
  runSensorSelfCheck();

  showIdleScreen();
  lastActivityAt = millis();
}

/* ===========================================================================
 * MAIN LOOP
 * ======================================================================== */
void loop() {
  Gesture g = pollGesture();

  if (g != GESTURE_NONE) {
    lastActivityAt = millis();

    if (state == STATE_IDLE) {
      // Wake up. Swallow the waking gesture so the display doesn't jump
      // to an unexpected phrase the instant the patient engages.
      enterNavigate();
    } else {
      handleGesture(g);
    }
  }

  // Inactivity timeout: drop back to the idle screen so a wandering gaze
  // can't accidentally drive the device while nobody is using it.
  if (state != STATE_IDLE && (millis() - lastActivityAt) >= INACTIVITY_MS) {
    state = STATE_IDLE;
    showIdleScreen();
  }

  updateScroll();
}

/* ===========================================================================
 * GESTURE DETECTION (debounced, refractory-limited)
 * ======================================================================== */

/*
 * Scans all four sensors and returns at most one gesture per call.
 *
 * Debounce: a sensor must read active continuously for its hold time
 * (400 ms for gaze, 1500 ms for blink-hold) before its gesture fires.
 * Brief flickers and natural blinks therefore never register.
 *
 * Refractory period: after any gesture fires, all sensors are ignored for
 * REFRACTORY_MS, and the firing sensor must be released before it can fire
 * again. Together these guarantee one glance = one action.
 */
Gesture pollGesture() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < 4; i++) {
    SensorState &s = sensors[i];
    bool active = (digitalRead(s.pin) == SENSOR_ACTIVE_LEVEL);

    if (active && !s.wasActive) {          // rising edge: start the clock
      s.activeSince = now;
      s.fired = false;
    }
    if (!active) {                         // released: allow re-fire later
      s.fired = false;
    }
    s.wasActive = active;

    bool heldLongEnough = active && !s.fired &&
                          (now - s.activeSince) >= s.holdMs;
    bool refractoryOver = (now - lastGestureAt) >= REFRACTORY_MS;

    if (heldLongEnough && refractoryOver) {
      s.fired = true;                      // once per activation
      lastGestureAt = now;
      return s.gesture;
    }
  }
  return GESTURE_NONE;
}

/* ===========================================================================
 * STATE MACHINE ACTIONS
 * ======================================================================== */
void handleGesture(Gesture g) {
  switch (state) {

    case STATE_NAVIGATE:
      if (g == GESTURE_LEFT) {
        phraseIndex = (phraseIndex == 0) ? (PHRASE_COUNT - 1)
                                         : (phraseIndex - 1);
        beepNav();
        showNavigateScreen();
      } else if (g == GESTURE_RIGHT) {
        phraseIndex = (phraseIndex + 1) % PHRASE_COUNT;
        beepNav();
        showNavigateScreen();
      } else if (g == GESTURE_UP) {
        state = STATE_SELECTED;
        beepConfirm();
        showSelectedScreen();
      }
      // Blink-hold in NAVIGATE is ignored on purpose: the patient must
      // explicitly confirm (look up) before the device will call out,
      // which prevents accidental alarms from long involuntary blinks.
      break;

    case STATE_SELECTED:
      if (g == GESTURE_BLINK_HOLD) {
        speakPhrase(phraseIndex);          // blocking by design; see below
        showSelectedScreen();              // stay selected for repeats
      } else if (g == GESTURE_LEFT || g == GESTURE_RIGHT) {
        // Patient changed their mind - back to browsing.
        state = STATE_NAVIGATE;
        handleGesture(g);                  // apply the prev/next immediately
      }
      break;

    case STATE_IDLE:
      break;                               // handled in loop()
  }
}

void enterNavigate() {
  state = STATE_NAVIGATE;
  beepNav();
  showNavigateScreen();
}

/*
 * "Speak" the phrase: full-screen LCD banner plus the phrase's call tone
 * pattern. This deliberately blocks the main loop - while the device is
 * calling out, gestures should not be able to interrupt or re-trigger it.
 */
void speakPhrase(uint8_t idx) {
  char phrase[MAX_PHRASE_LEN];
  char pattern[12];
  loadPhrase(idx, phrase);
  strcpy_P(pattern, (const char *)pgm_read_ptr(&TONES[idx]));

  lcd.clear();
  lcd.print(F("** CALLING **"));
  lcd.setCursor(0, 1);
  printClipped(phrase);                    // long phrases show first 16 chars
                                           // here; full text scrolls again on
                                           // the selected screen afterwards

  for (uint8_t r = 0; r < CALL_REPEATS; r++) {
    playPattern(pattern);
    delay(GAP_MS * 3);                     // gap between repeats
  }
}

/* ===========================================================================
 * LCD SCREENS
 * ======================================================================== */
void showIdleScreen() {
  lcd.clear();
  lcd.print(F("Eye Comm ready"));
  lcd.setCursor(0, 1);
  lcd.print(F("Glance to start"));
  scrollNeeded = false;
}

/*
 * NAVIGATE screen:
 *   line 1: current phrase (scrolls if >16 chars)
 *   line 2: position in bank + reminder of the confirm gesture, e.g.
 *           "3/8    up=select"
 */
void showNavigateScreen() {
  lcd.clear();
  startScrollLine(phraseIndex);

  lcd.setCursor(0, 1);
  lcd.print(phraseIndex + 1);
  lcd.print('/');
  lcd.print(PHRASE_COUNT);
  lcd.setCursor(7, 1);
  lcd.print(F("up=select"));
}

/*
 * SELECTED screen:
 *   line 1: the confirmed phrase (scrolls if >16 chars)
 *   line 2: reminder of the speak gesture
 */
void showSelectedScreen() {
  lcd.clear();
  startScrollLine(phraseIndex);

  lcd.setCursor(0, 1);
  lcd.print(F("hold blink=call"));
}

/* ---------------------------------------------------------------------------
 * Marquee scrolling for line 1.
 *
 * The 16x2 LCD can only show 16 characters, but phrases like "Please call
 * the nurse" are longer. Phrases that fit are printed statically; longer
 * ones scroll one character every SCROLL_STEP_MS, with a pause at the start
 * of each pass so the phrase can be read from its beginning.
 * ------------------------------------------------------------------------ */
void startScrollLine(uint8_t idx) {
  loadPhrase(idx, scrollBuf);
  scrollLen    = strlen(scrollBuf);
  scrollOffset = 0;
  scrollNeeded = (scrollLen > 16);
  lastScrollAt = millis() + SCROLL_PAUSE_MS;   // initial reading pause
  drawScrollWindow();
}

void updateScroll() {
  if (!scrollNeeded || state == STATE_IDLE) return;
  unsigned long now = millis();
  if ((long)(now - lastScrollAt) < (long)SCROLL_STEP_MS) return;

  scrollOffset++;
  if (scrollOffset > (uint8_t)(scrollLen - 16)) {
    scrollOffset = 0;
    lastScrollAt = now + SCROLL_PAUSE_MS;      // pause before next pass
  } else {
    lastScrollAt = now;
  }
  drawScrollWindow();
}

void drawScrollWindow() {
  lcd.setCursor(0, 0);
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t p = scrollOffset + i;
    lcd.write(p < scrollLen ? scrollBuf[p] : ' ');
  }
}

// Print at most 16 characters of a string at the current cursor position.
void printClipped(const char *s) {
  for (uint8_t i = 0; i < 16 && s[i] != '\0'; i++) {
    lcd.write(s[i]);
  }
}

// Copy phrase idx out of PROGMEM into a RAM buffer.
void loadPhrase(uint8_t idx, char *buf) {
  strcpy_P(buf, (const char *)pgm_read_ptr(&PHRASES[idx]));
}

/* ===========================================================================
 * SOUNDS
 * ======================================================================== */
void beepNav() {                           // short tick on prev/next
  tone(PIN_BUZZER, TONE_NAV_HZ, 40);
}

void beepConfirm() {                       // two ascending notes on select
  tone(PIN_BUZZER, 1600, 90);
  delay(110);
  tone(PIN_BUZZER, 2200, 120);
  delay(140);
}

void beepStartup() {                       // three ascending notes at boot
  tone(PIN_BUZZER, 1200, 90);  delay(110);
  tone(PIN_BUZZER, 1600, 90);  delay(110);
  tone(PIN_BUZZER, 2000, 120); delay(140);
}

void beepWarning() {                       // low buzz for self-check faults
  tone(PIN_BUZZER, TONE_ERR_HZ, 250);
  delay(300);
}

// Play one '.'/'-' call pattern at the call pitch.
void playPattern(const char *pattern) {
  for (const char *c = pattern; *c != '\0'; c++) {
    unsigned long dur = (*c == '-') ? DASH_MS : DOT_MS;
    tone(PIN_BUZZER, TONE_CALL_HZ, dur);
    delay(dur + GAP_MS);
  }
}

/* ===========================================================================
 * SENSOR SELF-CHECK (runs once at boot)
 * ======================================================================== */
void runSensorSelfCheck() {
  const char *names[4] = { "LEFT", "RIGHT", "UP", "BLINK" };
  bool allClear = true;

  for (uint8_t i = 0; i < 4; i++) {
    if (digitalRead(sensors[i].pin) == SENSOR_ACTIVE_LEVEL) {
      allClear = false;
      lcd.clear();
      lcd.print(F("Sensor blocked:"));
      lcd.setCursor(0, 1);
      lcd.print(names[i]);
      lcd.print(F(" - adjust fit"));
      beepWarning();
      delay(2000);                         // give the caregiver time to read
    }
  }

  lcd.clear();
  lcd.print(allClear ? F("Sensors OK") : F("Check fit, then"));
  lcd.setCursor(0, 1);
  lcd.print(allClear ? F("Starting...") : F("power-cycle"));
  delay(1200);
}
