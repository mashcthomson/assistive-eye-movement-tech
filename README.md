# Assistive Eye Movement Technology 👁️

![Arduino](https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=cplusplus&logoColor=white)

> Bachelor of IT Capstone Project – Sri Sairam Engineering College, 2020

---

## Why this exists

People with ALS, locked-in syndrome, or severe paralysis often lose the ability to speak and move their limbs entirely. The existing solutions that let them communicate — eye-tracking AAC (Augmentative and Alternative Communication) systems — work well but cost $10,000 or more, require sophisticated camera hardware, and need complex calibration. Most hospitals and care facilities in lower-resource settings simply can't afford them.

This project was an attempt to build something that does the same job for a fraction of the cost.

---

## What we built

A communication device using an Arduino Uno and four IR proximity sensors positioned around the eyes. By detecting left, right, up, and down eye movements, patients can navigate through a pre-loaded phrase bank and select what they want to say. The selected phrase shows on an LCD display and can be spoken through a buzzer/speaker module.

No camera. No complex calibration. No $10,000 price tag.

---

## How it works

**Eye gestures:**

| Eye Movement | Action |
|-------------|--------|
| Look Left | Navigate to previous option |
| Look Right | Navigate to next option |
| Look Up | Confirm selection |
| Blink (hold) | Speak selected phrase |

**System flow:**

```
IR Sensors (Left / Right / Up / Down)
            |
       Arduino Uno
     (Signal Processing)
            |
    +-------+-------+
    |               |
 LCD Display    Audio Output
(Phrase shown) (Text-to-speech)
```

**Sample phrases in the phrase bank:**
- "I need water"
- "I am in pain"
- "Please call the nurse"
- "Yes" / "No"
- "I need help"
- Custom phrases configurable per patient by caregivers

---

## Hardware

| Component | Purpose |
|-----------|---------|
| Arduino Uno | Main microcontroller |
| IR Proximity Sensors (x4) | Eye movement detection |
| 16x2 LCD Display | Visual output |
| Buzzer / Speaker Module | Audio feedback |
| Power Supply | 5V regulated |

---

## Results

- **90%+ cost reduction** compared to camera-based AAC systems
- Faster detection response than camera-based processing
- Usable in low-resource healthcare environments
- Phrase bank customisable per patient without technical knowledge

---

## What I'd do differently now

This was built in 2020, early in my degree. If I were building it today I'd look at adding ML-based eye movement calibration per user (people's eye movements aren't uniform), Bluetooth connectivity for smartphone output, and a predictive phrase selection system that learns a patient's most common phrases over time.

The core idea is solid though. IR sensors are cheap, reliable, and don't need lighting conditions the way cameras do.

---

*Capstone Project – Bachelor of Information Technology, Sri Sairam Engineering College, 2020*
