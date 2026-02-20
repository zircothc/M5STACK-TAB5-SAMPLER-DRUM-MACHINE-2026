// Tabla de frecuencias MIDI (0-127)
const float midiFrequencies[128] = {
    8.18, 8.66, 9.18, 9.72, 10.3, 10.91, 11.56, 12.25, 12.98, 13.75, 14.57, 15.43,
    16.35, 17.32, 18.35, 19.45, 20.6, 21.83, 23.12, 24.5, 25.96, 27.5, 29.14, 30.87,
    32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49.0, 51.91, 55.0, 58.27, 61.74,
    65.41, 69.3, 73.42, 77.78, 82.41, 87.31, 92.5, 98.0, 103.83, 110.0, 116.54, 123.47,
    130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.0, 196.0, 207.65, 220.0, 233.08, 246.94,
    261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.0, 415.3, 440.0, 466.16, 493.88,
    523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 880.0, 932.33, 987.77,
    1046.5, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760.0, 1864.66, 1975.53,
    2093.0, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.0, 3729.31, 3951.07
};

float PITCH_TABLE_FTW[128]; 

void precalculate_ftw_table() {
    float constante = 4294967296.0f / 44100.0f; // 2^32 / SampleRate
    
    for (int i = 0; i < 128; i++) {
        PITCH_TABLE_FTW[i] = PITCH_FREQS_HZ[i] * constante;
    }
}

// Referencia rápida al valor base de frecuencia (Do4 / C4)
const float BASE_FREQ_INV = 1.0f / 261.63f;

void synthESP32_begin(){

  // 16 filters + master L & R (18)
  for (int fi=0; fi < 18; fi++) {
    FILTROS[fi].setResonance(reso);
    FILTROS[fi].setCutoffFreq(cutoff);
  }
  // synth engine

  precalculate_ftw_table();

  for (int ch = 0; ch < 16; ch++) {
    PITCH[ch] = 255;
    FTW[ch] = 0;
    PCW[ch] = 0;
    AMP[ch] = 0;
    EFTW[ch] = 0;
    EPCW[ch] = 0;   
  }

  // Highest possible priority for realtime audio task
  xTaskCreatePinnedToCore(
                  audio_task,       //Function to implement the task 
                  "audio", //Name of the task
                  8000,       //Stack size in words 
                  NULL,       //Task input parameter 
                  configMAX_PRIORITIES - 1,          //Priority of the task 
                  NULL,       //Task handle.
                  0);         //Core where the task should run 

  // Iniciar NEWENDS
  //memcpy(NEWENDS, ENDS, sizeof(ENDS));

}

