

void select_rot(){
  switch (selected_rot) {
    case 17:
      zmpitch=0;
      counter1=zmpitch; 
      break;
    case 18:
      ztranspose=0;
      counter1=ztranspose;
      break;
    case 19:
      counter1=octave;      
      break;
    case 20:
      counter1=selected_scale;      
      break;
    case 21:
      counter1=sync_state;      
      break;
    case 22:
      counter1=bpm;      
      break;
    case 23:
      zmove=0;
      counter1=zmove;      
      break;
    case 24:
      counter1=master_vol;      
      break;
    case 25:
      counter1=master_filter;      
      break;
    case 26: // song mode, load pattern and sound, load pattern, load sound
      counter1=song_mode;      
      break;
    case 27:
      counter1=level_reverb;      
      break;
    case 28:
      counter1=level_delay;      
      break;
    case 29:
      counter1=level_chorus;      
      break;
    case 30:
      counter1=level_flanger;      
      break;
    case 31:
      counter1=level_tremolo;      
      break;
    case 32:
      counter1=reverb_type;      
      break;
    case 33:
      counter1=delay_time;      
      break;
    case 34:
      counter1=chorus_type;      
      break;
    case 35:
      counter1=flanger_type;      
      break;
    case 36:
      counter1=tremolo_type;      
      break;
    case 37:
      counter1=level_ringmod;      
      break;
    case 38:
      counter1=ringmod_type;      
      break;
    case 39:
      counter1=addnextsnd[selected_sound];      
      break;
    case 40:
      counter1=detune[selected_sound];
      break;
    case 41:
      counter1=level_distortion;      
      break;
    case 42:
      counter1=distortion_type;      
      break;
    case 43:
      counter1=level_bitcrusher;      
      break;
    case 44:
      counter1=bitcrusher_type;      
      break;      
    default:
      // selectable rot1 (wave table) shares position with sample
      if (selected_rot==0 && ROTvalue[selected_sound][16]==1) selected_rot=1;
      counter1=ROTvalue[selected_sound][selected_rot];
  }

}  


//////////////////////////////////////////////////////////////////////////////
void do_rot(){

  if (counter1>max_values[selected_rot]) counter1=max_values[selected_rot];
  if (counter1<min_values[selected_rot]) counter1=min_values[selected_rot];
 
  if (selected_rot==0 && counter1>samplesTotal) counter1=samplesTotal;

  if (counter1!=old_counter1){

    switch (selected_rot) {
      case 17:
        zmpitch=counter1;
        // cambiar pitch en melodic
        for (byte k=0; k<16;k++){  
          ROTvalue[k][12]=ROTvalue[k][12]+zmpitch;  
          for (byte f=0; f<16;f++){
            melodic[k][f]=melodic[k][f]+zmpitch;
          } 
        }
        refreshSEQ=true;
        zmpitch=0;
        counter1=zmpitch; 
        break;
      case 18:
        ztranspose=counter1;
        // cambiar pitch en melodic
        for (byte f=0; f<16;f++){
          melodic[selected_sound][f]=melodic[selected_sound][f]+ztranspose;
        }
        refreshSEQ=true;
        ztranspose=0;
        counter1=ztranspose;
        refreshSEQ=true;
        break;
      case 19:
        octave=counter1;      
        break;
      case 20:
        selected_scale=counter1;      
        break;
      case 21:
        sync_state=counter1;      
        break;
      case 22:
        bpm=counter1;
        seq.setBPM(bpm);    
        break;
      case 23:
        zmove=counter1;
        move_pattern(selected_sound,zmove);
        zmove=0;
        counter1=zmove;
        //refreshPATTERN=true;
        refreshSEQ=true;
        break;
      case 24:
        master_vol=counter1;
        synthESP32_setMVol(master_vol);     
        break;
      case 25:
        master_filter=counter1;
        synthESP32_setMFilter(master_filter);      
        break;
      case 26:
        song_mode=counter1;
        break;
      case 27:
        level_reverb=counter1;      
        break;
      case 28:
        level_delay=counter1;      
        break;
      case 29:
        level_chorus=counter1;      
        break;
      case 30:
        level_flanger=counter1;      
        break;
      case 31:
        level_tremolo=counter1;      
        break;
      case 32:
        reverb_type=counter1;
        myReverb.setPreset(reverb_type);      
        break;
      case 33:
        delay_time=counter1;
        myDelay.setTime(map(delay_time, 0, 255, 100, 88000));      
        break;
      case 34:
        chorus_type=counter1;
        myChorus.setPreset(chorus_type);      
        break;
      case 35:
        flanger_type=counter1;
        myFlanger.setPreset(flanger_type);       
        break;
      case 36:
        tremolo_type=counter1;
        myTremolo.setPreset(tremolo_type);       
        break;
      case 37:
        level_ringmod=counter1;
        break;        
      case 38:
        ringmod_type=counter1;
        myRingMod.setPreset(ringmod_type);
        break;
      case 39:
        addnextsnd[selected_sound]=counter1;
        ROTvalue[selected_sound][18]=counter1;
        break;
      case 40:
        detune[selected_sound]=counter1;
        ROTvalue[selected_sound][17]=counter1;
        break;
      case 41:
        level_distortion=counter1;
        break;
      case 42:
        distortion_type=counter1;
        myDistortion.setPreset(distortion_type);        
        break;
      case 43:
        level_bitcrusher=counter1;
        break;
      case 44:
        bitcrusher_type=counter1;
        myBitcrusher.setPreset(bitcrusher_type);         
        break;                        
      default:
        // selectable rot1 (wave table) shares position with sample
        if (selected_rot==0 && ROTvalue[selected_sound][16]==1) selected_rot=1;

        ROTvalue[selected_sound][selected_rot]=counter1;
        
        //setSound(selected_sound);
        int f=selected_sound;
        int sr=selected_rot;
        switch (sr){
          case 0:
            synthESP32_setIni(f, ROTvalue[f][6]);           
            synthESP32_setEnd(f, ROTvalue[f][7]);
            break;
          case 1:
            synthESP32_setWave(f,ROTvalue[f][1]);
            break;
          case 9:  
            synthESP32_setEnvelope(f,ROTvalue[f][9]);
            break;
          case 10:
            synthESP32_setLength(f,ROTvalue[f][10]);
            break;
          case 12:
            synthESP32_setPitch(f,ROTvalue[f][12]);
            refreshSEQ=true;
            break;
          case 11:
            synthESP32_setMod(f,ROTvalue[f][11]);
            break;
          // Modifican NEWINIS y NEWENDS, que son usados por write_buffer()
          case 6:
            synthESP32_setIni(f, ROTvalue[f][6]);  
            break;
          case 7:
            synthESP32_setEnd(f, ROTvalue[f][7]);
            break;
           // Modifican VOL_L[] y VOL_R[]
          case 13:
            synthESP32_updateVolPan(f);  
            break;
          case 14:
            synthESP32_updateVolPan(f);  
            break;
          // Modifica FILTROS[f]
          case 15:
            synthESP32_setFilter(f, ROTvalue[f][15]);
            break;
          case 16:
            setSound(selected_sound);
            refresh_sound_bars=true;
            break;
        }


    }


    // play sound if
    if (selected_rot<17 && !playing){
      synthESP32_TRIGGER(selected_sound);
    }

    drawBar(selected_rot);



  }  
}

