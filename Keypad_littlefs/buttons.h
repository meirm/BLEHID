#include <Arduino.h>

#define CTRL_MOD 1
#define ALT_MOD 2
#define SHIFT_MOD 4

class BUTTON{
  public:
  BUTTON(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char label[5],uint8_t code, uint8_t modifier = 0);
  void press(boolean p);
  boolean justReleased();
  boolean justPressed();
  boolean position();
  boolean contains(uint16_t x,uint16_t y);
  uint8_t getCode();
  uint8_t getModifier();
  char * getLabel();
  //void setLabel(char * label);
  //uint16_t radius(uint16_t x,uint16_t y);
  
  private:
  boolean _pressed;
  uint16_t _latency;
  unsigned long _lastFlip;
  uint16_t _x;
  uint16_t _y;
  uint16_t _w;
  uint16_t _h;
  char _label[5];
  uint8_t _code;
  uint8_t _modifier;

};
