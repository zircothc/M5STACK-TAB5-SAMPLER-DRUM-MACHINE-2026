////////////////////////////
// DRUM SAMPLER 2026 TAB5 //
////////////////////////////
// zcarlos 2026
// V1
// Lots of bugs


// includes
#include <Arduino.h>
#include <Wire.h>

#include <FreeRTOS.h>
#include <task.h>
#include "freertos/semphr.h"

#include "button.h"
#include "rot.h"
#include "wavetables.h"

// jack (varios días para conseguir apagar el **** altavoz interno cuando se inserta el jack!!!!)
uint8_t old_jack = 2;
uint8_t val = 0;
unsigned long lastCheck = 0;
// Direcciones y Registros
#define ADDR_EXP 0x43
#define REG_CONFIG 0x03
#define REG_OUTPUT 0x05
#define REG_INPUT 0x0F

// Máscaras
#define PIN_SPK 0x02   // Pin 1 (1 << 1)
#define PIN_JACK 0x80  // Pin 7 (1 << 7)


const int MAX_BUTTONS = 50;
const int MAX_BARS = 45;

Boton* mBoton[MAX_BUTTONS];
Rot* mRot[MAX_BARS];
Bseq* mBseq[MAX_BUTTONS];


// Display & Touch & Audio & SD
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>

// SD PINOUT
#define SD_SPI_CS_PIN 42
#define SD_SPI_SCK_PIN 43
#define SD_SPI_MOSI_PIN 44
#define SD_SPI_MISO_PIN 39

// SD
#define MAX_SAMPLES_COUNT 128
//#define RAM_LIMIT (16 * 1024 * 1024)  // 16 MB
#define RAM_LIMIT (24 * 1024 * 1024)  // 24 MB
int16_t* SAMPLES[MAX_SAMPLES_COUNT];
size_t SAMPLES_SIZES[MAX_SAMPLES_COUNT];
int samplesTotal = 0;
String SAMPLE_NAMES[MAX_SAMPLES_COUNT];
uint64_t ENDS[MAX_SAMPLES_COUNT];
M5Canvas waveSprite(&M5.Display);

// para dibujar mas rápido...
// Arrays para guardar los valores pre-calculados
int16_t cacheMinVal[2][MAX_SAMPLES_COUNT];
int16_t cacheMaxVal[2][MAX_SAMPLES_COUNT];

char bufferSeguro[27];

const int WAVE_WIDTH = 312;
const int WAVE_HEIGHT = 140;
const int WAVE_ORIGIN_X = 164;
const int WAVE_ORIGIN_Y = 56;

lgfx::touch_point_t tp[1];  // hasta 5 puntos
#include "fx.h"

TaskHandle_t usbTaskHandle = NULL;

///////////////////////////////////////////////////////////// MIDI USB HOST
#include <usb/usb_host.h>
#include "show_desc.hpp"
#include "usbhhelp.hpp"
bool isMIDI = false;
bool isMIDIReady = false;
const size_t MIDI_IN_BUFFERS = 8;
usb_transfer_t* MIDIIn[MIDI_IN_BUFFERS] = { NULL };
usb_transfer_t* MIDIOut = NULL;

// AKAI APC KEY25
uint8_t pageRot = 0;  // maps 8 cc pot into pages
uint8_t old_cc_page_note = 82;
const uint8_t mNN_pageRot[5] = { 82, 83, 84, 85, 86 };  // akai buttons (notes) to select pageRot

int rPage = 0;  // 0 sound, 1 global, 2 fx
int old_rPage = -1;

const int debounce_time = 230;  // time to wait, miliseconds
long start_debounce;
bool touchActivo = false;
int last_touched = -1;
bool show_last_touched = false;
bool clear_last_touched = false;
long start_showlt;

bool flag_ss = false;

///////////////// ROTS
int counter1 = 0;
int old_counter1 = 0;

///////////////// SHIFT
byte shiftR1 = false;
byte old_shiftR1 = true;