static void write_buffer() {
    for (int i = 0; i < DMA_BUF_LEN; i++) {
        
        // FX
        int32_t DRUMTOTAL_L = 0;   // Master Dry
        int32_t DRUMTOTAL_R = 0;
        int32_t REVERB_SEND_L = 0; // Bus Envío Reverb
        int32_t REVERB_SEND_R = 0;
        int32_t DELAY_SEND_L = 0;  // Bus Envío Delay
        int32_t DELAY_SEND_R = 0;
        int32_t CHORUS_SEND_L = 0;  // Bus Envío Chorus
        int32_t CHORUS_SEND_R = 0;
        int32_t FLANGER_SEND_L = 0;  // Bus Envío Flanger
        int32_t FLANGER_SEND_R = 0;
        int32_t TREMOLO_SEND_L = 0;  // Bus Envío Tremolo
        int32_t TREMOLO_SEND_R = 0;
        int32_t RINGMOD_SEND_L = 0;  // Bus Envío Ring mod
        int32_t RINGMOD_SEND_R = 0;
        int32_t DISTORTION_SEND_L = 0;  // Bus Envío Distortion
        int32_t DISTORTION_SEND_R = 0;
        int32_t BITCRUSHER_SEND_L = 0;  // Bus Envío Bitcrusher
        int32_t BITCRUSHER_SEND_R = 0;        
        // 16 Tracks
        int16_t sample;
        for (byte f = 0; f < 16; f++) {
          if (ROTvalue[f][16]==1) { //////////////////////////////////////////////////////////////////////////////////////////////////// synth
          
            if (PITCH[f] != 255) {

                EPCW[f] += EFTW[f];
                uint16_t envelope_index = (uint16_t)(EPCW[f] >> 24);

                if (envelope_index >= 255) {
                    AMP[f] = 0;
                    PITCH[f] = 255;
                } else {
                    AMP[f] = wenvs[envs[f]][envelope_index];
                }
                    
                float current_freq_hz;
                float base_pitch = (float)PITCH[f];
                float mod_shift = (float)MOD[f] - 64.0f;
                float modulation_amount = (float)(EPCW[f] >> 24) / 255.0f;
                
                // Base pitch con modulación de envolvente
                float interpolated_pitch = base_pitch + mod_shift * modulation_amount;

                float detune_amount = (float)((int8_t)detune[f]); 
                
                float detune_factor = 0.02f; 
                
                interpolated_pitch += (detune_amount * detune_factor);

                if (interpolated_pitch < 0.0f) interpolated_pitch = 0.0f;
                if (interpolated_pitch > 127.0f) interpolated_pitch = 127.0f;

                int low_pitch_index = (int)interpolated_pitch;
                int high_pitch_index = low_pitch_index + 1;
                float fractional_part = interpolated_pitch - (float)low_pitch_index;


                float low_ftw = PITCH_TABLE_FTW[low_pitch_index];
                float high_ftw = PITCH_TABLE_FTW[high_pitch_index];
                float current_ftw_f = low_ftw + (high_ftw - low_ftw) * fractional_part;
                FTW[f] = (uint32_t)current_ftw_f;
                PCW[f] += FTW[f]; 

                uint16_t sindex = (uint16_t)(PCW[f] >> 22);
                sample = ((int32_t)wtables[wavs[f]][sindex] * AMP[f]) >> 10;

            } else {
                sample = 0;
            }

              int32_t sss = FILTROS[f].next(sample);
              int32_t sampleL = (sss * VOL_L[f] * 255) >> 16;
              int32_t sampleR = (sss * VOL_R[f] * 255) >> 16;

              DRUMTOTAL_L += sampleL;
              DRUMTOTAL_R += sampleR;

              if ((reverbs >> f) & 1) {
                REVERB_SEND_L += sampleL;
                REVERB_SEND_R += sampleR;
              }
              if ((delays >> f) & 1) {
                DELAY_SEND_L += sampleL;
                DELAY_SEND_R += sampleR;
              }
              if ((choruss >> f) & 1) {
                CHORUS_SEND_L += sampleL;
                CHORUS_SEND_R += sampleR;
              }
              if ((flangers >> f) & 1) {
                FLANGER_SEND_L += sampleL;
                FLANGER_SEND_R += sampleR;
              }
              if ((tremolos >> f) & 1) {
                TREMOLO_SEND_L += sampleL;
                TREMOLO_SEND_R += sampleR;
              }
              if ((ringmods >> f) & 1) {
                RINGMOD_SEND_L += sampleL;
                RINGMOD_SEND_R += sampleR;
              }
              if ((distortions >> f) & 1) {
                DISTORTION_SEND_L += sampleL;
                DISTORTION_SEND_R += sampleR;
              }
              if ((bitcrushers >> f) & 1) {
                BITCRUSHER_SEND_L += sampleL;
                BITCRUSHER_SEND_R += sampleR;
              }

          } else { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////// SAMPLER

            if (latch[f]) {
              int16_t sample;
              uint64_t index = samplePos[f] >> 16;
              // uint32_t ini = NEWINIS[ROTvalue[f][0]];
              // uint32_t end = NEWENDS[ROTvalue[f][0]];
              uint32_t ini = NEWINIS[f]; 
              uint32_t end = NEWENDS[f];              

              if (ROTvalue[f][8] == 1) { 
                // Reverse logic
                if (index <= ini || index <= 0) {
                  latch[f] = 0; samplePos[f] = ini; stepSize[f] = 0; continue;
                } else { sample = SAMPLES[ROTvalue[f][0]][index]; samplePos[f] -= stepSize[f]; }
              } else { 
                // Normal logic
                if (index >= end || index >= ENDS[ROTvalue[f][0]]) {
                  latch[f] = 0; samplePos[f] = 0; stepSize[f] = 0; continue; 
                } else { sample = SAMPLES[ROTvalue[f][0]][index]; samplePos[f] += stepSize[f]; }
              }
              // ADSR
              uint32_t relPos = index - ini;
              uint32_t total = end - ini;
              uint8_t a = ROTvalue[f][2]; uint8_t d = ROTvalue[f][3]; uint8_t s = ROTvalue[f][4]; uint8_t r = ROTvalue[f][5];
              uint16_t ad = a + d; uint16_t ads = ad + r;
              uint8_t env = 255;
              if (total > 0) {
                uint8_t phase = (relPos * 255) / total;
                if (phase < a) env = (phase * 255) / max((uint8_t)1, a);
                else if (phase < ad) { uint8_t dp = phase - a; env = 255 - (dp * (255 - s)) / max((uint8_t)1, d); }
                else if (phase < ads) { uint8_t rp = phase - ad; env = s - (rp * s) / max((uint8_t)1, r); }
                else env = 0;
              }

              int32_t sss = FILTROS[f].next(sample);
              int32_t sampleL = (sss * VOL_L[f] * env) >> 16;
              int32_t sampleR = (sss * VOL_R[f] * env) >> 16;

              DRUMTOTAL_L += sampleL;
              DRUMTOTAL_R += sampleR;

              if ((reverbs >> f) & 1) {
                REVERB_SEND_L += sampleL;
                REVERB_SEND_R += sampleR;
              }
              if ((delays >> f) & 1) {
                DELAY_SEND_L += sampleL;
                DELAY_SEND_R += sampleR;
              }
              if ((choruss >> f) & 1) {
                CHORUS_SEND_L += sampleL;
                CHORUS_SEND_R += sampleR;
              }
              if ((flangers >> f) & 1) {
                FLANGER_SEND_L += sampleL;
                FLANGER_SEND_R += sampleR;
              }
              if ((tremolos >> f) & 1) {
                TREMOLO_SEND_L += sampleL;
                TREMOLO_SEND_R += sampleR;
              }
              if ((ringmods >> f) & 1) {
                RINGMOD_SEND_L += sampleL;
                RINGMOD_SEND_R += sampleR;
              }
              if ((distortions >> f) & 1) {
                DISTORTION_SEND_L += sampleL;
                DISTORTION_SEND_R += sampleR;
              }
              if ((bitcrushers >> f) & 1) {
                BITCRUSHER_SEND_L += sampleL;
                BITCRUSHER_SEND_R += sampleR;
              }
            }
          }
        }
        // DRY
        DRUMTOTAL_L = soft_clip(DRUMTOTAL_L);
        DRUMTOTAL_R = soft_clip(DRUMTOTAL_R);
        
        int32_t dryL = FILTROS[16].next(DRUMTOTAL_L);
        int32_t dryR = FILTROS[17].next(DRUMTOTAL_R);
        
        // dryL = (dryL * mvol) >> 8;
        // dryR = (dryR * mvol) >> 8;

        // REVERB
        int32_t wetReverbL = 0, wetReverbR = 0;
        if (is_reverb) {
            int16_t rvInL = soft_clip(REVERB_SEND_L);
            int16_t rvInR = soft_clip(REVERB_SEND_R);
            myReverb.process(rvInL, rvInR, wetReverbL, wetReverbR);
            // APLICAR NIVEL (0-255)
            wetReverbL = ((int64_t)wetReverbL * level_reverb) >> 8;
            wetReverbR = ((int64_t)wetReverbR * level_reverb) >> 8;
        }

        // DELAY
        int32_t wetDelayL = 0, wetDelayR = 0;
        if (is_delay) {
            int16_t dlInL = soft_clip(DELAY_SEND_L);
            int16_t dlInR = soft_clip(DELAY_SEND_R);
            myDelay.process(dlInL, dlInR, wetDelayL, wetDelayR);
            wetDelayL = ((int64_t)wetDelayL * level_delay) >> 8;
            wetDelayR = ((int64_t)wetDelayR * level_delay) >> 8;
        }

        // CHORUS
        int32_t wetChorusL = 0, wetChorusR = 0;
        if (is_chorus) {
            int16_t chInL = soft_clip(CHORUS_SEND_L);
            int16_t chInR = soft_clip(CHORUS_SEND_R);
            myChorus.process(chInL, chInR, wetChorusL, wetChorusR);
            wetChorusL = ((int64_t)wetChorusL * level_chorus) >> 8;
            wetChorusR = ((int64_t)wetChorusR * level_chorus) >> 8;
        }

        // FLANGER
        int32_t wetFlangerL = 0, wetFlangerR = 0;
        if (is_flanger) {
            int16_t flInL = soft_clip(FLANGER_SEND_L);
            int16_t flInR = soft_clip(FLANGER_SEND_R);
            myFlanger.process(flInL, flInR, wetFlangerL, wetFlangerR);
            wetFlangerL = ((int64_t)wetFlangerL * level_flanger) >> 8;
            wetFlangerR = ((int64_t)wetFlangerR * level_flanger) >> 8;
        }

        // TREMOLO
        int32_t wetTremoloL = 0, wetTremoloR = 0;
        if (is_tremolo) {
            int16_t trInL = soft_clip(TREMOLO_SEND_L);
            int16_t trInR = soft_clip(TREMOLO_SEND_R);
            myTremolo.process(trInL, trInR, wetTremoloL, wetTremoloR);
            wetTremoloL = ((int64_t)wetTremoloL * level_tremolo) >> 8;
            wetTremoloR = ((int64_t)wetTremoloR * level_tremolo) >> 8;
        }

        // RING MOD
        int32_t wetRingmodL = 0, wetRingmodR = 0;
        if (is_ringmod) {
            int16_t rmInL = soft_clip(RINGMOD_SEND_L);
            int16_t rmInR = soft_clip(RINGMOD_SEND_R);
            myRingMod.process(rmInL, rmInR, wetRingmodL, wetRingmodR);
            wetRingmodL = ((int64_t)wetRingmodL * level_ringmod) >> 8;
            wetRingmodR = ((int64_t)wetRingmodR * level_ringmod) >> 8;
        }

        // DISTORTIOM
        int32_t wetDistortionL = 0, wetDistortionR = 0;
        if (is_distortion) {
            int16_t dtInL = soft_clip(DISTORTION_SEND_L);
            int16_t dtInR = soft_clip(DISTORTION_SEND_R);
            myDistortion.process(dtInL, dtInR, wetDistortionL, wetDistortionR);
            wetDistortionL = ((int64_t)wetDistortionL * level_distortion) >> 8;
            wetDistortionR = ((int64_t)wetDistortionR * level_distortion) >> 8;
        }

        // BITCRUSHER
        int32_t wetBitcrusherL = 0, wetBitcrusherR = 0;
        if (is_bitcrusher) {
            int16_t btInL = soft_clip(BITCRUSHER_SEND_L);
            int16_t btInR = soft_clip(BITCRUSHER_SEND_R);
            myBitcrusher.process(btInL, btInR, wetBitcrusherL, wetBitcrusherR);
            wetBitcrusherL = ((int64_t)wetBitcrusherL * level_bitcrusher) >> 8;
            wetBitcrusherR = ((int64_t)wetBitcrusherR * level_bitcrusher) >> 8;
        }

        // MIX
        // Sum: Dry + Reverb + Delay + Chorus + Flanger + Tremolo + Ringmod + Distortion + Bitcrusher
        int32_t finalMixL = dryL + wetReverbL + wetDelayL + wetChorusL + wetFlangerL + wetTremoloL + wetRingmodL + wetDistortionL + wetBitcrusherL;
        int32_t finalMixR = dryR + wetReverbR + wetDelayR + wetChorusR + wetFlangerR + wetTremoloR + wetRingmodR + wetDistortionR + wetBitcrusherR;

        finalMixL = (finalMixL * mvol) >> 8;
        finalMixR = (finalMixR * mvol) >> 8;

        finalMixL = soft_clip(finalMixL);
        finalMixR = soft_clip(finalMixR);

        // // --- VISUALIZER ---
        // int32_t mono = (finalMixL + finalMixR) / 2;
        // fastBuffer[fastIdx] = (int16_t)mono;
        // fastIdx++;
        // if (fastIdx >= SCOPE_W) fastIdx = 0;
        // static int waveSkip = 0;
        // if (++waveSkip >= 250) { 
        //     waveSkip = 0;
        //     memmove(waveBuffer, &waveBuffer[1], (SCOPE_W - 1) * sizeof(int16_t));
        //     waveBuffer[SCOPE_W - 1] = (int16_t)mono;
        // }

        out_buf[i * 2]     = (int16_t)finalMixL;
        out_buf[i * 2 + 1] = (int16_t)finalMixR;
    }

    bool enviado = false;
    while (!enviado) {
        enviado = M5.Speaker.playRaw(out_buf, DMA_BUF_LEN * 2, SAMPLE_RATE, true, 1, 0); 
        if (!enviado) vTaskDelay(1);
    }
}


