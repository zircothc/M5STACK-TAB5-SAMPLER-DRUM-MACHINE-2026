
void init_colors(){

//#define OSCURO M5.Display.color565(10, 35, 10)
//#define OSCURO M5.Display.color565(10, 20, 10)
#define OSCURO M5.Display.color565(25, 25, 20)
#define ZGREEN M5.Display.color565(50, 90, 50)

#define ZRED M5.Display.color565(222, 43, 27)
#define ZYELLOW M5.Display.color565(201, 191, 40)
#define ZMAGENTA M5.Display.color565(128, 92, 156)
#define ZCYAN M5.Display.color565(126, 225, 247)
#define ZBLUE M5.Display.color565(80, 110, 240)
#define ZGREENCLEAR M5.Display.color565(197, 240, 221)
#define ZGREENALTER M5.Display.color565(90, 232, 170)

#define DARKGREY M5.Display.color565(100, 100, 100)

#define PADCOLOR M5.Display.color565(100, 100, 50)
#define ZBLACK M5.Display.color565(1, 1, 1)


#define bRojo M5.Display.color565(230, 50, 41)
#define bNaranja M5.Display.color565(241, 142, 28)
#define bAmarillo M5.Display.color565(249, 208, 6)
#define bBlanco M5.Display.color565(217, 217, 217)


}


void showLastTouched() {
  if (!show_last_touched) {
    return;
  }
  // Serial.print("lt ");
  // Serial.println(last_touched);
  //M5.Display.drawRect(mBoton[last_touched]->x + 0, mBoton[last_touched]->y + 0, mBoton[last_touched]->w - 1, mBoton[last_touched]->h - 1, mBoton[last_touched]->color);
  M5.Display.drawRect(mBoton[last_touched]->x + 3, mBoton[last_touched]->y + 3, mBoton[last_touched]->w - 7, mBoton[last_touched]->h - 7, mBoton[last_touched]->color);
  M5.Display.drawRect(mBoton[last_touched]->x + 4, mBoton[last_touched]->y + 4, mBoton[last_touched]->w - 9, mBoton[last_touched]->h - 9, mBoton[last_touched]->color);
  show_last_touched = false;
  clear_last_touched = true;
}

void clearLastTouched() {
  if (!clear_last_touched) {
    return;
  }

  if (start_showlt + (debounce_time * 2) > millis()) {
    return;
  }
  //Serial.print("lt ");
  //Serial.println(last_touched);
  for (byte f = 0; f < MAX_BUTTONS; f++) {
    //if (f<33 || (f>32 && rPage==2) || (f>37 && f<43) || (f>42 && rPage==2)) {
    if (mBoton[f]->rPage==0 || mBoton[f]->rPage==rPage) {
      M5.Display.drawRect(mBoton[f]->x + 3, mBoton[f]->y + 3, mBoton[f]->w - 7, mBoton[f]->h - 7, BLACK);
      M5.Display.drawRect(mBoton[f]->x + 4, mBoton[f]->y + 4, mBoton[f]->w - 9, mBoton[f]->h - 9, BLACK);
    }
  }
  clear_last_touched = false;

}



//////// MUTE AND SOLO MARKS

void paint_mute_marks(uint8_t f,uint16_t color){
  int pos_x;
  int pos_y;  
    pos_x=mBoton[f]->x+15;
    pos_y=mBoton[f]->y+15;
    M5.Display.setCursor(pos_x,pos_y);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.print("M");      
}  
void paint_solo_marks(uint8_t f,uint16_t color){  
  int pos_x;
  int pos_y;  
    pos_x=mBoton[f]->x+35;
    pos_y=mBoton[f]->y+15;
    M5.Display.setCursor(pos_x,pos_y);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.print("S");       
}

void clean_mute_marks(){
  int pos_x;
  int pos_y;  
  // borrar todos los marcadores de mute
  for (int f=0;f<16;f++){      
    pos_x=mBoton[f]->x+15;
    pos_y=mBoton[f]->y+15;
    M5.Display.setCursor(pos_x,pos_y);
    M5.Display.setTextColor(OSCURO, BLACK);
    M5.Display.print("M");      
  }
}  
void clean_solo_marks(){
  int pos_x;
  int pos_y;
  // borrar todos los marcadores de solo
  for (int f=0;f<16;f++){      
    pos_x=mBoton[f]->x+35;
    pos_y=mBoton[f]->y+15;
    M5.Display.setCursor(pos_x,pos_y);
    M5.Display.setTextColor(OSCURO, BLACK);
    M5.Display.print("S");       
  }  
}
void show_dark_selectors_ONLY1(){  // only once
  int pos_x;
  int pos_y;

  // borrar todos los marcadores de selected_sound
  for (int f=0;f<16;f++){      
    pos_x=mBoton[f]->x+50;
    pos_y=mBoton[f]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);
  }

  // borrar todos los marcadores de selected_pattern 
  for (int f=0;f<16;f++){      
    pos_x=mBoton[f]->x+66;
    pos_y=mBoton[f]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);     
  }

  // borrar todos los marcadores de selected_sndSet
  for (int f=0;f<16;f++){      
    pos_x=mBoton[f]->x+82;
    pos_y=mBoton[f]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);          
  }  

  // borrar todos los marcadores de selected_memory
  for (int f=0;f<16;f++){      
    pos_x=mBoton[f]->x+98;
    pos_y=mBoton[f]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);       
  }

  clean_mute_marks();
  clean_solo_marks();

}


void show_all_bars(){   
  int pos_x;
  int pos_y;

  for (int f=1; f<MAX_BARS; f++){ 
    if (mRot[f]->rPage==rPage){ // si el controlador pertenece a la pagina seleccionada
      int y_offSet=0;
      //if (f<2) y_offSet=150;
      M5.Display.drawRect(mRot[f]->x, mRot[f]->y, mRot[f]->w-1, mRot[f]->h-1, DARKGREY);
      pos_x=mRot[f]->x+5;  
      pos_y=mRot[f]->y+5; //32+y_offSet;
      M5.Display.setCursor(pos_x,pos_y);
      M5.Display.setTextColor(DARKGREY, BLACK);
      M5.Display.print(mRot[f]->text);
    }
  }


}


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void drawPADsound(byte f, int color){
  M5.Display.fillRect(mBoton[f]->x + 68, mBoton[f]->y + 15, 24, 12, color);
}

