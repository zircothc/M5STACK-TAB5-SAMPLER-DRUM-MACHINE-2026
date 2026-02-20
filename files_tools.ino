
void load_memory(){ // load selected memory from flash into memory_ arrays
  for (byte f = 0; f < 16; f++) {
    load_pattern_mem(f);
    load_sound_mem(f);
  }
 
}

////////////////////////////////////////////////////////////////////////////////////////////

void load_pattern_mem(uint8_t pat){

  // Leer pattren
  String nombreArchivoP = "/PAT" + String(pat)+"_"+String(selected_memory);
  File archivoP = SPIFFS.open(nombreArchivoP, FILE_READ);   
  if (!archivoP) {
    Serial.println("Error al abrir el archivo para leer");
    // Rellenar con 0
    memset(memory_pattern[pat], 0, sizeof(memory_pattern[pat]));
    memset(memory_melodic[pat], 0, sizeof(memory_melodic[pat]));
    return;
  }
  archivoP.read((uint8_t*)memory_pattern[pat], sizeof(memory_pattern[pat]));
  archivoP.read((uint8_t*)memory_melodic[pat], sizeof(memory_melodic[pat]));
  archivoP.close();

}

void load_sound_mem(uint8_t pat){
  
  // Leer sound
  String nombreArchivoS = "/SND" + String(pat)+"_"+String(selected_memory);
  File archivoS = SPIFFS.open(nombreArchivoS, FILE_READ);
  if (!archivoS) {
    Serial.println("Error al abrir el archivo para leer");
    // Rellenar con 0
    memset(memory_ROTvalue[pat], 0, sizeof(memory_ROTvalue[pat]));    
    return;
  }
  archivoS.read((uint8_t*)memory_ROTvalue[pat],     sizeof(memory_ROTvalue[pat]));
  archivoS.close();
  flag_ss=true; 
}

void load_pattern(uint8_t pat){
  memcpy(pattern, memory_pattern[pat], sizeof(pattern));
  memcpy(melodic, memory_melodic[pat], sizeof(melodic));
}

void load_sound(uint8_t pat){
   memcpy(ROTvalue, memory_ROTvalue[pat], sizeof(ROTvalue));
   for (byte f=0;f<16;f++){
     detune[f]=ROTvalue[f][17];
     addnextsnd[f]=ROTvalue[f][18];
     isMelodic[f]=ROTvalue[f][19];
   }
}

void save_pattern(uint8_t pat){
  
  // Guardar pattern
  String nombreArchivoP = "/PAT" + String(pat)+"_"+String(selected_memory);
  File archivoP = SPIFFS.open(nombreArchivoP, FILE_WRITE);   
  if (!archivoP) {
    Serial.println("Error al abrir el archivo para escribir");
    return;
  }
  archivoP.write((uint8_t*)pattern,    sizeof(pattern));
  archivoP.write((uint8_t*)melodic,    sizeof(melodic));

  archivoP.close();

  memcpy(memory_pattern[pat], pattern, sizeof(pattern));
  memcpy(memory_melodic[pat], melodic, sizeof(melodic));
 

}

void save_sound(uint8_t pat){
  
  // Guardar sound
  String nombreArchivoS = "/SND" + String(pat)+"_"+String(selected_memory);
  File archivoS = SPIFFS.open(nombreArchivoS, FILE_WRITE); 
  if (!archivoS) {
    Serial.println("Error al abrir el archivo para escribir");
    return;
  }
  archivoS.write((uint8_t*)ROTvalue,      sizeof(ROTvalue));
  archivoS.close();

  memcpy(memory_ROTvalue[pat],   ROTvalue,   sizeof(ROTvalue));
  
}