////////////////////////////////////////////////////////////////////////////////

static void audio_task(void *userData){
  while(1) {
      write_buffer();      
  }
} 

//*********************************************************************
//  Setup ADSR 
//*********************************************************************

void initADSR(){
  for (int f = 0; f < 16; f++) {
    // attackTime[f]=0;
    // decayTime[f]=255;
    // sustainLevel[f]=255;
    // releaseTime[f]=255;
      ROTvalue[f][2]=0;  
      ROTvalue[f][3]=255;  
      ROTvalue[f][4]=255;  
      ROTvalue[f][5]=255;  
  }

}
// //*********************************************************************
// //  Setup initial 
// //*********************************************************************

// void synthESP32_setIni(unsigned char voice, int ini) {
//   NEWINIS[ROTvalue[voice][0]]=(ini * ENDS[ROTvalue[voice][0]]) >>11; // map de 0-2048 a los valores del sample
// }

// //*********************************************************************
// //  Setup end 
// //*******************************************************************

// void synthESP32_setEnd(unsigned char voice, int end) {
//   NEWENDS[ROTvalue[voice][0]]=(end * ENDS[ROTvalue[voice][0]]) >>11; // map de 0-2048 a los valores del sample
// }
void synthESP32_setIni(unsigned char voice, int ini) {
  NEWINIS[voice] = (ini * ENDS[ROTvalue[voice][0]]) >> 11; 
}

