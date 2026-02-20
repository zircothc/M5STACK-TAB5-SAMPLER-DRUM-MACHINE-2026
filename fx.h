// Función de soft clipping en enteros
inline int16_t soft_clip(int32_t sample) {
    if (sample > 32767) return 32767 - ((sample - 32767) >> 3);
    if (sample < -32768) return -32768 + ((-32768 - sample) >> 3);
    return sample;
}


#include <vector>
#include <cmath>
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////     REVERB    /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ProReverb {
public:
    // VARIABLES
    float room_size = 0.5f;
    float damping   = 0.5f;
    float wet       = 0.3f;
    float dry       = 1.0f;
    float width     = 1.0f;

    const char* presetName = "Default";

private:

    struct ReverbPreset {
        const char* name;
        float r_size; // Room Size
        float damp;   // Damping
        float wet;    // Wet Level
        float dry;    // Dry Level
        float wid;    // Width
    };

    // PRESETS
    const ReverbPreset presets[16] = {
        // Nombre              Size   Damp   Wet    Dry    Width
        {"0 Off/Bypass",      0.00f, 0.00f, 0.00f, 1.00f, 0.00f}, // 0: Apagado
        {"1 Small Room",      0.30f, 0.20f, 0.20f, 1.00f, 0.80f}, // 1: Habitación pequeña (seca)
        {"2 Drum Booth",      0.45f, 0.60f, 0.35f, 1.00f, 1.00f}, // 2: Cabina batería (opaca)
        {"3 Tiled Room",      0.40f, 0.05f, 0.40f, 1.00f, 1.00f}, // 3: Baño/Azulejos (brillante)
        {"4 Studio Plate",    0.60f, 0.10f, 0.45f, 0.90f, 1.00f}, // 4: Plate clásico ochentero
        {"5 Vocal Hall",      0.75f, 0.40f, 0.50f, 0.80f, 1.00f}, // 5: Hall suave para voces
        {"6 Large Hall",      0.82f, 0.50f, 0.60f, 0.70f, 1.00f}, // 6: Sala de conciertos
        {"7 Cathedral",       0.88f, 0.30f, 0.70f, 0.60f, 1.00f}, // 7: Catedral (larga y brillante)
        {"8 Deep Space",      0.93f, 0.10f, 0.80f, 0.50f, 1.00f}, // 8: Muy larga y etérea
        {"9 Dark Cave",       0.85f, 0.90f, 0.60f, 0.70f, 0.90f}, // 9: Cueva (muy grave/oscuro)
        {"10 Slapback",       0.20f, 0.00f, 0.50f, 1.00f, 0.50f}, // 10: Eco rápido tipo Rockabilly
        {"11 Ambient",        0.90f, 0.70f, 1.00f, 0.40f, 1.00f}, // 11: Música ambiental (mucha mezcla)
        {"12 Mono Spring",    0.50f, 0.10f, 0.40f, 1.00f, 0.00f}, // 12: Muelle mono (guitarra vieja)
        {"13 Wide Pad",       0.86f, 0.50f, 0.50f, 0.00f, 1.00f}, // 13: 100% Wet (sin señal original)
        {"14 Bright Hall",    0.78f, 0.05f, 0.45f, 0.90f, 1.00f}, // 14: Hall muy agudo
        {"15 Endless",        0.96f, 0.20f, 0.90f, 0.20f, 1.00f}  // 15: Cola casi infinita (Cuidado!)
    };

    static const int numcombs = 8;
    static const int numallpasses = 4;
    
    // BUFFER SIZES
    const int comb_lengths[numcombs] = {4464, 4752, 5108, 5424, 5688, 5964, 6228, 6468};
    const int allpass_lengths[numallpasses] = {900, 1364, 1764, 2224};
    const int stereo_spread = 92;

    struct Buffer {
        std::vector<float> data;
        int size;
        int idx = 0;
        float store = 0;
    };

    Buffer combL[numcombs];
    Buffer combR[numcombs];
    Buffer allpassL[numallpasses];
    Buffer allpassR[numallpasses];