// Touchscreen coordinates: (x, y) and pressure (z)
int cox, coy, coz;

////////////////////////////// SPIFFS
#include <FS.h>
#include <SPIFFS.h>
//const char* RUTA_ARCHIVO = "/backup_completo.bin";

////////////////////////////// SYNTH
#include "tablesESP32_E.h"


#define SAMPLE_RATE 44100
#define I2S_BITS I2S_BITS_PER_SAMPLE_16BIT

#include "synthESP32LowPassFilter_E.h"  // filter
#define MAX_16 32767
#define MIN_16 -32767

////////////////////////////// TIMER SEQ
#include "seq.h"
volatile int tick = 0;
ESP32Sequencer seq;

////////////////////////////////////////////////////////////////////////////////////////////

// modeZ types

//  0 - play pad sound
//  1 - select sound
//  2 - edit track, write
//  3 - mute
//  4 - solo
//  5 - clear

//  6 - load pattern
//  7 - load pattern
//  8 - load pattern and sound

//  9 - save pattern
// 10 - save sound
// 11 - save sound and pattern

// 12 - random sound
// 13 - random pattern
// 14 - first step
// 15 - last step
// 16 - set melodic (read pitch from pattern melodic instead of rotary value)
// 17 - random note
// 18 - piano
// 19 - song or pattern
// 20 - select memory
// 21 - first pattern
// 22 - last pattern
// 23 - random sound 2

// 24 - reverb
// 25 - delay
// 26 - chorus
// 27 - flanger
// 28 - tremolo
// 29 - ring mod
// 30 - distortion
// 31 - bitcrusher


#define tPad 0
#define tSel 1
#define tWrite 2
#define tMute 3
#define tSolo 4
#define tClear 5
#define tLoaPS 6
#define tLoadP 7
#define tLoadS 8
#define tSavPS 9
#define tSaveP 10
#define tSaveS 11
#define tRndS 12
#define tRndP 13
#define tFirst 14
#define tLast 15
#define tMelod 16
#define tRndNo 17
#define tPiano 18
#define tSong 19
#define tMemo 20
#define tPfirs 21
#define tPlast 22
#define tRndS2 23
#define tReverb 24
#define tDelay 25
#define tChorus 26
#define tFlanger 27
#define tTremolo 28
#define tRingmod 29
#define tDistortion 30
#define tBitcrusher 31

//uint64_t NEWENDS[MAX_SAMPLES_COUNT];
//uint64_t NEWINIS[MAX_SAMPLES_COUNT];
uint64_t NEWENDS[16];
uint64_t NEWINIS[16];

byte latch[16];

uint64_t samplePos[16];
uint64_t stepSize[16];


// I2S

#define DMA_BUF_LEN 256  //4
#define DMA_NUM_BUF 8

static int16_t out_buf[DMA_BUF_LEN * 2];

// 16 filters + master L & R (Total = 18)
LowPassFilter FILTROS[18] = {
  LowPassFilter(), LowPassFilter(), LowPassFilter(), LowPassFilter(),
  LowPassFilter(), LowPassFilter(), LowPassFilter(), LowPassFilter(),
  LowPassFilter(), LowPassFilter(), LowPassFilter(), LowPassFilter(),
  LowPassFilter(), LowPassFilter(), LowPassFilter(), LowPassFilter(),
  LowPassFilter(), LowPassFilter()
};

const int cutoff = 255;
const int reso = 511;

static int VOL_R[16] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };
static int VOL_L[16] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };
static int PAN[16] = { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 };

static int mvol = 10;
int master_vol = 100;
int master_filter = 0;
int octave = 5;

bool is_reverb = true;
bool is_delay = true;
bool is_chorus = true;
bool is_flanger = true;
bool is_tremolo = true;
bool is_ringmod = true;
bool is_distortion = true;
bool is_bitcrusher = true;

uint32_t level_reverb = 100;
uint32_t level_delay = 100;
uint32_t level_chorus = 100;
uint32_t level_flanger = 100;
uint32_t level_tremolo = 100;
uint32_t level_ringmod = 100;
uint32_t level_distortion = 100;
uint32_t level_bitcrusher = 100;

