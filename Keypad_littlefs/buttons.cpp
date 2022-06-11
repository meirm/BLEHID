#include "buttons.h"

BUTTON::BUTTON(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char label[5], uint8_t code, uint8_t modifier) {
  _pressed = false;
  _lastFlip = 0;
  _x = x;
  _y = y;
  _w = w;
  _h = h;
  _latency = 500;
  strncpy(_label, label, 5);
  _label[4] = '\0';
  _code = code;
  _modifier = modifier;
}
char * BUTTON::getLabel(){
  return _label;
}
//void setLabel(char * label);
void BUTTON::press(boolean p) {
  if (p != _pressed) {
    _lastFlip = millis();
  }
  _pressed = p;
}

boolean BUTTON::justReleased() {
  if ((! _pressed) && (millis() - _lastFlip < _latency)) return true;
      return false;
}
boolean BUTTON::justPressed() {
  if ((_pressed) && (millis() - _lastFlip < _latency)) return true;
      return false;
}
boolean BUTTON::position() {
  return _pressed;
}
boolean BUTTON::contains(uint16_t x, uint16_t y) {
  if ((x > _x ) && (x < _x + _w) && (y > _y) && (y < _y + _h)) return true;
  return false;
}

uint8_t BUTTON::getCode(){
  return _code;
}
uint8_t BUTTON::getModifier(){
  return _modifier;
}