public:

    void setPreset(uint8_t index) {
        if (index > 15) index = 0; // Protección
        room_size   = presets[index].r_size;
        damping     = presets[index].damp;
        wet         = presets[index].wet;
        dry         = presets[index].dry;
        width       = presets[index].wid;
        presetName  = presets[index].name;

    }

    void init() {
        Serial.printf("PSRAM pre REVERB: %d\n", ESP.getFreePsram());
        for(int i=0; i<numcombs; i++) {
            combL[i].size = comb_lengths[i];
            combL[i].data.resize(combL[i].size, 0);
            combR[i].size = comb_lengths[i] + stereo_spread;
            combR[i].data.resize(combR[i].size, 0);
        }
        for(int i=0; i<numallpasses; i++) {
            allpassL[i].size = allpass_lengths[i];
            allpassL[i].data.resize(allpassL[i].size, 0);
            allpassR[i].size = allpass_lengths[i] + stereo_spread;
            allpassR[i].data.resize(allpassR[i].size, 0);
        }
        Serial.printf("PSRAM POST REVERB: %d\n", ESP.getFreePsram());

        setPreset(8); 
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {

        if (wet <= 0.01f) {
            outL = inL; outR = inR; return;
        }

        float inputL = (float)inL * 0.015f; 
        float inputR = (float)inR * 0.015f;
        float outSampleL = 0;
        float outSampleR = 0;

        for(int i=0; i<numcombs; i++) {
            outSampleL += processComb(&combL[i], inputL);
            outSampleR += processComb(&combR[i], inputR);
        }

        for(int i=0; i<numallpasses; i++) {
            outSampleL = processAllPass(&allpassL[i], outSampleL);
            outSampleR = processAllPass(&allpassR[i], outSampleR);
        }

        float wet1 = wet * (width / 2.0f + 0.5f);
        float wet2 = wet * ((1.0f - width) / 2.0f);

        outL = (int32_t)(outSampleL * wet1 + outSampleR * wet2 + (float)inL * dry);
        outR = (int32_t)(outSampleR * wet1 + outSampleL * wet2 + (float)inR * dry);
    }

private:
    inline float processComb(Buffer *b, float input) {
        float output = b->data[b->idx];
        b->store = (output * (1.0f - damping)) + (b->store * damping);
        b->data[b->idx] = input + (b->store * room_size);
        if(++b->idx >= b->size) b->idx = 0;
        return output;
    }

    inline float processAllPass(Buffer *b, float input) {
        float buf_out = b->data[b->idx];
        float output = -input + buf_out;
        b->data[b->idx] = input + (buf_out * 0.5f);
        if(++b->idx >= b->size) b->idx = 0;
        return output;
    }
};

ProReverb myReverb;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////     DELAY     /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SimpleDelay {
  public:
    // VARIABLES  
    int32_t delaySamples = 12000; 
    int32_t feedback = 120;
    int32_t inputLevel = 160;

    int16_t *lBuffer = NULL;
    int16_t *rBuffer = NULL;
    int32_t len = 0;
    int32_t writeIndex = 0;

    volatile bool ready = false; 

    void init(int32_t sizeSamples) {
        Serial.printf("PSRAM PRE DELAY: %d\n", ESP.getFreePsram());
        ready = false;
        len = sizeSamples;

        // PSRAM
        lBuffer = (int16_t *)heap_caps_calloc(len, sizeof(int16_t), MALLOC_CAP_SPIRAM); 
        rBuffer = (int16_t *)heap_caps_calloc(len, sizeof(int16_t), MALLOC_CAP_SPIRAM);
        
        if (lBuffer == NULL) lBuffer = (int16_t *)calloc(len, sizeof(int16_t));
        if (rBuffer == NULL) rBuffer = (int16_t *)calloc(len, sizeof(int16_t));

        if (lBuffer != NULL && rBuffer != NULL) {
            writeIndex = 0;
            ready = true; 
        }
        Serial.printf("PSRAM POST DELAY: %d\n", ESP.getFreePsram());
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {
        if (!ready || lBuffer == NULL || rBuffer == NULL) {
            outL = 0; outR = 0; return; 
        }

        // READ HEAD
        int32_t readIndex = writeIndex - delaySamples;
        if (readIndex < 0) readIndex += len; 
        if (readIndex >= len) readIndex = 0; // Seguridad extra

        int16_t delayedL = lBuffer[readIndex];
        int16_t delayedR = rBuffer[readIndex];

        // WRITE HEAD
        int32_t nextL = ((inL * inputLevel) >> 8) + ((delayedL * feedback) >> 8);
        int32_t nextR = ((inR * inputLevel) >> 8) + ((delayedR * feedback) >> 8);

        lBuffer[writeIndex] = soft_clip(nextL);
        rBuffer[writeIndex] = soft_clip(nextR);

        // GO ON
        writeIndex++;
        if (writeIndex >= len) writeIndex = 0;

        // OUT
        outL = delayedL;
        outR = delayedR;
    }
    
    void setTime(int32_t samples) { if(ready && samples < len) delaySamples = samples > 0 ? samples : 1; }
    void setFeedback(uint8_t fb) { feedback = fb; }
    void setInputLevel(uint8_t level) { inputLevel = level; }
};


SimpleDelay myDelay;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////     CHORUS    /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PresetChorus {
public:
    // VARIABLES
    float c_speed     = 0.5f;     // Velocidad en Hz (Humanamente legible)
    float c_depth     = 150.0f;   // Profundidad en muestras
    float c_mix       = 0.5f;     // 0.0 Dry - 1.0 Wet
    float c_baseDelay = 600.0f;   // Retardo base en muestras
    float c_feedback  = 0.0f;     // Feedback (usualmente 0 en chorus)
    
    const char* presetName = "Default";