uint8_t reverb_type = 8;
uint8_t delay_time = 20;
uint8_t chorus_type = 7;
uint8_t flanger_type = 8;
uint8_t tremolo_type = 1;
uint8_t ringmod_type = 1;
uint8_t distortion_type = 1;
uint8_t bitcrusher_type = 1;

/////// SYNTH ENGINE
static uint32_t PCW[16];
static uint32_t FTW[16];
static unsigned char AMP[16];
static uint32_t PITCH[16];
static int MOD[16] = { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 };
static int32_t sound_A[16];
static unsigned int wavs[16];
static unsigned int envs[16];
static uint32_t EPCW[16];
static uint32_t EFTW[16];

////////////////////////////// SEQ
int bpm = 120;
byte selected_pattern = 0;
byte s_old_selected_pattern = 1;
byte sstep = 0;
int s_old_sstep = -1;

uint16_t mutes = 0;
uint16_t solos = 0;
uint16_t reverbs = 0;
uint16_t delays = 0;
uint16_t choruss = 0;
uint16_t flangers = 0;
uint16_t tremolos = 0;
uint16_t ringmods = 0;
uint16_t distortions = 0;
uint16_t bitcrushers = 0;

uint16_t pattern[16];  // 16 patterns, on off (16 bit) for step
int16_t  detune[16];  // 16 sounds, from -127 to 127. 0=no detune
uint16_t addnextsnd[16]; // 16 sounds, this is a sort of multi oscilator. case: if addnextsnd[2]=2 then when sound 2 plays sound 3 at the same time
uint8_t  melodic[16][16];  // 16 patterns, 16 steps with note number

uint16_t memory_pattern[16][16]; 
uint8_t  memory_melodic[16][16][16];
int32_t  memory_ROTvalue[16][16][20];


byte fx1 = 0;

byte selected_memory = 0;
byte s_old_selected_memory = 1;
int pattern_song_counter = -1;
byte last_pattern_song = 255;


uint16_t isMelodic[16]; 

byte firstStep = 0;
byte lastStep = 15;
byte newLastStep = 15;

byte firstPattern = 0;  // new name from firstMemory
byte lastPattern = 15;
byte newLastPattern = 15;

byte selected_sound = 0;
byte oldselected_sound = 0;
byte s_old_selected_sound = 1;
byte selected_sndSet = 0;
byte s_old_selected_sndSet = 1;
int ztranspose = 0;
int zmpitch = 0;
int zmove = 0;
bool load_flag = false;

byte sync_state = 0;  // 0 no sync, 1, master, 2 slave
bool sync_flag = false;

const char* txt_songmodes[3] = {
    "0 PAT. & SND.",
    "1 Only PAT.",
    "2 Only SND."
};
uint8_t song_mode=0;

// ESCALAS (255 = void)
uint8_t selected_scale = 0;
const uint8_t escalas[13][8] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },  // free
  // Escalas diatónicas
  { 0, 2, 4, 5, 7, 9, 11, 255 },  // Mayor (Jónica)
  { 0, 2, 3, 5, 7, 8, 10, 255 },  // Menor natural (Eólica)
  { 0, 2, 3, 5, 7, 8, 11, 255 },  // Menor armónica
  { 0, 2, 3, 5, 7, 9, 11, 255 },  // Menor melódica

  // Escalas pentatónicas y blues
  { 0, 2, 4, 7, 9, 255, 255, 255 },   // Pentatónica mayor
  { 0, 3, 5, 7, 10, 255, 255, 255 },  // Pentatónica menor
  { 0, 3, 5, 6, 7, 10, 255, 255 },    // Blues menor

  // Modos
  { 0, 2, 3, 5, 7, 9, 10, 255 },  // Dórico
  { 0, 1, 3, 5, 7, 8, 10, 255 },  // Frigio
  { 0, 2, 4, 6, 7, 9, 11, 255 },  // Lidio
  { 0, 2, 4, 5, 7, 9, 10, 255 },  // Mixolidio
  { 0, 1, 3, 5, 6, 8, 10, 255 }   // Locrio
};
const char* nombresEscalas[] = {
  "0 Free",
  "1 Maj",
  "2 Min Nat",
  "3 Min Arm",
  "4 Min Mel",
  "5 Pent Maj",
  "6 Pent Min",
  "7 Blues Min",
  "8 Dorico",
  "9 Frigio",
  "10 Lidio",
  "11 Mixolidio",
  "12 Locrio"
};


