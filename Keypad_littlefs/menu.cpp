#include "menu.h"
#include <bleKeyboard.h> 

void initButtons(BUTTON * button[]) {
  button[0] = new BUTTON(1, 81, 78, 78, "z", 'z');
  button[1] = new BUTTON(81, 81, 78, 78, "+", KEY_NUM_PLUS); 
  button[2] = new BUTTON(161,81, 78, 78, "-", KEY_NUM_MINUS);
  button[3] = new BUTTON(241, 81, 78, 78, "7", KEY_NUM_7); 
  button[4] = new BUTTON(1, 161, 78, 78, "/", KEY_NUM_SLASH);
  button[5] = new BUTTON(81, 161, 78, 78, "8", KEY_NUM_8);
  button[6] = new BUTTON(161, 161, 78, 78, "9", KEY_NUM_9);
  button[7] = new BUTTON(241, 161, 78, 158, "1", KEY_NUM_1); 
  button[8] = new BUTTON(1, 241, 78, 78, "4", KEY_NUM_4);
  button[9] = new BUTTON(81, 241, 78, 78, "5", KEY_NUM_5);
  button[10] = new BUTTON(161, 241, 78, 78, "6", KEY_NUM_6);
  button[11] = new BUTTON(1, 321, 78, 78, "*", KEY_NUM_0, CTRL_MOD  + ALT_MOD); // set camera to view
  button[12] = new BUTTON(81, 321, 78, 78, "2", KEY_NUM_2);
  button[13] = new BUTTON(161, 321, 78, 78, "*", KEY_NUM_0, CTRL_MOD); // set current object as active camera
  button[14] = new BUTTON(241, 321, 78, 158, "3", KEY_NUM_3);
  button[15] = new BUTTON(1, 401, 158, 78, "0", KEY_NUM_0);
  button[16] = new BUTTON(161, 401, 78, 78, "0", KEY_NUM_PERIOD);
}