private:
    struct ChorusPreset {
        const char* name;
        float spd;      // Hz (0.1 a 5.0)
        float dep;      // Samples (0 a 1000)
        float mx;       // 0.0 a 1.0
        float b_del;    // Samples (400 a 1500)
        float fb;       // -0.9 a 0.9
    };

    // PRESETS
    const ChorusPreset presets[16] = {
        // Name               Hz       Samples  Mix     Base    Fback
        {"0 Bypass",          0.00f,   0.0f,    0.00f,  10.0f,  0.00f},
        {"1 Juno I",          0.50f,   180.0f,  0.50f,  600.0f, 0.00f}, // Lento y profundo (El clásico)
        {"2 Juno II",         0.80f,   120.0f,  0.50f,  500.0f, 0.00f}, // Más rápido, menos profundo
        {"3 Dimension D",     0.25f,   300.0f,  0.50f,  800.0f, 0.00f}, // Muy sutil y ancho
        {"4 Thick Pad",       0.30f,   400.0f,  0.45f,  900.0f, 0.00f}, // Mucho delay base = sonido gordo
        {"5 Fast Leslie",     4.50f,   60.0f,   0.60f,  300.0f, 0.00f}, // Vibrato rápido (Hammond)
        {"6 Slow Rotary",     0.90f,   150.0f,  0.50f,  450.0f, 0.00f}, // Rotatorio lento
        {"7 Vibrato",         3.00f,   80.0f,   1.00f,  200.0f, 0.00f}, // 100% Wet = Pitch bend
        {"8 Doubler",         0.00f,   0.0f,    0.50f,  1200.0f,0.00f}, // Estático, parece dos guitarras
        {"9 Deep Sea",        0.15f,   600.0f,  0.60f,  1000.0f,0.00f}, // Muy lento, barrido enorme
        {"10 Metallic",       0.60f,   100.0f,  0.60f,  20.0f,  0.70f}, // Delay corto + feedback = Metálico
        {"11 Warped Vinyl",   1.20f,   250.0f,  1.00f,  500.0f, 0.00f}, // Pitch inestable
        {"12 Bass Chorus",    0.40f,   100.0f,  0.30f,  400.0f, 0.00f}, // Sutil para no embarrar graves
        {"13 Alien Voice",    8.00f,   50.0f,   0.50f,  300.0f, 0.00f}, // Muy rápido (Casi anillo)
        {"14 Stereo Widen",   0.10f,   200.0f,  0.50f,  700.0f, -0.3f}, // Feedback negativo abre el estéreo
        {"15 Lush Strings",   0.60f,   220.0f,  0.70f,  850.0f, 0.10f}  // Un poco de feedback para "brillo"
    };

    // AUDIO ENGINE
    int16_t *lBuffer = NULL;
    int16_t *rBuffer = NULL;
    int32_t len = 0;
    int32_t writeIndex = 0;
    
    float lfoPhase = 0.0f;
    float sampleRate = 44100.0f; 
    volatile bool ready = false;

public:
    void setPreset(uint8_t index) {
        if (index > 15) index = 0;
        
        c_speed     = presets[index].spd;
        c_depth     = presets[index].dep;
        c_mix       = presets[index].mx;
        c_baseDelay = presets[index].b_del;
        c_feedback  = presets[index].fb;
        presetName  = presets[index].name;

        // LFO RESET
        lfoPhase = 0.0f;

    }

    void init(int32_t sizeSamples = 4096) {
        Serial.printf("PSRAM PRE CHORUS: %d\n", ESP.getFreePsram());
        ready = false;
        len = sizeSamples;
        
        lBuffer = (int16_t *)heap_caps_calloc(len, sizeof(int16_t), MALLOC_CAP_SPIRAM); 
        rBuffer = (int16_t *)heap_caps_calloc(len, sizeof(int16_t), MALLOC_CAP_SPIRAM);
        
        if (!lBuffer) lBuffer = (int16_t *)calloc(len, sizeof(int16_t));
        if (!rBuffer) rBuffer = (int16_t *)calloc(len, sizeof(int16_t));

        if (lBuffer && rBuffer) {
            writeIndex = 0;
            lfoPhase = 0;
            ready = true; 
            setPreset(1); 
        }
        Serial.printf("PSRAM POST CHORUS: %d\n", ESP.getFreePsram());
    }

    inline float readInterpolated(int16_t* buffer, float index) {
        int32_t idxInt = (int32_t)index;
        float idxFrac = index - idxInt;
        int32_t idxNext = idxInt + 1;
        if (idxNext >= len) idxNext = 0;

        float val1 = (float)buffer[idxInt];
        float val2 = (float)buffer[idxNext];
        return val1 + (val2 - val1) * idxFrac;
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {
        if (!ready) { outL = inL; outR = inR; return; }
        if (c_mix <= 0.01f) { outL = inL; outR = inR; return; }

        // LFO
        float phaseInc = c_speed / sampleRate;
        lfoPhase += phaseInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // SINE
        float lfoVal = sinf(lfoPhase * 6.2831853f);

        // CALC DELAYS
        float currentDelayL = c_baseDelay + (lfoVal * c_depth);
        float currentDelayR = c_baseDelay - (lfoVal * c_depth); // Fase invertida derecha

        // SAFETY
        if (currentDelayL < 2.0f) currentDelayL = 2.0f;
        if (currentDelayR < 2.0f) currentDelayR = 2.0f;

        // READ
        float readPosL = (float)writeIndex - currentDelayL;
        float readPosR = (float)writeIndex - currentDelayR;

        while (readPosL < 0.0f) readPosL += (float)len;
        while (readPosR < 0.0f) readPosR += (float)len;
        if (readPosL >= len) readPosL -= len;
        if (readPosR >= len) readPosR -= len;

        float wetSignalL = readInterpolated(lBuffer, readPosL);
        float wetSignalR = readInterpolated(rBuffer, readPosR);

        // FEEDBACK
        float nextL = (float)inL + (wetSignalL * c_feedback);
        float nextR = (float)inR + (wetSignalR * c_feedback);
        
        // SOFT CLIP
        if (nextL > 32000.0f) nextL = 32000.0f; if (nextL < -32000.0f) nextL = -32000.0f;
        if (nextR > 32000.0f) nextR = 32000.0f; if (nextR < -32000.0f) nextR = -32000.0f;

        lBuffer[writeIndex] = (int16_t)nextL;
        rBuffer[writeIndex] = (int16_t)nextR;

        // MIX
        outL = (int32_t)((inL * (1.0f - c_mix)) + (wetSignalL * c_mix));
        outR = (int32_t)((inR * (1.0f - c_mix)) + (wetSignalR * c_mix));

        writeIndex++;
        if (writeIndex >= len) writeIndex = 0;
    }
    
    // // Setters manuales
    // void setSpeed(float s) { c_speed = s; }
    // void setDepth(float d) { c_depth = d; }
    // void setMix(float m) { c_mix = m; }

};