const int max_values[MAX_BARS] = {
  MAX_SAMPLES_COUNT - 1,  // 0  SAMPLE
  WT_COUNT - 1,           // 1  WAVETABLE
  255,                    // 2  A
  255,                    // 3  D
  255,                    // 4  S
  255,                    // 5  R
  2047,                   // 6  INI
  2047,                   // 7  END
  1,                      // 8  REVERSE
  3,                      // 9  ENVELOPE
  127,                    // 10 LENGTH
  127,                    // 11 MODULATION
  127,                    // 12 PITCH
  127,                    // 13 PAN
  127,                    // 14 VOL
  127,                    // 15 FILTER
  1,                      // 16 TYPE
  1,                      // 17 M PITCH
  1,                      // 18 TRANSPOSE
  10,                     // 19 OCTAVE
  12,                     // 20 SCALE
  2,                      // 21 SYNC
  400,                    // 22 BPM
  1,                      // 23 MOVE
  127,                    // 24 M VOL
  127,                    // 25 M FILTER
  2,                      // 26 SONG MODE
  127,                    // 27 REVERB LEVEL
  127,                    // 28 DELAY LEVEL
  127,                    // 29 CHORUS LEVEL
  127,                    // 30 FLANGER LEVEL
  127,                    // 31 TREMOLO LEVEL
  15,                     // 32 REVERB TYPE
  255,                    // 33 DELAY TIME
  15,                     // 34 CHORUS TYPE
  15,                     // 35 FLANGER TYPE
  15,                     // 36 TREMOLO TYPE
  127,                    // 37 RINGMOD LEVEL
  15,                     // 38 RINGMOD TYPE
  16,                     // 39 ADD NRXT SND
  127,                    // 40 DETUNE
  127,                    // 41 DISTORTION LEVEL
  15,                     // 42 DISTORTION TYPE
  127,                    // 43 BITCRUSHER LEVEL
  15                      // 44 BITCRUSHER TYPE

};

const int min_values[MAX_BARS] = {
  0,     // 0  SAMPLE
  0,     // 1  WAVETABLE
  0,     // 2  A
  0,     // 3  D
  0,     // 4  S
  0,     // 5  R
  0,     // 6  INI
  0,     // 7  END
  0,     // 8  REVERSE
  0,     // 9  ENVELOPE
  0,     // 10 LENGTH
  0,     // 11 MODULATION
  0,     // 12 PITCH
  -127,  // 13 PAN
  0,     // 14 VOL
  0,     // 15 FILTER
  0,     // 16 TYPE
  -1,    // 17 M PITCH
  -1,    // 18 TRANSPOSE
  0,     // 19 OCTAVE
  0,     // 20 SCALE
  0,     // 21 SYNC
  0,     // 22 BPM
  -1,    // 23 MOVE
  0,     // 24 M VOL
  0,     // 25 M FILTER
  0,     // 26 SONG MODE
  0,     // 27 REVERB LEVEL
  0,     // 28 DELAY LEVEL
  0,     // 29 CHORUS LEVEL
  0,     // 30 FLANGER LEVEL
  0,     // 31 TREMOLO LEVEL
  0,     // 32 REVERB TYPE
  0,     // 33 DELAY TIME
  0,     // 34 CHORUS TYPE
  0,     // 35 FLANGER TYPE
  0,     // 36 TREMOLO TYPE
  0,     // 37 RINGMOD LEVEL
  0,     // 38 RINGMOD TYPE
  1,     // 39 ADD NEXT SND
  -127,  // 40 DETUNE
  0,     // 41 DISTORTION LEVEL
  0,     // 42 DISTORTION TYPE
  0,     // 43 BITCRUSHER LEVEL
  0      // 44 BITCRUSHER TYPE
};



