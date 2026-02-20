#ifndef BOTON_H
#define BOTON_H

#include <Arduino.h>

class Boton {
  public:
    int x, y, w, h;
    String text;
    uint16_t color;
    bool trigger_on;
    uint8_t rPage;
  public:
    // Constructor
    Boton(int _x, int _y, int _w, int _h, String _txt, uint8_t _rPage) {
      x = _x;
      y = _y;
      w = _w;
      h = _h;
      text = _txt;
      trigger_on = false;
      color = 0xFFFF; // Blanco por defecto
      rPage = _rPage;      
    }

    void mostrar() {
    }
};

class Bseq {
  public:
    int x, y, w, h;
    uint16_t color;
    bool trigger_on;
  public:
    // Constructor
    Bseq(int _x, int _y, int _w, int _h, uint16_t _color) {
      x = _x;
      y = _y;
      w = _w;
      h = _h;
      trigger_on = false;
      color = _color;
    }

    void mostrar() {
    }
};
#endif