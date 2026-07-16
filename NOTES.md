# Notes on this code

## Reconstruction disclaimer

The original 2020 source code for this capstone project was lost. The sketch in
`assistive_eye_comm/` is a faithful reconstruction written from the project's
design documentation — the gesture mapping, hardware list, phrase bank, and
system flow described in the README. It is **not** the recovered original 2020
submission, and it has **not** been re-tested on the physical hardware. It
compiles cleanly for the Arduino Uno, but anyone rebuilding the device should
expect to re-tune thresholds and timings on real hardware (see Calibration
below).

## Hardware wiring summary

| Component | Connection |
|-----------|-----------|
| IR sensor LEFT (OUT) | A0 |
| IR sensor RIGHT (OUT) | A1 |
| IR sensor UP (OUT) | A2 |
| IR sensor DOWN/BLINK (OUT) | A3 |
| IR sensors VCC / GND | 5V / GND |
| LCD RS / EN | D12 / D11 |
| LCD D4 / D5 / D6 / D7 | D5 / D4 / D3 / D2 |
| LCD RW | GND |
| LCD VO (contrast) | 10k pot wiper (ends to 5V/GND) |
| LCD backlight | 5V via 220 ohm resistor / GND |
| Buzzer (+) / (−) | D9 / GND |
| Power | Regulated 5V (USB fine for bench work) |

The full pin-by-pin table is in the header comment of
`assistive_eye_comm/assistive_eye_comm.ino`.

## Known limitations

- **No hardware re-test.** This reconstruction has been compile-verified only
  (`arduino-cli`, `arduino:avr:uno`). Behaviour on the assembled device has not
  been re-validated since 2020.
- **The buzzer is not text-to-speech.** A passive buzzer physically cannot
  produce speech. The original design's "audio output" is implemented honestly
  here as an attention-call system: each phrase has a distinct short/long beep
  pattern (like a call-bell code) that summons a caregiver, who reads the
  phrase off the LCD. Real speech output would need a DFPlayer Mini playing
  pre-recorded phrase MP3s, or a TTS module — the extension point is the
  `speakPhrase()` function, which already receives the phrase index and is the
  single place audio output happens.
- **IR sensor placement and thresholds are per-patient.** Eye geometry,
  eyelash interference, skin reflectivity, and glasses all change what the IR
  sensors see. Sensor position (a few millimetres matters), the trim-pot
  threshold on each IR module, and the hold-time constants in the sketch all
  need tuning for each individual user.
- **Ambient IR interference.** Direct sunlight and some indoor lighting can
  saturate cheap IR proximity modules. The device was intended for indoor
  bedside use.
- **Blocking audio.** While a call pattern is playing, gesture input is
  intentionally ignored (a deliberate design choice so an alarm can't be
  interrupted or double-triggered mid-call).

## Calibration guidance

1. **Mount and aim the sensors** on the frame so each one faces its region of
   the eye (left sclera, right sclera, upper lid/brow, lower lid). Power the
   device on *before* fitting it: the startup self-check flags any sensor that
   is already triggering, which usually means it sits too close to the skin.
2. **Set each IR module's trim pot** so the sensor is OFF at neutral gaze and
   reliably ON when the eye moves toward it. Adjust one sensor at a time;
   the onboard indicator LED on most modules makes this easy.
3. **Sensor polarity:** most modules pull OUT low on detection. If yours are
   active-high, flip `SENSOR_ACTIVE_LEVEL` in the sketch.
4. **Tune the timings** (named constants at the top of the sketch):
   - `GESTURE_HOLD_MS` (default 400 ms) — raise it if the user triggers
     phrases accidentally while glancing around; lower it if navigation feels
     sluggish.
   - `BLINK_HOLD_MS` (default 1500 ms) — must sit well above the user's
     natural blink duration. If natural blinks ever trigger a call, raise it.
   - `REFRACTORY_MS` (default 700 ms) — raise it if single glances ever
     register twice.
5. **Customise the phrase bank** in the clearly marked "caregiver
   customisation" section of the sketch: edit the `PHRASE_n` strings and give
   each one a distinct `TONE_n` beep pattern the care team can learn.
6. **Re-verify with the user** after any change: run through every phrase,
   confirm one glance moves exactly one step, and confirm natural blinks
   never trigger the call tone.
