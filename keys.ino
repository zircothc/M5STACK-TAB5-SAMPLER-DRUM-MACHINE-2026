
void DO_KEYPAD(){

  for (int f=0;f<MAX_BUTTONS;f++) {
    if (mBoton[f]->trigger_on==1){
      mBoton[f]->trigger_on=0;

      nkey=f;

      show_last_touched=true;
      start_showlt=start_debounce;

      if (nkey<16){
         
        switch (modeZ) {

          case tPiano: // 16 keys=16 notes
            synthESP32_TRIGGER_P(selected_sound,nkey+(12*octave));
            if (recording){
              bitWrite(pattern[selected_sound],sstep,1);
              melodic[selected_sound][sstep]=nkey+(12*octave);
            }
            //refreshPATTERN=true;
            break;          
          case tPad: // play pads

            synthESP32_TRIGGER(nkey);


            if (recording){
              bitWrite(pattern[nkey],sstep,1); //!bitRead(pattern[nkey],sstep));
              melodic[selected_sound][nkey]=ROTvalue[selected_sound][12];
            } else {

              synthESP32_TRIGGER(nkey);
            
            }
            //if (!shiftR1 && !shifting){
              selected_sound=nkey;
              if (selected_sound!=oldselected_sound){          
                oldselected_sound=selected_sound;
                refreshSEQ=true;
                //setSound(nkey);
                select_rot();
                refresh_sound_bars=true;
              }
            //}
            //refreshPATTERN=true;
            refreshSEQ=true;
            break;          
          case tSel:  // Select sound
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){   
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              refresh_sound_bars=true; 
              //setSound(nkey);
              select_rot();
            }
            // modeZ=tWrite;
              refreshPATTERN=true;
            // draw_sound_bars2();
            // refreshMODES=true;
            break;
          case tWrite:
            // bitWrite(pattern[selected_sound],nkey,!bitRead(pattern[selected_sound],nkey));
            // melodic[selected_sound][nkey]=ROTvalue[selected_sound][12];
            // refreshPATTERN=true;
            break;
          case tMelod:
            isMelodic[nkey]=!isMelodic[nkey];
            ROTvalue[selected_sound][19]=isMelodic[nkey];
            refreshPATTERN=true; 
            refreshSEQ=true;
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              select_rot();
              refresh_sound_bars=true;
            }
            //selected_sound=nkey;
            break;

          case tMute:
            bitWrite(mutes,nkey,!bitRead(mutes,nkey));
            refreshPATTERN=true;
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }            
            break;
          case tSolo:
            bitWrite(solos,nkey,!bitRead(solos,nkey));
            refreshPATTERN=true;
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;
          case tClear: // clear
            pattern[nkey]=0;
            refreshSEQ=true;
            break;
          case tLoadP:
            load_pattern(nkey);
            pattern_song_counter=nkey;
            selected_pattern=nkey;
            refreshSEQ=true;
            break;
          case tSaveP:
            save_pattern(nkey);
            selected_pattern=nkey;
            modeZ=tPad; // pongo el modeZ en 0 para que no se quede en grabar
            refreshMODES=true;
            break;
          case tLoadS:
            load_sound(nkey);
            selected_sndSet=pattern_song_counter;
            selected_sndSet=nkey;
            setSoundALL();
            select_rot();
            refresh_sound_bars=true;
            break;
          case tSaveS:
            save_sound(nkey);
            selected_sndSet=nkey;
            modeZ=tPad; // pongo el modeZ en 0 para que no se quede en grabar
            refreshMODES=true;
            break;
          case tLoaPS:
            load_pattern(nkey);
            pattern_song_counter=nkey;
            selected_pattern=nkey;
            load_sound(nkey);
            selected_sndSet=pattern_song_counter;
            selected_sndSet=nkey;
            refreshSEQ=true;
            setSoundALL();
            select_rot();
            refresh_sound_bars=true;
            break;
          case tSavPS:
            save_pattern(nkey);
            selected_pattern=nkey;
            save_sound(nkey);
            selected_sndSet=nkey;                        
            modeZ=tPad; // pongo el modeZ en 0 para que no se quede en grabar
            refreshMODES=true;
            break;
          case tRndS:
            selected_sound=nkey;
            setRandomVoice(nkey);
            //if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            //} 
            break;
          case tRndS2:
            selected_sound=nkey;
            setRandomVoice2(nkey);
            //if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            //} 
            break;  
          case tRndP:
            selected_sound=nkey;
            setRandomPattern(nkey);
            //if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            //} 
            break;
          case tRndNo:
            selected_sound=nkey;
            setRandomNotes(nkey);
            //if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            //} 
            break;
          case tFirst:
            refreshPATTERN=true;
            firstStep=nkey;
            break;          
          case tLast:
            refreshPATTERN=true;
            newLastStep=nkey;
            break;  
          case tMemo: // memory
            selected_memory=nkey;
            load_memory();
            break;         
          case tPfirs:
            refreshPATTERN=true;
            firstPattern=nkey;
            break;          
          case tPlast:
            refreshPATTERN=true;
            newLastPattern=nkey;
            break; 
          case tReverb:
            bitWrite(reverbs,nkey,!bitRead(reverbs,nkey));
            refreshPATTERN=true;
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;            
          case tDelay:
            bitWrite(delays,nkey,!bitRead(delays,nkey));
            refreshPATTERN=true;
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break; 
          case tChorus:
            bitWrite(choruss,nkey,!bitRead(choruss,nkey));
            refreshPATTERN=true;            
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;           
          case tFlanger:
            bitWrite(flangers,nkey,!bitRead(flangers,nkey));
            refreshPATTERN=true;            
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break; 
          case tTremolo:
            bitWrite(tremolos,nkey,!bitRead(tremolos,nkey));
            refreshPATTERN=true;            
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;
          case tRingmod:
            bitWrite(ringmods,nkey,!bitRead(ringmods,nkey));
            refreshPATTERN=true;            
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;
          case tDistortion:
            bitWrite(distortions,nkey,!bitRead(distortions,nkey));
            refreshPATTERN=true;            
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;
          case tBitcrusher:
            bitWrite(bitcrushers,nkey,!bitRead(bitcrushers,nkey));
            refreshPATTERN=true;            
            selected_sound=nkey;
            if (selected_sound!=oldselected_sound){          
              oldselected_sound=selected_sound;
              refreshSEQ=true;
              //setSound(nkey);
              select_rot();
              refresh_sound_bars=true;
            }                         
            break;             
        }
        //refreshPADTOUCHED=true;

      //// func keys
      } else if (nkey<38 || nkey>42){

        refreshMODES=true;
        
        if (!shiftR1 && !shifting){

          switch (nkey) {

            // playpad/select
            case 16:
              if (modeZ==tPad){
                modeZ=tSel;
              } else {
                modeZ=tPad;
              }
              clearPATTERNPADS=true;
              //refreshPATTERN=true;  
              break;
            // melodic     
            case 17:
              modeZ=tMelod;
              //clearPATTERNPADS=true;
              refreshPATTERN=true;
              break;
            // random pattern     
            case 18:
              modeZ=tRndP;
              clearPATTERNPADS=true;
              break;
            // load       
            case 19:
              if (modeZ==tLoaPS) {
                modeZ=tLoadP;
              } else if (modeZ==tLoadP ) {
                modeZ=tLoadS;
              } else {
                modeZ=tLoaPS;
              }
              clearPATTERNPADS=true;
              break;              
            // save      
            case 20:
              if (modeZ==tSavPS) {
                modeZ=tSaveP;
              } else if (modeZ==tSaveP ) {
                modeZ=tSaveS;
              } else {
                modeZ=tSavPS;
              }
              clearPATTERNPADS=true;       
              break; 
            // piano & clear
            case 21:
              if (modeZ==tPiano) {
                modeZ=tClear;
              } else {
                modeZ=tPiano;
              }
              clearPATTERNPADS=true;
              //refreshPATTERN=true;  
              break;
            // play
            case 22:
              if (playing){
                seq.stop();
                sstep=firstStep;
                recording=false;
                clearPADSTEP=true;
                pattern_song_counter=0; 
              } else {
                if (sync_state==2){ // if this machine is slave dont start playing now
                  pre_playing=true;
                } else {
                if (songing) pattern_song_counter=selected_pattern;
                  tick = 0;
                  seq.start();
                  sstep=firstStep;
                  refreshPADSTEP=true;  
                }
              }
              playing=!playing;            
              break;
            // song
            case 23:
              songing=!songing;
              if (recording) songing=false; // if recording I cant set song mode
              break;
            // mute
            case 24:
              modeZ=tMute;
              refreshPATTERN=true;  
              break;
            // solo
            case 25:
              modeZ=tSolo;
              refreshPATTERN=true;  
              break;
            // first step, last step, first Pattern, last Pattern
            case 26:
              if (modeZ==tFirst) {
                modeZ=tLast;
              } else if (modeZ==tLast) {
                modeZ=tPfirs;
              } else if (modeZ==tPfirs) {
                 modeZ=tPlast;
              } else {
                modeZ=tFirst;
              }
              refreshPATTERN=true;
              break;
            case 27:
              if (old_vol!=0) {
                master_vol=old_vol;
                old_vol=0;
              } else {
                old_vol=master_vol;
                master_vol=0;
              }
              synthESP32_setMVol(master_vol);
              drawBar(24);
              break;
            case 28:
              rPage=0;
              s_old_selected_rot=99; // para que vuelva a marcar el rot seleccionado
              refresh_rPage=true;
              break;
            case 29:
              rPage=1;
              s_old_selected_rot=99;
              refresh_rPage=true;
              break;
            case 30:
              rPage=2;
              s_old_selected_rot=99;
              refresh_rPage=true;
              break;  
            // memory
            case 31:
              modeZ=tMemo;
              refreshPATTERN=true;  
              break;              
            // rnd sound
            case 32:
              modeZ=tRndS;
              clearPATTERNPADS=true;  
              break;
            // Reverb
            case 33:
              modeZ=tReverb;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            // Delay
            case 34:
              modeZ=tDelay;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 35:
              modeZ=tChorus;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 36:
              modeZ=tFlanger;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 37:
              modeZ=tTremolo;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 43:
              modeZ=tRingmod;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 44:
              modeZ=tDistortion;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 45:
              modeZ=tBitcrusher;
              //clearPATTERNPADS=true;
              refreshPATTERN=true; 
              break;
            case 46:
              fx1=0;
              break;
            case 47:
              fx1=1;
              break;
            case 48:
              fx1=2;
              break;
            case 49:
              fx1=3;
              break;              
          }  

        // shift 
                                  
        } else {
          switch (nkey){
            case 16:
              modeZ=tSel;
              break;
            // case 32:
            //   modeZ=tRndS2;
            //   break;  
            // reset mutes & solos and firstStep=0 and lastStep=15       
            case 24:
              mutes=0;
              clean_mute_marks();
              clearPATTERNPADS=true;
              break;
            case 25:
              solos=0;
              clean_solo_marks(); 
              clearPATTERNPADS=true;
              break;
            case 26:
              firstStep=0;
              lastStep=15;
              clearPATTERNPADS=true;
              break;
            // Play
            case 22:
              if (playing){
                recording=!recording;
              } else {
                tick = 0;
                seq.start();
                recording=true;
                playing=true;
                sstep=firstStep;
                refreshPADSTEP=true;  
              }
              if (songing) recording=false; // in song mode cant save modified patterns. I would need a new flag. so much cpu time?
              //refreshMODES=true;  
              break;
          }         
        }
      } else {
        switch (nkey){
          case 38:
            old_counter1=counter1;
            if (shifting || shiftR1) {
              counter1=counter1+100;
            } else {
              counter1=counter1+10;
            }
            do_rot();
            break;
          case 39:
            old_counter1=counter1;
            if (shifting || shiftR1) {
              counter1=counter1+1;
            } else {
              counter1=counter1+1;
            }
            do_rot();
            break;
          case 40:
            old_counter1=counter1;
            if (shifting || shiftR1) {
              counter1=counter1-1;
            } else {
              counter1=counter1-1;
            }
            do_rot();
            break;
          case 41:
            old_counter1=counter1;
            if (shifting || shiftR1) {
              counter1=counter1-100;
            } else {
              counter1=counter1-10;
            }
            do_rot();
            break;
          case 42:
            shifting=!shifting;
            refresh_shift_key();
            break;                        
        }   

      } 
    }
  }


  //////////////////////////////////////////////////////////////////// bars
  for (int f=0;f<MAX_BARS;f++) {
    if (mRot[f]->trigger_on==1){
      mRot[f]->trigger_on=0;

      nkey=f;
      selected_rot=nkey;

      if (shifting || shiftR1) {
        if (selected_rot==1){
          ROTvalue[selected_sound][6]=min_values[6];
        }
        if (selected_rot==2){
          ROTvalue[selected_sound][7]=max_values[7];
        }
      }
      select_rot();
    }
  }


  //////////////////////////////////////////////////////////////////// seq buttons
  for (int f=0;f<16;f++) {
    if (mBseq[f]->trigger_on==1){
      mBseq[f]->trigger_on=0;
      bitWrite(pattern[selected_sound],f,!bitRead(pattern[selected_sound],f));
      melodic[selected_sound][f]=ROTvalue[selected_sound][12];
      refreshSEQ=true;

    }
  }

}