// RECORDATORIO: detune addnext y ismelodic denerían tener los índices del array 17 18 Y 19, pero por no renumerar todo...al final tendré que reordenarlo

int32_t ROTvalue[16][20] = {
//      sy                                sy   sy  sy
// SMP, WT, A, D,   S,   R,  INI,END, REV,ENV,LEN,MOD, PIT,PAN,VOL,FIL,TYPE, DETUNE,ADDNEXTSND,ISMELODIC
  { 16,  0, 0, 255, 255, 255, 0, 2047, 0,  0,  10, 64, 60, -99, 80, 0, 1 ,     0,         1,      0},  // VOICE #0
  { 17,  1, 0, 255, 255, 255, 0, 2047, 0,  1,  10, 34, 60, -31, 80, 0, 1 ,     0,         1,      0},  // VOICE #1
  { 18,  2, 0, 255, 255, 255, 0, 2047, 0,  2,  50, 64, 60,  90, 80, 0, 1 ,     0,         1,      0},  // VOICE #2
  { 19,  3, 0, 255, 255, 255, 0, 2047, 0,  3,  10, 64, 60,   0, 80, 0, 1 ,     0,         1,      0},  // VOICE #3
  { 20,  4, 0, 255, 255, 255, 0, 2047, 0,  0,  20, 64, 60,  20, 70, 0, 0 ,     0,         1,      0},  // VOICE #4
  { 21,  5, 0, 255, 255, 255, 0, 2047, 0,  1,  30, 34, 60,   0, 70, 0, 0 ,     0,         1,      0},  // VOICE #5
  { 22,  6, 0, 255, 255, 255, 0, 2047, 0,  2,  10, 64, 60,  30, 70, 0, 0 ,     0,         1,      0},  // VOICE #6
  { 23,  7, 0, 255, 255, 255, 0, 2047, 0,  3,  20, 84, 60,   0, 70, 0, 0 ,     0,         1,      0},  // VOICE #7

  { 24,  8, 0, 255, 255, 255, 0, 2047, 0,  0,  10, 54, 60,  66, 80, 0, 1 ,     0,         1,      1},   // VOICE #8
  { 25,  9, 0, 255, 255, 255, 0, 2047, 0,  1,  10, 64, 60,  66, 80, 0, 1 ,     0,         1,      1},   // VOICE #9
  { 26, 10, 0, 255, 255, 255, 0, 2047, 0,  2,  10, 64, 60,   0, 80, 0, 1 ,     0,         1,      1},   // VOICE #10
  { 27, 11, 0, 255, 255, 255, 0, 2047, 0,  3,  10, 64, 60, -20, 80, 0, 1 ,     0,         1,      1},   // VOICE #11
  { 28, 12, 0, 255, 255, 255, 0, 2047, 0,  0,  10, 74, 60,   0, 70, 0, 0 ,     0,         1,      1},   // VOICE #12
  { 29, 13, 0, 255, 255, 255, 0, 2047, 0,  1,  20, 64, 60, -10, 70, 0, 0 ,     0,         1,      1},   // VOICE #13
  { 30, 14, 0, 255, 255, 255, 0, 2047, 0,  2,  30, 74, 60,  30, 70, 0, 0 ,     0,         1,      1},   // VOICE #14
  { 31, 15, 0, 255, 255, 255, 0, 2047, 0,  3,  10, 64, 60, -30, 70, 0, 0 ,     0,         1,      1}    // VOICE #15
};


byte selected_rot = 0;
byte s_old_selected_rot = 1;

byte nkey;

static int show_bar = -1;
////////////////////////////// MISC
byte modeZ = 0;
int s_old_modeZ = -1;