PresetChorus myChorus;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////     FLANGER    ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PresetFlanger {
public:
    // VARIABLES
    float rate      = 0.5f;     
    float depth     = 1.0f;     
    float manual    = 2.0f;     
    float feedback  = 0.0f;     
    float mix       = 0.5f;     
    
    const char* presetName = "Default";

private:
    struct FlangerPreset {
        const char* name;
        float rate;     // Hz
        float depth;    // ms
        float manual;   // ms
        float feedback; // -1.0 a 1.0
        float mix;      // 0.0 a 1.0
    };

    // PRESETS
    const FlangerPreset presets[16] = {
        {"0 Bypass",          0.00f,   0.00f,    1.0f,      0.00f,    0.00f},
        {"1 Classic Jet",     0.20f,   3.00f,    2.0f,      0.60f,    0.50f}, 
        {"2 Slow Deep",       0.10f,   8.00f,    1.0f,      0.50f,    0.50f}, 
        {"3 Metallic Robot",  0.00f,   0.00f,    0.8f,      0.90f,    0.60f}, 
        {"4 Hollow Tube",     0.30f,   2.00f,    3.0f,     -0.70f,    0.50f}, 
        {"5 Fast Wobble",     4.00f,   1.00f,    2.0f,      0.30f,    0.50f}, 
        {"6 Through Zero",    0.25f,   1.50f,    0.5f,      0.40f,    0.70f}, 
        {"7 Underwater",      0.50f,   4.00f,    3.0f,      0.40f,    0.40f}, 
        {"8 Screamer",        0.15f,   2.00f,    1.0f,      0.92f,    0.50f}, 
        {"9 Stereo Wide",     0.20f,   3.00f,    5.0f,      0.20f,    0.50f}, 
        {"10 Dark Flange",    0.30f,   4.00f,    6.0f,      0.30f,    0.40f}, 
        {"11 Subtle Tape",    0.10f,   0.50f,    1.5f,      0.10f,    0.50f}, 
        {"12 Sci-Fi Alarm",   6.00f,   5.00f,    1.0f,      0.80f,    0.60f}, 
        {"13 Chorus-y",       0.40f,   2.00f,   10.0f,      0.10f,    0.50f}, 
        {"14 Reso Bass",      0.20f,   1.00f,    0.6f,      0.85f,    0.40f}, 
        {"15 Invert Mix",     0.25f,   3.00f,    2.0f,      0.50f,    1.00f}  
    };

    // AUDIO ENGINE
    int16_t *lBuffer = NULL;
    int16_t *rBuffer = NULL;
    int32_t len = 0;
    int32_t writeIndex = 0;
    
    float lfoPhase = 0.0f;
    float sampleRate = 44100.0f; 
    volatile bool ready = false;