void synthESP32_setEnd(unsigned char voice, int end) {
  NEWENDS[voice] = (end * ENDS[ROTvalue[voice][0]]) >> 11;
}

//*********************************************************************
//  Setup master vol [0-127]
//*********************************************************************

void synthESP32_setMVol(unsigned char vol) {
  mvol=vol;	
}



//*********************************************************************
//  Setup master filter [0-255] parameter 0-127
//*********************************************************************

void synthESP32_setMFilter(unsigned char freq)  {
  // ya que 0 es no filter hago un map y cambio el rango
  freq=map(freq,0,127,255,0);
  FILTROS[16].setCutoffFreq(freq);
  FILTROS[17].setCutoffFreq(freq);
}  

//*********************************************************************
//  Setup voice filter [0-255] parameter 0-127
//*********************************************************************

void synthESP32_setFilter(unsigned char voice, unsigned char freq)  {
  // ya que 0 es no filter hago un map y cambio el rango
  freq=map(freq,0,127,255,0);
  FILTROS[voice].setCutoffFreq(freq);
} 

//*********************************************************************
//  Setup vol [0-127]
//*********************************************************************

void synthESP32_setVol(unsigned char voice,unsigned char vol) {
  synthESP32_updateVolPan(voice);
}

//*********************************************************************
//  Setup pan [-127 - 127%]
//*********************************************************************