bool playing = false;
bool pre_playing = false;
bool songing = false;  // switch to make load auto patterns++
bool recording = false;
bool shifting = false;

bool clearPADSTEP = true;
bool clearPATTERNPADS = true;
bool refreshPATTERN = true;
bool refreshSEQ = true;
bool refreshPADSTEP = true;
bool refreshMODES = true;
bool refresh_sound_bars = false;
bool refresh_global_bars = false;
bool refresh_rPage = true;

uint8_t old_vol = 0;

#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP 0xFC

//////////////////////////////////////////////////////////////////////////////////////////

void usb_host_task_wrapper(void* pvParameters) {

  for (;;) {
    usbh_task();
  }
  // this code never runs
  vTaskDelete(NULL);
}

static void task_LCD(void* pvParameters) {

  for (;;) {
    REFRESH_PAGE();
    REFRESH_STATUS();
    showLastTouched();
    clearLastTouched();
    read_touch();
    DO_KEYPAD();
    // vTaskDelay(1);
    if (show_bar > -1) {
      if (mRot[show_bar]->rPage == rPage) {
        do_drawBar(show_bar);
        vTaskDelay(1);
      }
    }

    if (refresh_sound_bars) {
      refresh_sound_bars = false;



      if (rPage == 0) {
        if (ROTvalue[selected_sound][16]) {
          do_drawBar(1);
        } else {
          do_drawBar(0);
        }
        do_drawBar(9);
        do_drawBar(10);
        do_drawBar(11);
        do_drawBar(12);
        do_drawBar(13);
        do_drawBar(14);
        do_drawBar(15);
        do_drawBar(16);
        do_drawBar(39);
        do_drawBar(40);

        do_drawBar(2);
        do_drawBar(3);
        do_drawBar(4);
        do_drawBar(5);
        do_drawBar(6);
        do_drawBar(7);
        do_drawBar(8);
      } else if (rPage == 1) {
        do_drawBar(17);
        do_drawBar(18);
        do_drawBar(19);
        do_drawBar(20);
        do_drawBar(21);
        do_drawBar(22);
        do_drawBar(23);
        do_drawBar(24);
        do_drawBar(25);
        do_drawBar(26);
      } else {
        do_drawBar(27);
        do_drawBar(28);
        do_drawBar(29);
        do_drawBar(30);
        do_drawBar(31);
        do_drawBar(32);
        do_drawBar(33);
        do_drawBar(34);
        do_drawBar(35);
        do_drawBar(36);
        do_drawBar(37);
        do_drawBar(38);
        do_drawBar(41);
        do_drawBar(42);
        do_drawBar(43);
        do_drawBar(44);
      }
    }

    REFRESH_KEYS();

    vTaskDelay(1);
  }
  // this code never runs
  vTaskDelete(NULL);
}

/////////////////////////////////////////////////////////////////////////
// Función auxiliar simple para leer 4 bytes
uint32_t read32(File& f) {
  uint32_t val;
  f.read((uint8_t*)&val, 4);
  return val;
}

// Función auxiliar simple para leer 2 bytes
uint16_t read16(File& f) {
  uint16_t val;
  f.read((uint8_t*)&val, 2);
  return val;
}