public:
    void setPreset(uint8_t index) {
        if (index > 15) index = 0;
        rate     = presets[index].rate;
        depth    = presets[index].depth;
        manual   = presets[index].manual;
        feedback = presets[index].feedback;
        mix      = presets[index].mix;
        presetName = presets[index].name;
        
        // LFO RESET
        lfoPhase = 0.0f; 

    }

    void init(int32_t sizeSamples = 4096) {
        Serial.printf("PSRAM PRE FLANGER: %d\n", ESP.getFreePsram());
        ready = false;
        len = sizeSamples;

        lBuffer = (int16_t *)heap_caps_calloc(len, sizeof(int16_t), MALLOC_CAP_SPIRAM); 
        rBuffer = (int16_t *)heap_caps_calloc(len, sizeof(int16_t), MALLOC_CAP_SPIRAM);
        
        if (!lBuffer) lBuffer = (int16_t *)calloc(len, sizeof(int16_t));
        if (!rBuffer) rBuffer = (int16_t *)calloc(len, sizeof(int16_t));

        if (lBuffer && rBuffer) {
            writeIndex = 0;
            lfoPhase = 0;
            ready = true;
            setPreset(1); 
        }
        Serial.printf("PSRAM POST FLANGER: %d\n", ESP.getFreePsram());
    }

    inline float readInterpolated(int16_t* buffer, float index) {
        int32_t idxInt = (int32_t)index;
        float idxFrac = index - idxInt;
        int32_t idxNext = idxInt + 1;
        if (idxNext >= len) idxNext = 0;

        float val1 = (float)buffer[idxInt];
        float val2 = (float)buffer[idxNext];
        return val1 + (val2 - val1) * idxFrac;
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {
        if (!ready) { outL = inL; outR = inR; return; }
        if (mix <= 0.01f) { outL = inL; outR = inR; return; }

        float phaseInc = rate / sampleRate;
        lfoPhase += phaseInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        float lfoVal = sinf(lfoPhase * 6.2831853f); 

        float msToSamples = sampleRate / 1000.0f;
        
        float oscL = lfoVal * (depth * 0.5f);
        float delayMsL = manual + (depth * 0.5f) + oscL;
        
        float oscR = -lfoVal * (depth * 0.5f);
        float delayMsR = manual + (depth * 0.5f) + oscR;

        if (delayMsL < 0.1f) delayMsL = 0.1f;
        if (delayMsR < 0.1f) delayMsR = 0.1f;

        float delaySamplesL = delayMsL * msToSamples;
        float delaySamplesR = delayMsR * msToSamples;
        
        float readPosL = (float)writeIndex - delaySamplesL;
        float readPosR = (float)writeIndex - delaySamplesR;

        while (readPosL < 0.0f) readPosL += (float)len;
        while (readPosR < 0.0f) readPosR += (float)len;
        if (readPosL >= len) readPosL -= len;
        if (readPosR >= len) readPosR -= len;

        float wetL = readInterpolated(lBuffer, readPosL);
        float wetR = readInterpolated(rBuffer, readPosR);

        // Feedback con Soft Clip básico
        float nextL = (float)inL + (wetL * feedback);
        float nextR = (float)inR + (wetR * feedback);

        if (nextL > 32000.0f) nextL = 32000.0f; if (nextL < -32000.0f) nextL = -32000.0f;
        if (nextR > 32000.0f) nextR = 32000.0f; if (nextR < -32000.0f) nextR = -32000.0f;

        lBuffer[writeIndex] = (int16_t)nextL;
        rBuffer[writeIndex] = (int16_t)nextR;

        outL = (int32_t)((inL * (1.0f - mix)) + (wetL * mix));
        outR = (int32_t)((inR * (1.0f - mix)) + (wetR * mix));

        writeIndex++;
        if (writeIndex >= len) writeIndex = 0;
    }
    
    void setManual(float ms) { manual = ms; }
    void setFeedback(float fb) { feedback = fb; } 
};

PresetFlanger myFlanger;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////     TREMOLO    ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PresetTremolo {
public:
    // VARIABLES
    float t_rate    = 4.0f;     
    float t_depth   = 1.0f;     
    float t_shape   = 0.0f;
    float t_spread  = 0.0f;     
    
    const char* presetName = "Default";

private:
    struct TremoloData {
        const char* name;
        float p_rate;   
        float p_depth;  
        float p_shape;  
        float p_spread; 
    };

    // PRESETS
    const TremoloData presets[16] = {
        {"0 Bypass",        0.00f,  0.00f,  0.0f,  0.0f},
        {"1 Vintage Amp",   5.00f,  0.70f,  0.0f,  0.0f}, 
        {"2 Deep Pulse",    3.00f,  0.95f,  0.3f,  0.0f}, 
        {"3 Helicopter",    8.00f,  1.00f,  1.0f,  0.0f},
        {"4 Slow Pan",      0.50f,  0.90f,  0.0f,  1.0f}, 
        {"5 Ping Pong",     3.00f,  1.00f,  0.0f,  1.0f}, 
        {"6 Hard Chop LR",  6.00f,  1.00f,  1.0f,  1.0f}, 
        {"7 Nervous",       12.0f,  0.60f,  0.0f,  0.2f}, 
        {"8 Subtle Move",   0.20f,  0.40f,  0.0f,  1.0f}, 
        {"9 Trance Gate",   4.00f,  1.00f,  1.0f,  0.0f}, 
        {"10 50s Ballad",   2.00f,  0.50f,  0.0f,  0.0f}, 
        {"11 Machine Gun",  15.0f,  1.00f,  1.0f,  0.0f},  
        {"12 ...       ",   1.00f,  0.25f,  1.0f,  0.0f}, 
        {"13 ...       ",   2.00f,  0.50f,  0.0f,  1.0f}, 
        {"14 ...       ",   3.00f,  0.75f,  1.0f,  1.0f}, 
        {"15 ...       ",   4.00f,  1.00f,  0.0f,  0.0f} 
    };

    float lfoPhase = 0.0f;
    float sampleRate = 44100.0f; 
    volatile bool ready = false;