void synthESP32_setPan(unsigned char voice,signed char pan) {
  synthESP32_updateVolPan(voice);
}
//*********************************************************************
//  Update vol and pan
//*********************************************************************

void synthESP32_updateVolPan(unsigned char voice) {
  int vol = ROTvalue[voice][14];
  int pan = ROTvalue[voice][13];

  // PAN va de -127 (izquierda) a +127 (derecha)
  int panL = 128 - pan; // más PAN a la derecha, menos volumen a la izquierda
  int panR = 128 + pan; // más PAN a la izquierda, menos volumen a la derecha

  // Normalizamos el volumen de cada canal
  VOL_L[voice] = (vol * constrain(panL, 0, 128)) >> 7;
  VOL_R[voice] = (vol * constrain(panR, 0, 128)) >> 7;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////


portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

////////////////////////////////////////////////////////////////////////////////////////////////////////

void synthESP32_TRIGGER(int nkey) {

  if (nkey < 0 || nkey > 15) return; 
  
  int ppitch=ROTvalue[nkey][12];

  for (int osc = 0; osc < addnextsnd[nkey]; osc++) {
    // Uso de bitwise AND para el wrap-around (solo funciona si el total es 16)
    int wkey = (nkey + osc) & 15; 

    // --- MODO SINTETIZADOR ---
    if (ROTvalue[wkey][16] == 1) {
      AMP[wkey] = 255; 
      PITCH[wkey] = ppitch;
      EPCW[wkey] = 0; // Reset de fase/envolvente
    } 
    // --- MODO SAMPLER ---
    else {
      int sampleIdx = ROTvalue[wkey][0];
      bool isReverse = (ROTvalue[wkey][8] == 1);

      // // Reinicio de posición

      // if (isReverse) {
      //   samplePos[wkey] = (uint64_t)NEWENDS[sampleIdx] << 16;
      // } else {
      //   samplePos[wkey] = (uint64_t)NEWINIS[sampleIdx] << 16;
      // }

      if (isReverse) {
        samplePos[wkey] = (uint64_t)NEWENDS[wkey] << 16;
      } else {
        samplePos[wkey] = (uint64_t)NEWINIS[wkey] << 16;
      }

      // Cálculo de velocidad de reproducción (Step Size)
      // Optimizamos: (frecuencia_objetivo * constante_precalculada)
      float speedRatio = midiFrequencies[ppitch] * BASE_FREQ_INV;
      stepSize[wkey] = (uint64_t)(speedRatio * 65536.0f);
      
      latch[wkey] = 1;
    }
  }
}

void synthESP32_TRIGGER_P(int nkey, int ppitch) {
  if (nkey < 0 || nkey > 15) return; 

  for (int osc = 0; osc < addnextsnd[nkey]; osc++) {
    int wkey = (nkey + osc) & 15; 

    // --- MODO SINTETIZADOR ---
    if (ROTvalue[wkey][16] == 1) {
      AMP[wkey] = 255; 
      PITCH[wkey] = ppitch;
      EPCW[wkey] = 0; // Reset de fase/envolvente
    } 
    // --- MODO SAMPLER ---
    else {
      int sampleIdx = ROTvalue[wkey][0];
      bool isReverse = (ROTvalue[wkey][8] == 1);

      // Reinicio de posición con casting limpio

      // if (isReverse) {
      //   samplePos[wkey] = (uint64_t)NEWENDS[sampleIdx] << 16;
      // } else {
      //   samplePos[wkey] = (uint64_t)NEWINIS[sampleIdx] << 16;
      // }

      if (isReverse) {
        samplePos[wkey] = (uint64_t)NEWENDS[wkey] << 16;
      } else {
        samplePos[wkey] = (uint64_t)NEWINIS[wkey] << 16;
      }      

      // Cálculo de velocidad de reproducción (Step Size)
      // Optimizamos: (frecuencia_objetivo * constante_precalculada)
      float speedRatio = midiFrequencies[ppitch] * BASE_FREQ_INV;
      stepSize[wkey] = (uint64_t)(speedRatio * 65536.0f);
      
      latch[wkey] = 1;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////
void setSoundALL(){
  for (byte f = 0; f < 16; f++) {
    setSound(f);
  }
}
void setSound(byte f) {
  //if (xSemaphoreTake(audioMutex, portMAX_DELAY) == pdTRUE) {
    if (ROTvalue[f][16]) {
      // synth engine
      synthESP32_setWave(f,ROTvalue[f][1]);  
      synthESP32_setEnvelope(f,ROTvalue[f][9]);
      synthESP32_setLength(f,ROTvalue[f][10]);
      // cambio pitch solo si no es melodic
      if (!isMelodic[f])  synthESP32_setPitch(f,ROTvalue[f][12]);

      synthESP32_setMod(f,ROTvalue[f][11]);

      //AMP[f] = 255; 
      //PITCH[f] = 255;
      //EPCW[f] = 0; // Reset de fase/envolvente


    } else {
      // Modifican NEWINIS y NEWENDS, que son usados por write_buffer()
     synthESP32_setIni(f, ROTvalue[f][6]);  
     synthESP32_setEnd(f, ROTvalue[f][7]);
    }

    // Modifican VOL_L[] y VOL_R[]
    synthESP32_updateVolPan(f);  

    // Modifica FILTROS[f]
    synthESP32_setFilter(f, ROTvalue[f][15]);

    //xSemaphoreGive(audioMutex);
  //}

}
void setRandomVoice2(byte f){
  // ROTvalue[f][0]=random(min_values[0], max_values[0]);
  // ROTvalue[f][3]=random(min_values[3], max_values[3]);
  // ROTvalue[f][6]=random(min_values[6], max_values[6]);
}

void setRandomVoice(byte f){

  if (ROTvalue[f][16]){
    ROTvalue[f][1]=random(min_values[1], max_values[1]);
    ROTvalue[f][9]=random(min_values[9], max_values[9]);
    ROTvalue[f][10]=random(min_values[10], max_values[10]);
    ROTvalue[f][12]=random(min_values[12]+10, max_values[12]-80);
     byte domod=random(0,2);
    int rmod=0;
    if (domod) rmod=random(-10,10);
     ROTvalue[f][11]=64+rmod;
  } else {
    ROTvalue[f][0]=random(min_values[0], samplesTotal);
  }
  ROTvalue[f][13]=random(min_values[13], max_values[13]);

}

void setRandomPattern(byte f){
  byte veces=random(0,2);
  for (byte b = 0; b < 16; b++) {
    byte mybit=random(0,2);
    if (veces) {
      if (mybit) mybit=random(0,2); // Si es 1 hago otro random para que haya menos unos
    }
    bitWrite(pattern[f],b,mybit);
  } 
  setRandomPitch(f);
}

bool find_scale(uint8_t note){

  uint8_t candidate=note % 12;
  // buscar en array de escalas
  bool in_scale=false;
  for (byte s = 0; s < 8; s++) {
    if (note==escalas[selected_scale][s]){
      in_scale=true;
      break;
    }
  }
  if (in_scale){
    return true;
  }
  return false;
}

void setRandomPitch(uint8_t ssound) {
  // Root NOTE
  uint8_t baseNote = ROTvalue[ssound][12];

  // (ignore note 255)
  uint8_t numNotas = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (escalas[selected_scale][i] == 255) {
      break;
    }
    numNotas++;
  }

  if (numNotas == 0) return; 

  // fill array
  for (uint8_t i = 0; i < 16; i++) {
    
    uint8_t randomIndex = random(0, numNotas);
    uint8_t intervalo = escalas[selected_scale][randomIndex];

    int8_t octaveShift = (random(0, 3) - 1) * 12;

    int16_t finalNote = baseNote + intervalo + octaveShift;

    if (finalNote < 0) finalNote = 0;
    if (finalNote > 127) finalNote = 127;

    melodic[ssound][i] = (uint8_t)finalNote;
  }
}


void setRandomNotes(byte f){
  setRandomPitch(f);
} 


//*********************************************************************
//  Setup wave [0-16]
//*********************************************************************

void synthESP32_setWave(unsigned char voice, unsigned char wave){
  wavs[voice]=wave;
}

//*********************************************************************
//  Setup Envelope [0-3]
//*********************************************************************

void synthESP32_setEnvelope(unsigned char voice, unsigned char env) {
  envs[voice]=env;
}

//*********************************************************************
//  Setup Length [0-127]
//*********************************************************************

void synthESP32_setLength(unsigned char voice,unsigned char length) {
    // Convierte el valor de 0-127 a un rango de 0 a x segundos.
    float duration_seconds = (float)length / 127.0f * 5.0f; 
    uint32_t samples_per_note = (uint32_t)(duration_seconds * SAMPLE_RATE);

    if (samples_per_note > 0) {
        EFTW[voice] = (1ULL << 32) / samples_per_note;
    } else {
        EFTW[voice] = 0; 
    }
}

//*********************************************************************
//  Setup Pitch [0-127]
//*********************************************************************

void synthESP32_setPitch(unsigned char voice,unsigned char MIDInote) {
  PITCH[voice]=MIDInote; //PITCH_FREQS_HZ[MIDInote];
}

//*********************************************************************
//  Setup mod [0-127][64 no mod]
//*********************************************************************

void synthESP32_setMod(unsigned char voice,unsigned char mod) {
  MOD[voice]=(int)mod;
}

//*********************************************************************
//  Move pattern left right -1, +1
//*********************************************************************

void move_pattern(int ssound, int dir) {
    int i;
    
    // PATTERN
    uint16_t val = (uint16_t)pattern[ssound];
    if (dir == 1) {
        pattern[ssound] = (val << 1) | (val >> 15);
    } 
    else if (dir == -1) {
        pattern[ssound] = (val >> 1) | (val << 15);
    }


    // MELODIC (Rotación de ARRAY)
    if (dir == -1) {
        uint8_t temp_m = melodic[ssound][0];
        for (i = 0; i < 15; i++) {
            melodic[ssound][i] = melodic[ssound][i + 1];
        }
        melodic[ssound][15] = temp_m;
    } 
    else if (dir == 1) {
        uint8_t temp_m = melodic[ssound][15];
        for (i = 15; i > 0; i--) {
            melodic[ssound][i] = melodic[ssound][i - 1];
        }
        melodic[ssound][0] = temp_m;
    }
}