void drawSEQmark(byte f, int color){
  int bval;
  M5.Display.fillRect(mBseq[f]->x + 21, mBseq[f]->y + 70, 24, 12, color);
  if (color==ZRED){
    if (isMelodic[selected_sound]) {
      bval=melodic[selected_sound][f];
    } else {
      bval=ROTvalue[selected_sound][12];
    }
  } else {
    bval=0;
  }
  //if (bval!=0){
    int tx=M5.Display.textWidth(String(bval));
    int ty=M5.Display.fontHeight();
    int pos_x=mBseq[f]->x+(mBseq[f]->w/2)-(tx/2);
    int pos_y=mBseq[f]->y+(mBseq[f]->h/2)-(ty/2);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.fillRect(mBseq[f]->x+9, pos_y, mBseq[f]->w-19, ty, BLACK);
    if (bval!=0){ 
      M5.Display.setCursor(pos_x, pos_y);
      M5.Display.print(bval);
    }
    // if (bval!=0){ 
    //   M5.Display.printf("%-3d", bval);
    // } else {
    //   M5.Display.printf("%-3s", "");
    // }
  //}
}

void drawSTEPmark(byte f, int color){
  M5.Display.fillRect(mBseq[f]->x + 20, mBseq[f]->y + 15, 24, 12, color);

}

void drawBTPAD(byte bt, int color, String texto = "") { // only 1
  byte f=bt;
  mBoton[bt]->color = color;
  M5.Display.drawRect(mBoton[f]->x + 3, mBoton[f]->y + 3, mBoton[f]->w - 7, mBoton[f]->h - 7, color);

}

///////////////////////////////////////////////////////////////////////////////////

void drawBTSEQ(byte bt) {

  byte f=bt;
  M5.Display.drawRect(mBseq[f]->x + 3, mBseq[f]->y + 3, mBseq[f]->w - 7, mBseq[f]->h - 7, mBseq[f]->color);
  M5.Display.drawRect(mBseq[f]->x + 4, mBseq[f]->y + 4, mBseq[f]->w - 9, mBseq[f]->h - 9, mBseq[f]->color);
  M5.Display.drawRect(mBseq[f]->x + 5, mBseq[f]->y + 5, mBseq[f]->w - 11, mBseq[f]->h - 11, mBseq[f]->color);
  M5.Display.drawRect(mBseq[f]->x + 6, mBseq[f]->y + 6, mBseq[f]->w - 13, mBseq[f]->h - 13, mBseq[f]->color);
  M5.Display.drawRect(mBseq[f]->x + 7, mBseq[f]->y + 7, mBseq[f]->w - 15, mBseq[f]->h - 15, mBseq[f]->color);
  M5.Display.drawRect(mBseq[f]->x + 8, mBseq[f]->y + 8, mBseq[f]->w - 17, mBseq[f]->h - 17, mBseq[f]->color);
}


void drawBT(byte bt, int color, String texto = "") {

  if (mBoton[bt]->rPage==0 || mBoton[bt]->rPage==rPage) {
  } else {
    return;
  }
  mBoton[bt]->color = color;
  byte f=bt;

  //M5.Display.drawRect(mBoton[f]->x + 2, mBoton[f]->y + 2, mBoton[f]->w - 5, mBoton[f]->h - 5, color);
  
  //
    M5.Display.drawRect(mBoton[f]->x + 5, mBoton[f]->y + 5, mBoton[f]->w - 11, mBoton[f]->h - 11, color);

  if (f<16) {

    //M5.Display.drawRect(mBoton[f]->x + 4, mBoton[f]->y + 4, mBoton[f]->w -  9, mBoton[f]->h -  9, color);
    //M5.Display.drawRect(mBoton[f]->x + 5, mBoton[f]->y + 5, mBoton[f]->w - 11, mBoton[f]->h - 11, color);

    M5.Display.drawRect(mBoton[f]->x + 6, mBoton[f]->y + 6, mBoton[f]->w - 13, mBoton[f]->h - 13, color);
    M5.Display.drawRect(mBoton[f]->x + 7, mBoton[f]->y + 7, mBoton[f]->w - 15, mBoton[f]->h - 15, color);
    //M5.Display.drawRect(mBoton[f]->x + 8, mBoton[f]->y + 8, mBoton[f]->w - 17, mBoton[f]->h - 17, color);

  } 
  // if (color!=DARKGREY) {
  //   M5.Display.drawRect(mBoton[f]->x + 4, mBoton[f]->y + 4, mBoton[f]->w - 9, mBoton[f]->h - 9, color);
  //   M5.Display.drawRect(mBoton[f]->x + 5, mBoton[f]->y + 5, mBoton[f]->w - 11, mBoton[f]->h - 11, color);  
  // } else {
  //   M5.Display.drawRect(mBoton[f]->x + 4, mBoton[f]->y + 4, mBoton[f]->w - 9, mBoton[f]->h - 9, BLACK);
  //   M5.Display.drawRect(mBoton[f]->x + 5, mBoton[f]->y + 5, mBoton[f]->w - 11, mBoton[f]->h - 11, BLACK);      
  // } 
   if (color!=DARKGREY) {
    M5.Display.drawRect(mBoton[f]->x + 6, mBoton[f]->y + 6, mBoton[f]->w - 13, mBoton[f]->h - 13, color);
    M5.Display.drawRect(mBoton[f]->x + 7, mBoton[f]->y + 7, mBoton[f]->w - 15, mBoton[f]->h - 15, color);  
  } else {
    M5.Display.drawRect(mBoton[f]->x + 6, mBoton[f]->y + 6, mBoton[f]->w - 13, mBoton[f]->h - 13, BLACK);
    M5.Display.drawRect(mBoton[f]->x + 7, mBoton[f]->y + 7, mBoton[f]->w - 15, mBoton[f]->h - 15, BLACK);      
  } 

  /////////////////////// Button texts

  String btext;
  if (texto == "") {
    btext=mBoton[f]->text;
  } else {
    btext=texto;
  }

  int tx=M5.Display.textWidth(btext);
  int ty=M5.Display.fontHeight();

  int pos_x=mBoton[f]->x+(mBoton[f]->w/2)-(tx/2);
  int pos_y=mBoton[f]->y+(mBoton[f]->h/2)-(ty/2);

  M5.Display.fillRect(mBoton[f]->x + 10, pos_y, mBoton[f]->w - 20, ty, BLACK);

  if (bt < 16) {
    M5.Display.setTextColor(LIGHTGREY, BLACK);
    if (modeZ == tWrite) {
      // if (bitRead(pattern[selected_sound], bt)) {  // note on
      //   M5.Display.setTextColor(ZRED, BLACK);
      //   M5.Display.setCursor(mBoton[f]->x + 10, pos_y);
      //   M5.Display.printf("%-3d", melodic[selected_sound][bt]);
      // }
    } else {
      //M5.Display.fillRect(mBoton[f]->x + 10, pos_y, mBoton[f]->w - 20, ty, BLACK);
      M5.Display.setTextColor(color, BLACK);
      M5.Display.setCursor(pos_x, pos_y); 
      M5.Display.print(btext);      
    }
      
  } else { 
    //M5.Display.fillRect(mBoton[f]->x + 10, pos_y, mBoton[f]->w - 20, ty, BLACK);
    M5.Display.setTextColor(color, BLACK);
    M5.Display.setCursor(pos_x, pos_y); 
    M5.Display.print(btext);
  } 
}

