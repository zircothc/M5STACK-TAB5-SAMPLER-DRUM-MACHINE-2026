# M5STACK-TAB5-SAMPLER-DRUM-MACHINE-2026

M5STACK TAB5 SAMPLER DRUM MACHINE 2026 - ESP32 P4

This is my ported vsampler to M5STACK TAB5

Added a lot of features.

Samples
- Samples (wav mono 44100 16bit) are loaded from SD card into PSRAM at start. No need to convert into array
- ADSR
  
Synth
- Added synth engine (upgraded from my previous drum synth. Now is 16bit)

FX
- FX per channel: Reverb, Delay, Chorus, Flanger, Tremolo.
- Sets of presets on every FX

USB HOST
- USB MIDI host. Now is mapped to use with AKI APC KEY25
