void mySEQ() {

  // FX1
  if (playing){
    if (!(tick % (12)) && fx1==1) {
      synthESP32_TRIGGER(selected_sound);
    }
    if (!(tick % (6)) && fx1==2) {
      synthESP32_TRIGGER(selected_sound);
    }
    if (!(tick % (3)) && fx1==3) {
      synthESP32_TRIGGER(selected_sound);
    }
  }
  
  // Limpiar marcas de sound y step
  if ((tick % (6))==4) clearPADSTEP=true;

  // Lanzar cambio de step
  if (!(tick % (6))) {  //tic();
    // leer step actual de cada canal
    for (int f = 0; f < 16; f++) { 
      if (!bitRead(mutes, f)) {
        if (solos == 0 || (solos > 0 && bitRead(solos, f))) {
          if (bitRead(pattern[f], sstep)) { // note on
            latch[f]=0;        
            if (isMelodic[f]){
              // Trigger con la nota almacenada
              synthESP32_TRIGGER_P(f,melodic[f][sstep]);
            } else {
              // Trigger con el pitch del canal
              synthESP32_TRIGGER(f);
            }
          } 
        }
      }
    }

    sstep++;

    // Comprobar step final
    if (sstep==(lastStep+1) || sstep==(newLastStep+1) || sstep==16) {
      if (songing){
        pattern_song_counter++;    
        // Comprobar step final
        if ((pattern_song_counter > lastPattern) || (pattern_song_counter > newLastPattern) || (pattern_song_counter == 16)) {
          lastPattern = newLastPattern;
          pattern_song_counter = firstPattern;
        }
        // song mode
        if (song_mode==0 || song_mode==1) {
          load_pattern(pattern_song_counter);
          selected_pattern = pattern_song_counter;
        }
        if (song_mode==0 || song_mode==2) {
          load_sound(pattern_song_counter);
          selected_sndSet = pattern_song_counter;
          setSoundALL();
        }

        refreshPADSTEP = true;
        refreshSEQ = true;
        refresh_sound_bars=true;

      }
      lastStep=newLastStep;
      sstep=firstStep;
    }
    refreshPADSTEP=true;

  } // tic

  tick++;
  if (tick >= 96) tick = 0; 
}