void REFRESH_KEYS() {
  if (refreshMODES) {
    refreshMODES = false;

    // Clear modes
    for (byte f = 16; f < MAX_BUTTONS; f++) {
      if ((mBoton[f]->rPage==0 || mBoton[f]->rPage==rPage) && f!=42) { // 42=shift
        drawBT(f, DARKGREY);
      }
    }    
    switch (modeZ) {
      case tPad:
        drawBT(16, DARKGREEN);
        break;
      case tSel:
        drawBT(16, ZBLUE, "SELECT");
        break;
      case tWrite:
        // drawBT(16, ZRED,  "WRITE");
        break;
      case tMute:
        drawBT(24, ZRED);
        break;
      case tSolo:
        drawBT(25, ZGREENALTER);
        break;
      case tMelod:
        drawBT(17, ZYELLOW, "MELODIC");
        break;
      case tClear:
        drawBT(21, ZGREENALTER, "CLEAR");
        break;
      case tFirst:
        drawBT(26, ZCYAN, "FIRST STEP");
        break;
      case tLast:
        drawBT(26, GREENYELLOW, "LAST STEP");
        break;
      case tLoaPS:
        drawBT(19, ZBLUE);
        break;
      case tLoadS:
        drawBT(19, ZCYAN, "LOAD S");
        break;
      case tLoadP:
        drawBT(19, ZGREENCLEAR, "LOAD P");
        break;
      case tSavPS:
        drawBT(20, ZGREENCLEAR);
        break;
      case tSaveS:
        drawBT(20, ZYELLOW, "SAVE S");
        break;
      case tSaveP:
        drawBT(20, ZRED, "SAVE P");
        break;
      case tRndS:
        drawBT(32, MAGENTA, "RND SOUND");
        break;
      case tRndP:
        drawBT(18, ZYELLOW, "RND PATTERN");
        break;
      case tRndNo:
        drawBT(18, ZCYAN,   "RND NOTES");
        break;
      case tPiano:
        drawBT(21, MAGENTA,"PIANO");
        break;
      case tMemo:
        drawBT(31, ZGREENALTER,"MEMORY");
        break;
      case tPfirs:
        drawBT(26, ZCYAN, "FIRST PAT");
        break;
      case tPlast:
        drawBT(26, GREENYELLOW, "LAST PAT");
        break;
      case tReverb:
        drawBT(33, ZYELLOW, "REVERB");
        break;        
      case tDelay:
        drawBT(34, ZGREENCLEAR, "DELAY");
        break; 
      case tChorus:
        drawBT(35, ZCYAN, "CHORUS");
        break;          
      case tFlanger:
        drawBT(36, GREENYELLOW, "FLANGER");
        break;  
      case tTremolo:
        drawBT(37, ZBLUE, "TREMOLO");
        break; 
      case tRingmod:
        drawBT(43, ZCYAN, "RING MOD");
        break; 
      case tDistortion:
        drawBT(44, ZGREENCLEAR, "DISTORTION");
        break; 
      case tBitcrusher:
        drawBT(45, GREENYELLOW, "BITCRUSHER");
        break; 
    }

    switch (rPage) {
      case 0:
        drawBT(28, ZCYAN, "SOUND");
        break;
      case 1:
        drawBT(29, ZGREENCLEAR, "GLOBAL");
        break;
      case 2:
        drawBT(30, MAGENTA, "FX");
        break;
    }
    // play/stop
    if (playing) {
      drawBT(22, DARKGREEN);
    }
    if (recording) {
      drawBT(22, ZRED, "   REC   ");
    }
    // song
    if (songing) {
      drawBT(23, ZBLUE);
    }
 
  }

  if (clearPATTERNPADS) {
    clearPATTERNPADS = false;
    clearPATTERN();
  }


  if (refreshPATTERN) {
    refreshPATTERN = false;

    switch (modeZ) {
      case tWrite:

        // for (byte f = 0; f < 16; f++) {
        //   if bitRead (pattern[selected_sound], f) {
        //     drawBT(f, ZRED);
        //   } else {
        //     drawBT(f, PADCOLOR);
        //   }
        // }
        break;
      case tMute:

        for (byte f = 0; f < 16; f++) {
          if bitRead (mutes, f) {
            drawBT(f, ZRED);
            paint_mute_marks(f,ZRED);
          } else {
            drawBT(f, PADCOLOR);
            paint_mute_marks(f,OSCURO);
          }
        }
        break;
      case tSolo:

        for (byte f = 0; f < 16; f++) {
          if bitRead (solos, f) {
            drawBT(f, ZGREENALTER);
            paint_solo_marks(f,ZGREENALTER);          
          } else {
            drawBT(f, PADCOLOR);
            paint_solo_marks(f,OSCURO);            
          }
        }
        break;
      case tMelod:
        for (byte f = 0; f < 16; f++) {
          if (isMelodic[f]) {
            drawBT(f, ZRED);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tClear:
        for (byte f = 0; f < 16; f++) {
          drawBT(f, PADCOLOR);
        }
        break;        
      case tFirst:
        for (byte f = 0; f < 16; f++) {
          if (firstStep == f) {
            drawBT(f, GREEN);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tLast:
        for (byte f = 0; f < 16; f++) {
          if (newLastStep == f) {
            drawBT(f, ZRED);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tPfirs:
        for (byte f = 0; f < 16; f++) {
          if (firstPattern == f) {
            drawBT(f, GREEN);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tPlast:
        for (byte f = 0; f < 16; f++) {
          if (newLastPattern == f) {
            drawBT(f, ZRED);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tReverb:
        for (byte f = 0; f < 16; f++) {
          if bitRead (reverbs, f) {
            drawBT(f, ZYELLOW);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;    
      case tDelay:
        for (byte f = 0; f < 16; f++) {
          if bitRead (delays, f) {
            drawBT(f, ZGREENCLEAR);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;             
      case tChorus:
        for (byte f = 0; f < 16; f++) {
          if bitRead (choruss, f) {
            drawBT(f, ZCYAN);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;        
      case tFlanger:
        for (byte f = 0; f < 16; f++) {
          if bitRead (flangers, f) {
            drawBT(f, GREENYELLOW);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tTremolo:
        for (byte f = 0; f < 16; f++) {
          if bitRead (tremolos, f) {
            drawBT(f, ZBLUE);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tRingmod:
        for (byte f = 0; f < 16; f++) {
          if bitRead (ringmods, f) {
            drawBT(f, ZCYAN);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tDistortion:
        for (byte f = 0; f < 16; f++) {
          if bitRead (distortions, f) {
            drawBT(f, ZGREENCLEAR);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      case tBitcrusher:
        for (byte f = 0; f < 16; f++) {
          if bitRead (bitcrushers, f) {
            drawBT(f, GREENYELLOW);
          } else {
            drawBT(f, PADCOLOR);
          }
        }
        break;
      default:
        clearPATTERNPADS=true;        
    }
  }

  if (refreshSEQ) {
    refreshSEQ = false;
    for (byte f = 0; f < 16; f++) {
      if bitRead (pattern[selected_sound], f) {
        drawSEQmark(f, ZRED);
      } else {
        drawSEQmark(f, OSCURO);
      }
    }
  }
  if (refreshPADSTEP) {
    refreshPADSTEP = false;
    // arreglo de visualizacion, quito 1 step
    int vstep=sstep-1;
    if (vstep<0) vstep=15;
    // Si SEQ activo muestro los pads que suenan
    if (playing) {
      for (byte f = 0; f < 16; f++) {
        if bitRead (pattern[f], vstep) {
          drawPADsound(f, BLUE);
        } else {
          drawPADsound(f, OSCURO);
        }
      }
    }
    // Mostrar step actual
    if (vstep!=s_old_sstep && s_old_sstep!=-1){
      drawSTEPmark(s_old_sstep, OSCURO);
    } 
    drawSTEPmark(vstep, YELLOW);
    s_old_sstep=vstep;  
    
  }


  if (clearPADSTEP) {
    clearPADSTEP = false;
    if (modeZ != 2) {
      for (byte f = 0; f < 16; f++) {
        drawPADsound(f, OSCURO);
        drawSTEPmark(f, OSCURO);
      }
    }

  }
}


void drawScreen1_ONLY1() {

  // botones
  for (byte f = 0; f < MAX_BUTTONS; f++) {
    if (f < 16) {
      //drawBTPAD(f, PADCOLOR);
      drawBT(f, PADCOLOR);
      drawPADsound(f,OSCURO);
      drawSTEPmark(f,OSCURO);
    } else {
      drawBT(f, DARKGREY);
    }
  }

  // botones secuenciador
  for (byte f = 0; f < 16; f++) {
    drawBTSEQ(f);
  }

  show_dark_selectors_ONLY1();

  // barras
  show_all_bars();

}

void clearPATTERN() {
  for (byte f = 0; f < 16; f++) {
    drawBT(f, PADCOLOR);
  }
}

/*
0 SAMPLE
1 WAVETABLE
2 A
3 D
4 S
5 R
6 INI
7 END
8 REVERSE
9 ENVELOPE
10 LENGTH
11 MODULATION
12 PITCH
13 PAN
14 VOL
15 FILTER
16 TYPE
*/
/*
17 M PITCH
18 TRANSPOSE
19 OCTAVE
20 SCALE
21 SYNC
22 BPM
23 MOVE
24 M VOL
25 M FILTER
26 26
---
27 REVERB LEVEL
28 DELAY LEVEL
29 CHORUS LEVEL
30 FLANGER LEVEL
31 TREMOLO LEVEL
32 REVERB TYPE
33 DELAY TIME
34 CHORUS TYPE
35 FLANGER TYPE
36 TREMOLO TYPE

37 RINGMOD LEVEL
38 RINGMOD TYPE

39 ADD NEXT SOUND
40 DETUNE

41 DISTORTION LEVEL
42 DISTORTION TYPE
43 BITCRUSHER LEVEL
44 BITCRUSHER TYPE
*/

void drawBar(byte bar){
  show_bar=bar;
}

void do_drawBar(byte bar){  

  byte f=bar;
  // valor
  int value=0;
  // Serial.print(ROTvalue[selected_sound][bar]);
  // Serial.print("-");
  // Serial.println(bar);
  switch (bar) {
    case 17:
      value=zmpitch; 
      break;
    case 18:
      value=ztranspose;
      break;
    case 19:
      value=octave;      
      break;
    case 20:
      value=selected_scale;      
      break;
    case 21:
      value=sync_state;      
      break;
    case 22:
      value=bpm;      
      break;
    case 23:
      value=zmove;      
      break;
    case 24:
      value=master_vol;      
      break;
    case 25:
      value=master_filter;      
      break;
    case 26:
      value=song_mode;      
      break;
    case 27:
      value=level_reverb;      
      break;
    case 28:
      value=level_delay;      
      break;
    case 29:
      value=level_chorus;      
      break;
    case 30:
      value=level_flanger;      
      break;
    case 31:
      value=level_tremolo;      
      break;
    case 32:
      value=reverb_type;      
      break;
    case 33:
      value=delay_time;      
      break;
    case 34:
      value=chorus_type;      
      break;
    case 35:
      value=flanger_type;      
      break;
    case 36:
      value=tremolo_type;      
      break;
    case 37:
      value=level_ringmod;      
      break;
    case 38:
      value=ringmod_type;      
      break;
    case 39:
      value=addnextsnd[selected_sound];      
      break;
    case 40:
      value=detune[selected_sound];      
      break;
    case 41:
      value=level_distortion;      
      break;
    case 42:
      value=distortion_type;      
      break;
    case 43:
      value=level_bitcrusher;      
      break;
    case 44:
      value=bitcrusher_type;      
      break;

    default:
      value=ROTvalue[selected_sound][bar];
  }

  int bar_max_width=150;
  if (bar==0 || bar==1) bar_max_width=310;
  int nvalue=map(value,min_values[bar],max_values[bar],0,bar_max_width);


  // Color
  int qcolor=ZCYAN;
  if (rPage==1) qcolor=ZGREENCLEAR;
  if (rPage==2) qcolor=MAGENTA;
  if (rPage==0) {
    qcolor=ZCYAN;
    if (ROTvalue[selected_sound][16]==1){ // poner en gris de 2 a 8
      if (bar>1 && bar<9) {
        qcolor=DARKGREY;
      }
    } else {
      if ((bar>8 && bar<12) || bar==40 ) {
        qcolor=DARKGREY;
      }      
    }
  }

  byte qbar=bar;
  int pos_x=mRot[bar]->x;  
  int pos_y=mRot[bar]->y;
  int ancho=mRot[bar]->w;
  int alto =mRot[bar]->h;

  
  int y_offSet=0;
  int t_background_size=12;

  // linea
  M5.Display.fillRect(pos_x+5, pos_y+22+y_offSet, nvalue,7, qcolor);
  M5.Display.fillRect(pos_x+5+nvalue, pos_y+22+y_offSet, bar_max_width-nvalue,7,OSCURO);

  // valor
  M5.Display.fillRect(pos_x+5, pos_y+32+y_offSet, bar_max_width,M5.Display.fontHeight(),OSCURO);
  M5.Display.setCursor(pos_x+5,pos_y+32+y_offSet);

  M5.Display.setTextColor(qcolor, OSCURO);

  if (f>1 && f!=20) M5.Display.printf("%-*d", t_background_size,value);

  // special
  if (f==20) {
    snprintf(bufferSeguro, sizeof(bufferSeguro), "%-12s", nombresEscalas[selected_scale]);
    M5.Display.print(bufferSeguro);
  }
  if (f<2) {
    if (ROTvalue[selected_sound][16]==0) {
      snprintf(bufferSeguro, sizeof(bufferSeguro), "%-26s", SAMPLE_NAMES[ROTvalue[selected_sound][0]].c_str());      
    } else {
      snprintf(bufferSeguro, sizeof(bufferSeguro), "%-26s",  txt_wtables[ROTvalue[selected_sound][1]]);      
    }
    M5.Display.print(bufferSeguro);
  }

  if (bar<9 || bar==16) { // todos los rot relacionados
     drawWaveform();
  }
  show_bar=-1;
}

void draw_pagerot_marks(){
  for (byte f = 0; f < MAX_BARS; f++) { 
    // Marca de pagerot
    if (pageRot==mRot[f]->pageRot){
      M5.Display.drawRect(mRot[f]->x+1, mRot[f]->y+1, mRot[f]->w-3, mRot[f]->h-3, ZRED);
    } else {
      M5.Display.drawRect(mRot[f]->x+1, mRot[f]->y+1, mRot[f]->w-3, mRot[f]->h-3, BLACK);
    }
  }
}

void draw_sound_bars2(){
  refresh_sound_bars=true;
}

void draw_global_bars2(){ // only 1
  for (byte f = 16; f < MAX_BARS; f++) { 
    do_drawBar(f);
  }
}


void REFRESH_STATUS(){
  
  int pos_x;
  int pos_y;

  // ticks: selected sound, pattern, soundset, memory
  if (selected_sound!=s_old_selected_sound) {    
    pos_x=mBoton[s_old_selected_sound]->x+50;
    pos_y=mBoton[s_old_selected_sound]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);
    pos_x=mBoton[selected_sound]->x+50;
    pos_y=mBoton[selected_sound]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, ZYELLOW); 
    s_old_selected_sound=selected_sound; 
  }

  if (selected_pattern!=s_old_selected_pattern) {
    pos_x=mBoton[s_old_selected_pattern]->x+66;
    pos_y=mBoton[s_old_selected_pattern]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);
    pos_x=mBoton[selected_pattern]->x+66;
    pos_y=mBoton[selected_pattern]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, GREEN); 
    s_old_selected_pattern=selected_pattern; 
  }

  if (selected_sndSet!=s_old_selected_sndSet) {    
    pos_x=mBoton[s_old_selected_sndSet]->x+82;
    pos_y=mBoton[s_old_selected_sndSet]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);
    pos_x=mBoton[selected_sndSet]->x+82;
    pos_y=mBoton[selected_sndSet]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, ZBLUE);
    s_old_selected_sndSet=selected_sndSet; 
  }

  if (selected_memory!=s_old_selected_memory) {  
    pos_x=mBoton[s_old_selected_memory]->x+98;
    pos_y=mBoton[s_old_selected_memory]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, OSCURO);   
    pos_x=mBoton[selected_memory]->x+98; 
    pos_y=mBoton[selected_memory]->y+70;
    M5.Display.fillRect(pos_x, pos_y, 12, 8, MAGENTA);
    s_old_selected_memory=selected_memory;
  }

  // selected rotary
   if (selected_rot!=s_old_selected_rot) {
    if (s_old_selected_rot!=99) {
      if (mRot[s_old_selected_rot]->rPage==rPage){ 
        M5.Display.drawRect(mRot[s_old_selected_rot]->x, mRot[s_old_selected_rot]->y, mRot[s_old_selected_rot]->w-1, mRot[s_old_selected_rot]->h-1, DARKGREY); 
        M5.Display.drawRect(mRot[s_old_selected_rot]->x+1, mRot[s_old_selected_rot]->y+1, mRot[s_old_selected_rot]->w-3, mRot[s_old_selected_rot]->h-3, BLACK); 
      }
    }
    if (mRot[selected_rot]->rPage==rPage){ 
      M5.Display.drawRect(mRot[selected_rot]->x, mRot[selected_rot]->y, mRot[selected_rot]->w-1, mRot[selected_rot]->h-1, ZYELLOW); 
      M5.Display.drawRect(mRot[selected_rot]->x+1, mRot[selected_rot]->y+1, mRot[selected_rot]->w-3, mRot[selected_rot]->h-3, ZYELLOW);     
      s_old_selected_rot=selected_rot;
    }
    
   }

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void fillBPOS() {

  /// sequencer buttons
  mBseq[0]  = new Bseq(    38, 410, 66, 100, bRojo);
  mBseq[1]  = new Bseq(   103, 410, 66, 100, bRojo);
  mBseq[2]  = new Bseq(   168, 410, 66, 100, bRojo);
  mBseq[3]  = new Bseq(   233, 410, 66, 100, bRojo);
  mBseq[4]  = new Bseq(   298, 410, 66, 100, bNaranja);
  mBseq[5]  = new Bseq(   363, 410, 66, 100, bNaranja);
  mBseq[6]  = new Bseq(   428, 410, 66, 100, bNaranja);
  mBseq[7]  = new Bseq(   493, 410, 66, 100, bNaranja);
  mBseq[8]  = new Bseq(   558, 410, 66, 100, bAmarillo);
  mBseq[9]  = new Bseq(   623, 410, 66, 100, bAmarillo);
  mBseq[10] = new Bseq(   688, 410, 66, 100, bAmarillo);
  mBseq[11] = new Bseq(   753, 410, 66, 100, bAmarillo);
  mBseq[12] = new Bseq(   818, 410, 66, 100, bBlanco);                      
  mBseq[13] = new Bseq(   883, 410, 66, 100, bBlanco);
  mBseq[14] = new Bseq(   948, 410, 66, 100, bBlanco);
  mBseq[15] = new Bseq(  1013, 410, 66, 100, bBlanco);

  // Buttons/pads
  // Boton(int _x, int _y, int _w, int _h, String _txt, uint8_t _rPage) { //// rPage=0 = all pages
  mBoton[0]  = new Boton(    0, 520, 160, 100, "0",0);
  mBoton[1]  = new Boton(  160, 520, 160, 100, "1",0);
  mBoton[2]  = new Boton(  320, 520, 160, 100, "2",0);
  mBoton[3]  = new Boton(  480, 520, 160, 100, "3",0);
  mBoton[4]  = new Boton(  640, 520, 160, 100, "4",0);
  mBoton[5]  = new Boton(  800, 520, 160, 100, "5",0);
  mBoton[6]  = new Boton(  960, 520, 160, 100, "6",0);
  mBoton[7]  = new Boton( 1120, 520, 160, 100, "7",0);
  mBoton[8]  = new Boton(    0, 620, 160, 100, "8",0);
  mBoton[9]  = new Boton(  160, 620, 160, 100, "9",0);
  mBoton[10] = new Boton(  320, 620, 160, 100, "10",0);
  mBoton[11] = new Boton(  480, 620, 160, 100, "11",0);
  mBoton[12] = new Boton(  640, 620, 160, 100, "12",0);                      
  mBoton[13] = new Boton(  800, 620, 160, 100, "13",0);
  mBoton[14] = new Boton(  960, 620, 160, 100, "14",0);
  mBoton[15] = new Boton( 1120, 620, 160, 100, "15",0);

  mBoton[16] = new Boton(    0, 300, 160, 100, "PAD",0);
  mBoton[17] = new Boton(  160, 100, 160, 100, "MELODIC",1);
  mBoton[18] = new Boton(  640, 300, 160, 100, "RND PATTERN",0);
  mBoton[19] = new Boton(  800, 300, 160, 100, "LOAD",0);
  mBoton[20] = new Boton(  800, 100, 160, 100, "SAVE",1);
  mBoton[22] = new Boton(  960, 300, 160, 100, "PLAY",0);
  mBoton[23] = new Boton(  960, 100, 160, 100, "SONG",1);
  mBoton[42] = new Boton( 1120, 410, 160, 100, "SHIFT",0);



  mBoton[24] = new Boton(  160, 300, 160, 100, "MUTE",0);
  mBoton[25] = new Boton(  320, 300, 160, 100, "SOLO",0);
  mBoton[32] = new Boton(  480, 300, 160, 100, "RND SOUND",0);
  mBoton[26] = new Boton(  320, 100, 160, 100, "STEP INIT",1);
  mBoton[21] = new Boton(  640, 100, 160, 100, "PIANO",1);
  mBoton[31] = new Boton(  480, 100, 160, 100, "MEMORY",1);
  mBoton[27] = new Boton(  960,   0, 160, 100, "SILENCE",1);
  mBoton[41] = new Boton( 1120, 300, 160, 100, "-10",0);

  mBoton[28] = new Boton(    0,   0, 160, 100, "SOUND",0);
  mBoton[29] = new Boton(    0, 100, 160, 100, "GLOBAL",0);
  mBoton[30] = new Boton(    0, 200, 160, 100, "FX",0);

  mBoton[38] = new Boton( 1120,   0, 160, 100, "+10",0);
  mBoton[39] = new Boton( 1120, 100, 160, 100, "+1",0);
  mBoton[40] = new Boton( 1120, 200, 160, 100, "-1",0);


  mBoton[33] = new Boton(  160,   0, 160, 100, "REVERB",2);
  mBoton[34] = new Boton(  480,   0, 160, 100, "DELAY",2);
  mBoton[35] = new Boton(  160, 100, 160, 100, "CHORUS",2);
  mBoton[36] = new Boton(  800,   0, 160, 100, "FLANGER",2);
  mBoton[37] = new Boton(  480, 100, 160, 100, "TREMOLO",2);
  mBoton[43] = new Boton(  800, 100, 160, 100, "RING MOD",2);
  mBoton[44] = new Boton(  160, 200, 160, 100, "DISTORTION",2); 
  mBoton[45] = new Boton(  480, 200, 160, 100, "BITCRUSHER",2); 

  mBoton[46] = new Boton(  800, 200, 80, 100, "0",2); 
  mBoton[47] = new Boton(  880, 200, 80, 100, "1",2); 
  mBoton[48] = new Boton(  960, 200, 80, 100, "2",2); 
  mBoton[49] = new Boton( 1040, 200, 80, 100, "3",2); 

  // Rots/sliders/selectors
  // Rot(int _x, int _y, int _w, int _h, String _txt, uint8_t _pageRot, uint8_t _cc, uint8_t _rPage)

  mRot[0] =  new Rot(  160,   0, 320, 200, "SAMPLE/WAVE", 0,48, 0);
  mRot[1] =  new Rot(  160,   0, 320, 200, "SAMPLE/WAVE", 0,48, 0);

  mRot[2] =  new Rot(  480,   0, 160,  50, "A",           0,49, 0);
  mRot[3] =  new Rot(  480,  50, 160,  50, "D",           0,50, 0);
  mRot[4] =  new Rot(  480, 100, 160,  50, "S",           0,51, 0);
  mRot[5] =  new Rot(  480, 150, 160,  50, "R",           0,52, 0);
  mRot[6] =  new Rot(  640,   0, 160,  50, "INI",         0,53, 0);
  mRot[7] =  new Rot(  640,  50, 160,  50, "END",         0,54, 0);
  mRot[8] =  new Rot(  640, 100, 160,  50, "REVERSE",     0,99, 0);

  mRot[9] =  new Rot(  800,   0, 160,  50, "ENVELOPE",    1,48, 0);
  mRot[10] = new Rot(  800,  50, 160,  50, "LEN",         1,49, 0);
  mRot[11] = new Rot(  800, 100, 160,  50, "MOD",         1,50, 0);
  mRot[40] = new Rot(  800, 150, 160,  50, "DETUNE",      1,51, 0);
  mRot[39] = new Rot(  800, 200, 160,  50, "ADD NEXT SND",0,55, 0);


  mRot[14] = new Rot(  960,   0, 160,  50, "VOL",         1,52, 0);
  mRot[13] = new Rot(  960,  50, 160,  50, "PAN",         1,53, 0);
  mRot[12] = new Rot(  960, 100, 160,  50, "PITCH",       1,54, 0);
  mRot[15] = new Rot(  960, 150, 160,  50, "FILTER",      1,55, 0);
  mRot[16] = new Rot(  960, 200, 160,  50, "TYPE",        1,99, 0);

//
  mRot[24] = new Rot(  160,   0, 160,  50, "M VOL",       2,48, 1);
  mRot[18] = new Rot(  320,   0, 160,  50, "TRANSPOSE",   2,49, 1);
  mRot[22] = new Rot(  480,   0, 160,  50, "BPM",         2,50, 1);
  mRot[19] = new Rot(  640,   0, 160,  50, "OCT",         2,51, 1);

  mRot[17] = new Rot(  160,  50, 160,  50, "M PITCH",     2,52, 1);
  mRot[25] = new Rot(  320,  50, 160,  50, "M FILTER",    2,53, 1);
  mRot[20] = new Rot(  480,  50, 160,  50, "SCALE",       2,54, 1);
  mRot[23] = new Rot(  800,   0, 160,  50, "MOVE",        2,55, 1);

  mRot[21] = new Rot(  640,  50, 160,  50, "SYNC",        2,99, 1);
  mRot[26] = new Rot(  800,  50, 160,  50, "SONG MODE",   2,99, 1);

//

  mRot[27] = new Rot(  320,  0, 160,  50, "R LEVEL",       3,48, 2); // r
  mRot[28] = new Rot(  640,  0, 160,  50, "D LEVEL",       3,49, 2); // d
  mRot[29] = new Rot(  320,100, 160,  50, "C LEVEL",       3,51, 2); // c
  mRot[30] = new Rot(  960,  0, 160,  50, "F LEVEL",       3,50, 2); // f
  mRot[31] = new Rot(  640,100, 160,  50, "T LEVEL",       3,52, 2); // t
  mRot[37] = new Rot(  960,100, 160,  50, "RM LEVEL",      3,53, 2); // rm

  mRot[32] = new Rot(  320,  50, 160,  50, "R TYPE",        4,48, 2);
  mRot[33] = new Rot(  640,  50, 160,  50, "D TIME",        4,49, 2);
  mRot[34] = new Rot(  320, 150, 160,  50, "C TYPE",        4,51, 2);
  mRot[35] = new Rot(  960,  50, 160,  50, "F TYPE",        4,50, 2);
  mRot[36] = new Rot(  640, 150, 160,  50, "T TYPE",        4,52, 2);
  mRot[38] = new Rot(  960, 150, 160,  50, "RM TYPE",       4,53, 2);

  mRot[41] = new Rot(  320, 200, 160,  50, "D LEVEL",       3,54, 2); // level 
  mRot[42] = new Rot(  320, 250, 160,  50, "D TYPE",        4,54, 2); // type 

  mRot[43] = new Rot(  640, 200, 160,  50, "B LEVEL",       3,55, 2); // level 
  mRot[44] = new Rot(  640, 250, 160,  50, "B TYPE",        4,55, 2); // type 

}

void refresh_shift_key(){
  int qcolor=DARKGREY;
  if (shifting) qcolor=ORANGE;
  drawBT(42, qcolor);
}

////////////////////////////////////////////////////////////////////////////////

void analizarOnda(uint8_t type, int soundIndex) {

  int16_t currentMin = INT16_MAX;
  int16_t currentMax = INT16_MIN;

  int msc=MAX_SAMPLES_COUNT;
  if (type) msc=WT_COUNT;

    int sampleIndex = ROTvalue[soundIndex][type];
  
    if (sampleIndex < 0 || sampleIndex >= msc) { 
        cacheMinVal[type][soundIndex] = -1; cacheMaxVal[type][soundIndex] = 1; return; 
    }
    const int16_t* miSample;
    if (type) {
       miSample= wtables[wavs[soundIndex]];
    } else {
       miSample = SAMPLES[sampleIndex];
    }

    if (miSample == NULL) {
        cacheMinVal[type][soundIndex] = -1; cacheMaxVal[type][soundIndex] = 1; return; 
    }

    size_t length = ENDS[sampleIndex];
    if (type) length = 1024;
    if (length == 0) return;

    for (size_t i = 0; i < length; i++) {
       int16_t val = miSample[i]; 
       if (val < currentMin) currentMin = val;
       if (val > currentMax) currentMax = val;
       
       // Evitar Watchdog en samples largos
       if (i % 5000 == 0) yield(); 
    }


  // Guardamos los datos seguros
  cacheMinVal[type][soundIndex] = currentMin;
  cacheMaxVal[type][soundIndex] = currentMax;
}

////////////////////////////////////////////////////////////////////////////



void drawWaveform() {

  waveSprite.fillSprite(OSCURO);

  // Recuperar valores pre-calculados
  int16_t minVal = cacheMinVal[0][selected_sound];
  int16_t maxVal = cacheMaxVal[0][selected_sound];
  if (ROTvalue[selected_sound][16] == 1){
    minVal = cacheMinVal[1][selected_sound];
    maxVal = cacheMaxVal[1][selected_sound];
  }
  // PROTECCIÓN
  if (minVal >= maxVal) {
     maxVal = minVal + 1; 
  }

  
  if (ROTvalue[selected_sound][16] == 1) { // SYNTH
    int wavIdx = wavs[selected_sound];
    
    if (wtables[wavIdx] != NULL) {
      for (int x = 0; x < WAVE_WIDTH - 1; x++) {
        size_t index1 = map(x, 0, WAVE_WIDTH - 1, 0, 1024);
        size_t index2 = map(x + 1, 0, WAVE_WIDTH - 1, 0, 1024);

        int y1 = map(wtables[wavIdx][index1], minVal, maxVal, WAVE_HEIGHT - 1, 0);
        int y2 = map(wtables[wavIdx][index2], minVal, maxVal, WAVE_HEIGHT - 1, 0);

        waveSprite.drawLine(x, y1, x + 1, y2, ZBLUE);
      }
    }
  }  else { // SAMPLE (FLASH)
    int sampleIdx = ROTvalue[selected_sound][0];
    const int16_t* miSample = SAMPLES[sampleIdx]; // Puntero a Flash

    if (miSample != NULL && ENDS[sampleIdx] > 0) {
      size_t sampleLen = ENDS[sampleIdx];

      for (int x = 0; x < WAVE_WIDTH - 1; x++) {
        // Mapeamos píxeles a índices del sample
        size_t index1 = map(x, 0, WAVE_WIDTH - 1, 0, sampleLen - 1);
        size_t index2 = map(x + 1, 0, WAVE_WIDTH - 1, 0, sampleLen - 1);

        // Leemos de Flash
        int16_t val1 = miSample[index1];
        int16_t val2 = miSample[index2];

        int y1 = map(val1, minVal, maxVal, WAVE_HEIGHT - 1, 0);
        int y2 = map(val2, minVal, maxVal, WAVE_HEIGHT - 1, 0);

        waveSprite.drawLine(x, y1, x + 1, y2, ZBLUE);
      }
    }
  }

  if (ROTvalue[selected_sound][16] == 0) {

    // ADSR
    int adsrWidth  = WAVE_WIDTH - 2;
    int adsrHeight = WAVE_HEIGHT - 2;

    uint32_t total = ROTvalue[selected_sound][2] + ROTvalue[selected_sound][3] + ROTvalue[selected_sound][5];
    if (total == 0) total = 1;

    uint32_t aX = (ROTvalue[selected_sound][2] * adsrWidth) / total;
    uint32_t dX = (ROTvalue[selected_sound][3] * adsrWidth) / total;
    uint32_t rX = (ROTvalue[selected_sound][5] * adsrWidth) / total;
    uint32_t sX = adsrWidth - (aX + dX + rX);

    uint32_t sustainY = map(ROTvalue[selected_sound][4], 0, 255, adsrHeight, 0); 

    // Puntos ADSR
    int bY = WAVE_HEIGHT-1; // Base Y (abajo)
    int tY = 0;             // Top Y (arriba)
    waveSprite.drawLine(0, bY, aX, tY, ZCYAN);                  // Attack
    waveSprite.drawLine(aX, tY, aX+dX, sustainY, ZCYAN);        // Decay
    waveSprite.drawLine(aX+dX, sustainY, aX+dX+sX, sustainY, ZCYAN); // Sustain
    waveSprite.drawLine(aX+dX+sX, sustainY, adsrWidth, bY, ZCYAN);   // Release

    // Marcadores Inicio/Fin
    int xini = map(ROTvalue[selected_sound][6], min_values[6], max_values[6], 0, WAVE_WIDTH - 1);
    int xfin = map(ROTvalue[selected_sound][7], min_values[7], max_values[7], 0, WAVE_WIDTH) - 1;
    
    waveSprite.drawLine(xini, 0, xini, WAVE_HEIGHT - 1, ZGREEN);
    waveSprite.drawLine(xfin, 0, xfin, WAVE_HEIGHT - 1, ZRED);
  
    // Indicador Reverse
    if (ROTvalue[selected_sound][8] == 1) { 
        waveSprite.setCursor(WAVE_WIDTH - 15, 15);
        waveSprite.setTextColor(ZRED, BLACK);
        waveSprite.print("R");
    }

  }

  // SEND
  waveSprite.pushSprite(WAVE_ORIGIN_X, WAVE_ORIGIN_Y);
}



void REFRESH_PAGE(){
  if (refresh_rPage){
    refresh_rPage=false;
    if (rPage!=old_rPage) {
      old_rPage=rPage;
      // Clear common zone
      M5.Display.fillRect(160, 0, 960, 300, BLACK);
      show_all_bars();
      refresh_sound_bars=true;
      refreshMODES=true;
    }
  }  
}


