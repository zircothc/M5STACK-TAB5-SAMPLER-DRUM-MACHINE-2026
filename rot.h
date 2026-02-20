#ifndef ROT_H
#define ROT

#include <Arduino.h>
class Rot {
  public:
    int x, y, w, h;
    String text;
    uint8_t pageRot;
    uint8_t cc;
    uint8_t rPage;
    uint16_t color;
    bool trigger_on;
  public:
    // Constructor
    Rot(int _x, int _y, int _w, int _h, String _txt, uint8_t _pageRot, uint8_t _cc, uint8_t _rPage) {
      x = _x;
      y = _y;
      w = _w;
      h = _h;
      text = _txt;
      pageRot = _pageRot;
      cc = _cc;
      rPage = _rPage;
      color = 0xFFFF; // default WHITE
      trigger_on = false;
    }
    void mostrar() {
    }
};
#endif