public:
    void setPreset(uint8_t index) {
        if (index > 11) index = 0;
        
        t_rate    = presets[index].p_rate;
        t_depth   = presets[index].p_depth;
        t_shape   = presets[index].p_shape;
        t_spread  = presets[index].p_spread;
        presetName = presets[index].name;
  
        lfoPhase = 0.0f; 
    }

    void init(float sr = 44100.0f) {
        Serial.printf("PSRAM PRE REMOLO: %d\n", ESP.getFreePsram());
        sampleRate = sr;
        ready = true;
        Serial.printf("PSRAM POST TREMOLO: %d\n", ESP.getFreePsram());
        setPreset(3); 
    }

    // Función auxiliar para generar onda cuadrada suave (Anti-Aliasing)
    inline float getLFOValue(float phase, float shape) {
        float rawSine = sinf(phase * 6.2831853f);
        float amplification = 1.0f + (shape * 15.0f); 
        float saturated = rawSine * amplification;

        if (saturated > 1.0f) saturated = 1.0f;
        if (saturated < -1.0f) saturated = -1.0f;

        return (saturated * 0.5f) + 0.5f;
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {
        if (!ready || t_depth <= 0.01f) { 
            outL = inL; outR = inR; return; 
        }

        // LFO
        float phaseInc = t_rate / sampleRate;
        lfoPhase += phaseInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // GET LFO
        float lfoValL = getLFOValue(lfoPhase, t_shape);

        // RIGHT CHANNEL
        float lfoValR = lfoValL;
        if (t_spread > 0.01f) {
             float phaseR = lfoPhase + (t_spread * 0.5f); 
             if (phaseR >= 1.0f) phaseR -= 1.0f;
             lfoValR = getLFOValue(phaseR, t_shape);
        }

        // GAIN
        float gainL = 1.0f - (lfoValL * t_depth);
        float gainR = 1.0f - (lfoValR * t_depth);

        outL = (int32_t)((float)inL * gainL);
        outR = (int32_t)((float)inR * gainR);
    }
    
    void setRate(float hz) { t_rate = hz; }
    void setDepth(float d) { t_depth = d; }
};

PresetTremolo myTremolo;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////     BITCRUSHER     ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct BitcrusherPreset {
    const char* name;
    float p_bits;   
    float p_rate;  
    float p_drive;  
    float p_mix; 
};

// PRESETS
static const BitcrusherPreset BitcrusherPresets[16] = {
    {"0 Bypass",        16.0f,  1.0f,   1.0f,  0.0f}, // Clean
    {"1 Vintage 12bit", 12.0f,  1.0f,   1.0f,  1.0f}, // MPC Style
    {"2 8-Bit Console", 8.00f,  1.0f,   1.0f,  1.0f}, // NES Style
    {"3 Arcade",        8.00f,  2.5f,   1.2f,  1.0f}, // Old Arcade
    {"4 Lo-Fi Radio",   6.00f,  4.0f,   2.0f,  0.8f}, 
    {"5 Speak & Spell", 4.00f,  6.0f,   1.0f,  1.0f}, 
    {"6 Destruction",   3.00f,  1.0f,   5.0f,  1.0f}, // Hard Fuzz
    {"7 Aliasing Bass", 12.0f,  12.0f,  1.0f,  1.0f}, // Metalic ring
    {"8 Toxic Waste",   2.00f,  2.0f,   3.0f,  1.0f}, 
    {"9 Subtle Grit",   10.0f,  1.5f,   1.2f,  0.4f}, // Mix 40%
    {"10 Broken Cable", 16.0f,  25.0f,  1.0f,  1.0f}, // Just sample reduction
    {"11 Digital Fart", 1.50f,  5.0f,   1.0f,  1.0f},  
    {"12 ...       ",   16.0f,  1.0f,   1.0f,  0.0f}, 
    {"13 ...       ",   16.0f,  1.0f,   1.0f,  0.0f}, 
    {"14 ...       ",   16.0f,  1.0f,   1.0f,  0.0f}, 
    {"15 ...       ",   16.0f,  1.0f,   1.0f,  0.0f} 
};

class PresetBitcrusher {
public:
    // VARIABLES
    float b_bits    = 16.0f;    // Resolución (1.0 a 16.0)
    float b_rate    = 1.0f;     // Sample Rate Divisor (1.0 a 50.0+)
    float b_drive   = 1.0f;     // Ganancia de entrada (1.0 a 10.0)
    float b_mix     = 1.0f;     // Dry/Wet (0.0 a 1.0)
    
    const char* presetName = "Default";

private:

    float sampleCounter = 0.0f;
    float currentHoldL = 0.0f;
    float currentHoldR = 0.0f;
    float sampleRate = 44100.0f; 
    volatile bool ready = false;

public:
    void setPreset(uint8_t index) {
        if (index > 11) index = 0;
        
        b_bits     = BitcrusherPresets[index].p_bits;
        b_rate     = BitcrusherPresets[index].p_rate;
        b_drive    = BitcrusherPresets[index].p_drive;
        b_mix      = BitcrusherPresets[index].p_mix;
        presetName = BitcrusherPresets[index].name;
  
        sampleCounter = 0.0f; 
    }

    void init(float sr = 44100.0f) {
        Serial.printf("PSRAM PRE CRUSHER: %d\n", ESP.getFreePsram());
        sampleRate = sr;
        ready = true;
        Serial.printf("PSRAM POST CRUSHER: %d\n", ESP.getFreePsram());
    }

