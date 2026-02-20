# M5STACK-TAB5-SAMPLER-DRUM-MACHINE-2026
![tab5_synth](https://github.com/user-attachments/assets/7a61c046-0e4a-479e-8a97-a1a52efa29c4)


M5STACK TAB5 SAMPLER DRUM MACHINE 2026 - ESP32 P4

This is my ported vsampler to M5STACK TAB5

Added a lot of features.

Samples
- Samples (wav mono 44100 16bit) are loaded from SD card into PSRAM at start. No need to convert into array
- ADSR
  
Synth
- Added synth engine (upgraded from my previous drum synth. Now is 16bit 44100 instead of 8bit 22050
- Each track can be sampler or synth.
- Multi-wavetable voice.

FX
- FX per channel: Reverb, Delay, Chorus, Flanger, Tremolo, etc
- Sets of presets on every FX

USB HOST
- USB MIDI host. Now is mapped to use with AKI APC KEY25

WHAT YOU NEED:
- M5STACK TAB5 HARDWARE 
- Arduino IDE 2.3.7
- ESP boards from Expressif (3.3.3)
- M%STACK boards (3.2.5)
- M5Unified Library (ArduinoIDE library manager) (3.2.5)
- esp32-usb-host-demos Library from touchgadget https://github.com/touchgadget/esp32-usb-host-demos
- Rest of libraries are already included (I think)

## SYSTEM OVERVIEW
Hybrid music production system based on the M5STACK TAB5 with the ESP32-P4 microcontroller. It combines an enhanced synthesis engine with a sample player, designed for live performance and generative musical ideas.

### Audio Engine
* **Quality:** 16-bit / 44,100 Hz (Significant upgrade from the previous 8-bit / 22kHz).
* **Trigger Architecture:** "One Shot" system (Single Note ON, no Note OFF event).
* **Duration Control:**
    * **Synthesizer:** Defined by the `LEN` parameter.
    * **Sampler:** Defined by Start (`INI`) and End (`END`) points.
    * *Note:* INI/END values are mapped in a range from 0 to 2048, offering approximate precision.

---

## USER INTERFACE (GUI)

The interface combines permanent physical controls with contextual page navigation.

### Global Controls (Always visible)
* **Pads:** Trigger sounds, select tracks, and Piano mode keyboard.
* **Navigation:** Buttons for track selection and value adjustment (+/-).
* **SHIFT Function:**
    * Modifies the +/- 10 buttons to count by 100s.
    * When combined with `SOLO`, `MUTE`, or `STEP INIT`, it resets parameters to default values.

### Page Structure
1. **SOUND:** Sound generation and shaping.
2. **GLOBAL:** Sequencer settings and utilities.
3. **FX:** Effects processing.

---

## PAGE: SOUND (Sound Generator)

The system features **16 Tracks**: 8 Synthesizer tracks (Left) and 8 Sampler tracks (Right).

### Sampler Features
* **Source:** WAV files (Mono, 16-bit, 44.1kHz) from SD card.
* **Capacity:** Loads the first 128 compatible files or until the 24MB of reserved PSRAM is full.
* **Fallback:** If no SD card is present, the system operates exclusively as a synthesizer.
* **Envelope:** Full ADSR available for samples.

### Synthesizer Features
* **Engine:** Improved version of the previous "Drum Synth."
* **Envelopes:** 4 predefined types (variations of attack, release, etc.).
* **Detune:** Exclusive parameter to detune the oscillator and thicken the sound.

### Editing Parameters
`PAN`, `VOL`, `FILTER`, `ENVELOPE` (Synth), `MOD` (Synth), `DETUNE` (Synth), `LEN` (Synth), `TYPE`, `A`, `D`, `S`, `R`, `REVERSE`.

### Layering Function
* **ADD NEXT SND:** Allows triggering multiple sounds simultaneously.
    * *Example:* If Track 7 has this value set to 2, triggering Track 7 will also play Track 8.

---

## PAGE: GLOBAL (Sequencer & Utilities)

### Playback Settings
* **BPM:** Global tempo.
* **MASTER VOL:** Overall volume.
* **MELODIC:**
    * *ON:* The engine uses the pitch stored in each step of the sequence.
    * *OFF:* The sound uses the fixed pitch defined in the `PITCH` parameter.
* **SAVE:** Saves the pattern and sounds. Pressing it allows you to change the SAVE type (patterns only or sounds only).

### Composition Tools
* **MOVE:** Shifts the selected pattern to the left or right.
* **SCALE:** Restricts random note generation to a specific musical scale.
* **PIANO MODE:** Allows playing the selected sound chromatically using the PADS.
    * *Press again:* Activates **CLEAR** mode to erase the selected pattern.
* **SONG LOAD MODE:** Defines what is loaded automatically:
    * `0`: Loads Pattern + Sounds.
    * `1`: Pattern Only.
    * `2`: Sounds Only.

### Random Generation
* **RANDOM SND:** Generates a randomized sound design.
* **RANDOM PATTERN:** Generates a rhythmic and melodic sequence.
    * Follows the selected `SCALE`.
    * If Scale = 0, it uses the base PITCH with octave variations (+/- 12 semitones).

---

## PAGE: FX (Effects)

* **Processing:** Utilizes the remaining PSRAM (not occupied by samples).
* **Persistence:** *Warning:* FX settings are NOT saved to Flash memory.
* **Presets:** Most effects use fixed presets for ease of use.
* **Delay:** The only effect with numerical configuration (milliseconds).
* **Repeat:** Note repeat with 4 rhythmic positions (0 = disabled).

---

## MEMORY MANAGEMENT & SONG MODE

### Data Structure
* **Memories:** 16 total Memory Banks.
* **Content:** Each Memory stores 16 different Patterns, each with its own set of sounds.
* **Storage:** All data is saved to internal Flash.

### Load Operations (LOAD)
* **LOAD P:** Loads only the 16 patterns.
* **LOAD S:** Loads only the 16 sounds.
* **LOAD:** Loads both patterns and sounds (default).
* *Startup Note:* Upon power-up, the system does not load any Memory automatically. You must load a Memory before loading a pattern.

### SONG Mode
Automatically chains patterns together.
* Operates within the active "Memory."
* Allows selecting a START pattern and an END pattern to create a playback loop.