void cargarWavsEnPSRAM() {
  M5.Display.setTextSize(3);
  // INICIALIZACIÓN HARDWARE
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  //if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 40000000)) {
    M5.Display.println("Error: SD not detected!");
    Serial.println("Error SD");
    return;
  }

  M5.Display.println("SD OK. Loading samples...");

  // B) PROCESO DE CARGA
  File root = SD.open("/");
  File file = root.openNextFile();

  size_t currentRamUsage = 0;
  samplesTotal = 0;

  while (file && samplesTotal < MAX_SAMPLES_COUNT) {
    // Filtro: solo archivos .wav
    if (!file.isDirectory() && String(file.name()).endsWith(".wav")) {

      bool esValido = true;
      uint32_t dataSize = 0;
      uint32_t dataOffset = 0;

      // Validación de cabecera (Mono, 16bit, 44100)
      if (file.size() >= 44) {
        file.seek(22);
        if (read16(file) != 1) esValido = false;  // Mono
        file.seek(24);
        if (read32(file) != 44100) esValido = false;  // 44100 Hz
        file.seek(34);
        if (read16(file) != 16) esValido = false;  // 16 bits
      } else {
        esValido = false;
      }

      // Buscar chunk 'data' (inicio real del audio)
      if (esValido) {
        file.seek(12);
        while (file.position() < file.size() - 8) {
          uint8_t id[4];
          file.read(id, 4);
          uint32_t len = read32(file);

          if (id[0] == 'd' && id[1] == 'a' && id[2] == 't' && id[3] == 'a') {
            dataSize = len;
            dataOffset = file.position();
            break;
          }
          file.seek(file.position() + len);
        }
        if (dataSize == 0) esValido = false;
      }

      // Cargar en RAM si todo es correcto
      if (esValido) {
        if ((currentRamUsage + dataSize) < RAM_LIMIT) {

          // Reservar memoria en PSRAM
          int16_t* ptr = (int16_t*)heap_caps_malloc(dataSize, MALLOC_CAP_SPIRAM);

          if (ptr != NULL) {
            file.seek(dataOffset);
            file.read((uint8_t*)ptr, dataSize);

            // --- RELLENAR TUS ARRAYS ---
            SAMPLES[samplesTotal] = ptr;
            //SAMPLE_NAMES[samplesTotal] = String(file.name());
            SAMPLE_NAMES[samplesTotal] = String(samplesTotal) + " " + String(file.name());

            ENDS[samplesTotal] = (dataSize / 2) - 1;

            currentRamUsage += dataSize;
            samplesTotal++;

            Serial.printf("Cargado [%d]: %s (%d bytes)\n", samplesTotal - 1, file.name(), dataSize);
          }
        } else {
          Serial.println("Memoria llena!");
          file.close();
          break;
        }
      }
    }
    file.close();
    file = root.openNextFile();
  }
  M5.Display.printf("Loaded: %d Samples\n", samplesTotal);
  M5.Display.setTextSize(1);
  delay(1000);
  Serial.printf("PSRAM despues cargar WAVS: %d\n", ESP.getFreePsram());
}

void comprobar_jack() {
  if (millis() - lastCheck > 2000) {
    lastCheck = millis();

    uint8_t val = 0;
    if (M5.In_I2C.readRegister(ADDR_EXP, REG_INPUT, &val, 1, 400000)) {
      bool jack = (val & PIN_JACK);
      if (jack != old_jack) {
        if (jack) {
          M5.In_I2C.bitOff(ADDR_EXP, REG_OUTPUT, PIN_SPK, 400000);
        } else {
          M5.In_I2C.bitOn(ADDR_EXP, REG_OUTPUT, PIN_SPK, 400000);
        }
        old_jack = jack;
      }
    }
  }
}


//////////////////////////////  S E T U P  //////////////////////////////