    // Función Inline para el proceso de reducción de bits
    inline float crushSample(float sample, float steps, float drive) {

        sample *= drive;
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        float crushed = floorf(sample * steps) / steps;
        
        return crushed;
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {
        // Bypass lógico si mix es 0 o bits son muy altos y rate es 1
        if (!ready || (b_mix <= 0.01f)) { 
            outL = inL; outR = inR; return; 
        }


        sampleCounter += 1.0f;
        
        if (sampleCounter >= b_rate) {
            sampleCounter -= b_rate; 
            
            // Normalizar a float -1.0 a 1.0
            float fL = (float)inL / 32768.0f;
            float fR = (float)inR / 32768.0f;
            
            float steps = powf(2.0f, b_bits); 

            currentHoldL = crushSample(fL, steps, b_drive);
            currentHoldR = crushSample(fR, steps, b_drive);
        }

        // MIX
        int32_t wetL = (int32_t)(currentHoldL * 32767.0f);
        int32_t wetR = (int32_t)(currentHoldR * 32767.0f);

        if (b_mix >= 0.99f) {
            outL = wetL;
            outR = wetR;
        } else {
            outL = (int32_t)(inL * (1.0f - b_mix) + wetL * b_mix);
            outR = (int32_t)(inR * (1.0f - b_mix) + wetR * b_mix);
        }
    }
    
    void setBits(float bits) { b_bits = bits; }
    void setRate(float rate) { b_rate = rate; }
    void setDrive(float drive) { b_drive = drive; }
    void setMix(float mix) { b_mix = mix; }
};

PresetBitcrusher myBitcrusher;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////      DISTORTION    ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DistortionPreset {
    const char* name;
    float p_drive;   
    float p_tone;  
    float p_level;  
    float p_mix; 
};

// PRESETS
static const DistortionPreset DistortionPresets[16] = {
    {"0 Bypass",        1.0f,   1.0f,   1.0f,  0.0f}, 
    {"1 Warm Tube",     3.0f,   0.8f,   0.9f,  1.0f}, // Saturación suave
    {"2 Blues Driver",  8.0f,   0.7f,   0.8f,  1.0f}, // Overdrive clásico
    {"3 Screamer",      15.0f,  0.5f,   0.7f,  1.0f}, // Medios enfocados
    {"4 Hard Rock",     25.0f,  0.6f,   0.6f,  1.0f}, 
    {"5 Fuzz Face",     40.0f,  0.4f,   0.5f,  1.0f}, // Mucha ganancia, oscuro
    {"6 Heavy Metal",   50.0f,  0.9f,   0.5f,  1.0f}, // Ganancia alta, brillante
    {"7 Parallel Dirt", 20.0f,  0.6f,   0.8f,  0.5f}, // Mezcla limpia + sucia
    {"8 Bass Grit",     5.0f,   0.3f,   1.0f,  0.8f}, // Solo graves saturados
    {"9 Broken Speaker",60.0f,  1.0f,   0.3f,  1.0f}, // Destrucción total
    {"10 Muffled",      10.0f,  0.1f,   0.9f,  1.0f}, // Tono cerrado
    {"11 ...       ",   1.0f,   1.0f,   1.0f,  0.0f}, 
    {"12 ...       ",   1.0f,   1.0f,   1.0f,  0.0f}, 
    {"13 ...       ",   1.0f,   1.0f,   1.0f,  0.0f}, 
    {"14 ...       ",   1.0f,   1.0f,   1.0f,  0.0f}, 
    {"15 ...       ",   1.0f,   1.0f,   1.0f,  0.0f} 
};

class PresetDistortion {
public:
    // VARIABLES
    float d_drive   = 1.0f;     // Cantidad de distorsión (1.0 a 50.0)
    float d_tone    = 1.0f;     // Brillo / Filtro (0.1 a 1.0, donde 1.0 es abierto)
    float d_level   = 1.0f;     // Volumen de salida compensado
    float d_mix     = 1.0f;     // Dry/Wet
    
    const char* presetName = "Default";

private:

    float sampleRate = 44100.0f; 

    float filterStateL = 0.0f;
    float filterStateR = 0.0f;
    
    volatile bool ready = false;

public:
    void setPreset(uint8_t index) {
        if (index > 11) index = 0;
        
        d_drive    = DistortionPresets[index].p_drive;
        d_tone     = DistortionPresets[index].p_tone;
        d_level    = DistortionPresets[index].p_level;
        d_mix      = DistortionPresets[index].p_mix;
        presetName = DistortionPresets[index].name;
    }

    void init(float sr = 44100.0f) {
        Serial.printf("PSRAM PRE DIST: %d\n", ESP.getFreePsram());
        sampleRate = sr;
        ready = true;
        Serial.printf("PSRAM POST DIST: %d\n", ESP.getFreePsram());
        //setPreset(1); 
    }

