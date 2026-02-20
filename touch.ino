void read_touch(){

  // Lee 1 punto de toque “raw”
  int n = M5.Display.getTouchRaw(tp, 1);
  if (n > 0) {

    // Convierte a píxeles
    M5.Display.convertRawXY(tp, n);
      cox=tp[0].x;
      coy=tp[0].y;

      //  Serial.print(cox);
      //  Serial.print(" ");
      //  Serial.println(coy);

    if (!touchActivo){
      touchActivo = true; 


      for (byte f=0;f<MAX_BUTTONS;f++){
        if (mBoton[f]->rPage==0 || mBoton[f]->rPage==rPage) {
          if ( (cox > mBoton[f]->x) && (cox < (mBoton[f]->x+mBoton[f]->w)) && (coy > mBoton[f]->y) && (coy < (mBoton[f]->y+mBoton[f]->h)) ) {
            if (f==last_touched ){
              if (start_debounce+debounce_time > millis() ){
                break;
              } 
            }
            mBoton[f]->trigger_on=1;
            last_touched=f;
            start_debounce=millis();
            touchActivo = true;
            //  Serial.print("b ");
            //  Serial.println(f);
            break;
          }
        }
      }


      for (byte f=0;f<MAX_BARS;f++){
        if (mRot[f]->rPage==rPage){ // si el controlador pertenece a la pagina seleccionada
          if ( (cox > mRot[f]->x) && (cox < (mRot[f]->x+mRot[f]->w)) && (coy > mRot[f]->y) && (coy < (mRot[f]->y+mRot[f]->h)) ) {
            if (f==last_touched ){
              if (start_debounce+debounce_time > millis() ){
                break;
              } 
            }
            mRot[f]->trigger_on=1;
            last_touched=f;
            start_debounce=millis();
            touchActivo = true;
            //Serial.print("r ");
            //Serial.println(f);
            break;
          }
        }
      }

      for (byte f=0;f<16;f++){
        if ( (cox > mBseq[f]->x) && (cox < (mBseq[f]->x+mBseq[f]->w)) && (coy > mBseq[f]->y) && (coy < (mBseq[f]->y+mBseq[f]->h)) ) {
          if (f==last_touched ){
            if (start_debounce+debounce_time > millis() ){
              break;
            } 
          }
          mBseq[f]->trigger_on=1;
          last_touched=f;
          start_debounce=millis();
          touchActivo = true;
          //Serial.print("bs ");
          //Serial.println(f);
          break;
        }
      }
    }

  } else {
    touchActivo = false;
  }

}