void setup() {

  // Serial
  Serial.begin(115200);
  delay(200);

  // M5UNIFIED
  auto cfg = M5.config();
  cfg.external_spk = true;  // Salida de línea/cascos
  //cfg.internal_spk = false;
  M5.begin(cfg);

  // AUDIO
  auto spk_cfg = M5.Speaker.config();
  spk_cfg.sample_rate = SAMPLE_RATE;
  spk_cfg.stereo = true;
  M5.Speaker.config(spk_cfg);
  M5.Speaker.setVolume(255);
  M5.Speaker.setAllChannelVolume(255);

  // FX
  myReverb.init();
  myDelay.init(88200);  // 2 segundos
  myChorus.init();
  myFlanger.init();
  myTremolo.init();
  myRingMod.init();
  myDistortion.init();
  myBitcrusher.init();
  
  // DISPLAY
  M5.Display.setBrightness(255);
  M5.Display.setRotation(1);
  //M5.Display.setFont(&fonts::Font2);
  M5.Display.setTextSize(2);
  init_colors();
  M5.Display.fillScreen(TFT_BLACK);

  // Load samples
  cargarWavsEnPSRAM();
  M5.Display.setTextSize(2);

  // AUDIO SYNTH SETUP
  synthESP32_begin();
  synthESP32_setMVol(0);  // inicializar en silencio

  // Set 16 voices
  // for (byte f = 0; f < 16; f++) {
  //   setSound(f);
  // }
  setSoundALL();
  initADSR();

  synthESP32_setMFilter(master_filter);

  // SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar el sistema de archivos SPIFFS");
  }

  // not used
  //cargarDatos(); // Carga desde flash 16 memory (patterns + sounds)

  // USB MIDI HOST
  Serial.println("Inicializando USB Host para MIDI...");
  ESP_LOGI("", "Logs del sistema USB Host activados.");
  usbh_setup(show_config_desc_full);

  // Tarea que ejecutará la función usb_host_task_wrapper
  xTaskCreatePinnedToCore(
    usb_host_task_wrapper,
    "USB Host Task",
    4096,
    NULL,
    5,
    &usbTaskHandle,
    0);

  // DISPLAY 2

  M5.Display.fillScreen(TFT_BLACK);
  waveSprite.createSprite(WAVE_WIDTH, WAVE_HEIGHT);

  fillBPOS();

  drawScreen1_ONLY1();

  draw_sound_bars2();
  draw_global_bars2();

  // tool to paint waves faster
  for (int i = 0; i < MAX_SAMPLES_COUNT; i++) {
    analizarOnda(0, i);
  }
  for (int i = 0; i < WT_COUNT; i++) {
    analizarOnda(1, i);
  }

  // Tarea que ejecutará las funciones de pantalla (tactil, procesar y actualizar display)
  xTaskCreatePinnedToCore(
    task_LCD,        // Function
    "LCD_REFRESH",   // Task name
    4096,            // Stack size
    NULL,            // Parameters
    4,               // Priority
    &usbTaskHandle,  // Handle
    1                // Core
  );

  // SEQUENCER
  for (byte a = 0; a < 16; a++) {
    for (byte b = 0; b < 16; b++) {
      melodic[a][b] = 60;
    }
  }
  // rest of the parameters
  for (byte f=0;f<16;f++){
    detune[f]=ROTvalue[f][17];
    addnextsnd[f]=ROTvalue[f][18];
    isMelodic[f]=ROTvalue[f][19];
  }

  // TIMER SEQ
  seq.setCallback(mySEQ);
  seq.setBPM(bpm);
  

  // // Basic demo test pattern
  // pattern[0]=B00010001<<8 | B10010001;
  // pattern[6]=B10110101<<8 | B10001111;
  // pattern[3]=B00000110<<8 | B00000000;
  // pattern[7]=B00000000<<8 | B01001000;

  // // Start
  // seq.start();
  // sstep=firstStep;
  // refreshPADSTEP=true;
  // playing=true;


  // Expander: headphones and speaker
  uint8_t dirReg = 0;
  M5.In_I2C.readRegister(ADDR_EXP, REG_CONFIG, &dirReg, 1, 400000);
  dirReg |= PIN_SPK;
  dirReg &= ~PIN_JACK;
  M5.In_I2C.writeRegister(ADDR_EXP, REG_CONFIG, &dirReg, 1, 400000);

  delay(400);
  synthESP32_setMVol(master_vol);
}
//////////////////////////////  L O O P  //////////////////////////////

void loop() {

  M5.update();

  comprobar_jack();

}

//////////////////////////////////////////////////////////////////////////

int mapRounded(long x, long in_min, long in_max, long out_min, long out_max) {
  long denominator = (in_max - in_min);
  long numerator = (x - in_min) * (out_max - out_min);
  return (int)((numerator + (denominator / 2)) / denominator + out_min);
}

