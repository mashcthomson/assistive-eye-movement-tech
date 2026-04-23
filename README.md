# Assistive Eye Movement Technology 👁️

![Arduino](https://img.shields.io/badge/Arduino-00979D?style=flat-square&logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=cplusplus&logoColor=white)

> Bachelor of IT Capstone Project – Sri Sairam Engineering College

## Overview

An assistive communication system that enables paralyzed patients to express words and phrases through **eye movement detection** using infrared (IR) sensors and an Arduino microcontroller. This project provides a low-cost, accessible alternative to expensive camera-based Augmentative and Alternative Communication (AAC) systems.

## The Problem

Patients with conditions like ALS, locked-in syndrome, or severe paralysis often lose the ability to speak or move their limbs. Existing eye-tracking AAC systems are:
- Expensive (often $10,000+)
- Require sophisticated camera hardware
- Need complex calibration
- Not accessible in low-resource healthcare settings

## Our Solution

A simple, affordable eye-tracking system using:
- **IR sensors** positioned near the eyes to detect left/right/up/down movements
- **Arduino microcontroller** to process sensor signals
- **Phrase selection interface** allowing communication through eye gestures
- **LCD/Audio output** to display or speak selected phrases

## Hardware Components

| Component | Purpose |
|-----------|--------|
| Arduino Uno | Main microcontroller |
| IR Proximity Sensors (x4) | Eye movement detection |
| 16x2 LCD Display | Visual output |
| Buzzer / Speaker Module | Audio feedback |
| Power Supply | 5V regulated |

## System Architecture

```
IR Sensors (Left/Right/Up/Down)
         |
    Arduino Uno
    (Signal Processing)
         |
   +-----------+
   |           |
 LCD Display  Audio Output
(Phrase shown) (Text-to-speech)
```

## Eye Gesture Mapping

| Eye Movement | Action |
|-------------|--------|
| Look Left | Navigate previous option |
| Look Right | Navigate next option |
| Look Up | Confirm selection |
| Blink (hold) | Speak selected phrase |

## Sample Phrase Bank

- "I need water"
- "I am in pain"
- "Please call the nurse"
- "Yes" / "No"
- "I need help"
- Custom phrases configurable by caregivers

## Key Outcomes

- **Lower cost**: Estimated 90%+ cost reduction vs camera-based systems
- **Improved accessibility**: Usable in resource-limited healthcare settings
- **Faster response**: IR-based detection faster than camera processing
- **Customizable**: Phrase bank easily modified for individual patients

## Future Enhancements

- Machine learning for personalized eye movement calibration
- Bluetooth connectivity for smartphone integration
- Expanded phrase library with predictive selection
- Integration with smart home devices

---

*Capstone Project – Bachelor of Information Technology, Sri Sairam Engineering College, 2020*