    inline float softClip(float sample) {
        if (sample < -1.0f) return -0.66f; // Protección hard limit
        if (sample > 1.0f) return 0.66f;
        
        // Fórmula mágica de soft clipping eficiente
        return sample / (1.0f + fabsf(sample));
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {
        if (!ready || (d_mix <= 0.01f && d_drive <= 1.1f)) { 
            outL = inL; outR = inR; return; 
        }

        float fL = (float)inL / 32768.0f;
        float fR = (float)inR / 32768.0f;

        float driveL = fL * d_drive;
        float driveR = fR * d_drive;

        float wetL = softClip(driveL);
        float wetR = softClip(driveR);

        filterStateL += (wetL - filterStateL) * d_tone;
        filterStateR += (wetR - filterStateR) * d_tone;
        wetL = filterStateL;
        wetR = filterStateR;

        wetL *= d_level;
        wetR *= d_level;

        int32_t finalWetL = (int32_t)(wetL * 32767.0f);
        int32_t finalWetR = (int32_t)(wetR * 32767.0f);

        // MIX
        if (d_mix >= 0.99f) {
            outL = finalWetL;
            outR = finalWetR;
        } else {
            outL = (int32_t)(inL * (1.0f - d_mix) + finalWetL * d_mix);
            outR = (int32_t)(inR * (1.0f - d_mix) + finalWetR * d_mix);
        }
    }
    
    void setDrive(float drive) { d_drive = drive; }
    void setTone(float tone) { d_tone = tone; }
    void setLevel(float level) { d_level = level; }
    void setMix(float mix) { d_mix = mix; }
};

PresetDistortion myDistortion;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////    RING MODULATOR  ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RingModPreset {
    const char* name;
    float p_freq;   
    float p_mix;  
    float p_shape;
    float p_drive; 
};

static const RingModPreset RingModPresets[16] = {
    {"0 Bypass",        0.0f,    0.0f,  0.0f,  1.0f}, 
    {"1 Classic Bell",  440.0f,  1.0f,  0.0f,  1.0f}, 
    {"2 Deep Growl",    50.0f,   0.8f,  0.2f,  1.2f}, 
    {"3 Dalek Voice",   30.0f,   1.0f,  0.0f,  1.5f}, 
    {"4 Iron Pipe",     800.0f,  1.0f,  0.0f,  1.0f}, 
    {"5 Broken Radio",  1200.0f, 0.5f,  0.8f,  1.0f}, 
    {"6 8-Bit Chaos",   220.0f,  1.0f,  1.0f,  1.0f}, 
    {"7 Tremolo",       4.0f,    0.5f,  0.0f,  1.0f}, 
    {"8 Sci-Fi Alarm",  600.0f,  1.0f,  0.5f,  1.0f}, 
    {"9 Submarine",     100.0f,  1.0f,  0.0f,  1.2f}, 
    {"10 High Sparkle", 2400.0f, 0.4f,  0.0f,  1.0f}, 
    {"11 Chopper",      15.0f,   1.0f,  1.0f,  1.0f}, 
    {"12 User 1",       440.0f,  0.5f,  0.0f,  1.0f}, 
    {"13 User 2",       440.0f,  0.5f,  0.0f,  1.0f}, 
    {"14 User 3",       440.0f,  0.5f,  0.0f,  1.0f}, 
    {"15 User 4",       440.0f,  0.5f,  0.0f,  1.0f} 
};

class PresetRingModulator {
public:
    // VARS
    float r_freq    = 440.0f;
    float r_mix     = 1.0f;
    float r_shape   = 0.0f;
    float r_drive   = 1.0f;
    
    const char* presetName = "Default";

private:
    float oscillatorPhase = 0.0f;
    float sampleRate = 44100.0f; 
    volatile bool ready = false;

public:
    void setPreset(uint8_t index) {
        if (index > 15) index = 0;
        
        r_freq     = RingModPresets[index].p_freq;
        r_mix      = RingModPresets[index].p_mix;
        r_shape    = RingModPresets[index].p_shape;
        r_drive    = RingModPresets[index].p_drive;
        presetName = RingModPresets[index].name;

    }

    void init(float sr = 44100.0f) {
        Serial.printf("PSRAM PRE RINGMOD: %d\n", ESP.getFreePsram());
        sampleRate = sr;
        ready = true;
        Serial.printf("PSRAM POST RINGMOD: %d\n", ESP.getFreePsram());
    }

    inline float getCarrier(float phase, float shape) {
        float raw = sinf(phase * 6.2831853f);
        if (shape > 0.01f) {
            float gain = 1.0f + (shape * 20.0f); 
            raw *= gain;
            if (raw > 1.0f) raw = 1.0f;
            if (raw < -1.0f) raw = -1.0f;
        }
        return raw;
    }

    void process(int16_t inL, int16_t inR, int32_t &outL, int32_t &outR) {

        if (!ready || r_mix <= 0.001f) { 
            outL = inL; outR = inR; return; 
        }

        float fL = ((float)inL * 0.000030517f) * r_drive; // 1/32768 optimizado
        float fR = ((float)inR * 0.000030517f) * r_drive;

        float phaseInc = r_freq / sampleRate;
        oscillatorPhase += phaseInc;
        if (oscillatorPhase >= 1.0f) oscillatorPhase -= 1.0f;

        float carrierVal = getCarrier(oscillatorPhase, r_shape);
        float wetL = fL * carrierVal;
        float wetR = fR * carrierVal;

        int32_t wetIntL = (int32_t)(wetL * 32767.0f);
        int32_t wetIntR = (int32_t)(wetR * 32767.0f);

        if (r_mix >= 0.99f) {
            outL = wetIntL;
            outR = wetIntR;
        } else {
            outL = inL + (int32_t)((wetIntL - inL) * r_mix);
            outR = inR + (int32_t)((wetIntR - inR) * r_mix);
        }
    }
    
    void setFreq(float freq) { r_freq = freq; }
    void setMix(float mix) { r_mix = mix; }
    void setShape(float shape) { r_shape = shape; }
    void setDrive(float drive) { r_drive = drive; }
};

PresetRingModulator myRingMod;