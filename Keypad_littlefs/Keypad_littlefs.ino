/*
  The TFT_eSPI library incorporates an Adafruit_GFX compatible
  button handling class, this sketch is based on the Arduin-o-phone
  example.

  This example diplays a keypad where numbers can be entered and
  send to the Serial Monitor window.

  The sketch has been tested on the ESP8266 (which supports SPIFFS)

  The minimum screen size is 320 x 240 as that is the keypad size.

  TOUCH_CS and SPI_TOUCH_FREQUENCY must be defined in the User_Setup.h file
  for the touch functions to do anything.
*/

// The SPIFFS (FLASH filing system) is used to hold touch screen
// calibration data
#define MAX_IMAGE_WIDTH 320 // Adjust for your images
#define FORMAT_LITTLEFS_IF_FAILED true
#define BLACK 0x0000
#define WHITE 0xFFFF
#include "FS.h"
#include <LITTLEFS.h>
#define FileSys LITTLEFS
#define LittleFS LITTLEFS

// Include the PNG decoder library
#include <PNGdec.h>

PNG png;

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#include <Adafruit_FT6206.h>
#include "menu.h"
#include "image.h"
#include <BleKeyboard.h>
BleKeyboard bleKeyboard;

Adafruit_FT6206 ts = Adafruit_FT6206();


TFT_eSPI tft = TFT_eSPI(); // Invoke custom library


BUTTON * key[nr_buttons];
//------------------------------------------------------------------------------------------

int16_t xpos = 0;
int16_t ypos = 0;

void setup() {
  // Use serial port
  Serial.begin(115200);
  Serial.println("started");
  // Initialise the TFT screen
  if (!ts.begin(18, 19, 40)) {
    Serial.println("Couldn't start touchscreen controller");
    while (true);
  }

  // Initialise FS
  if (!FileSys.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  
  initButtons(key);

  Serial.println("Starting BLE");
  bleKeyboard.begin();

  pinMode(TFT_BL, OUTPUT);
  tft.init();


  digitalWrite(TFT_BL, 128);

  //h = tft.height();
  //w = tft.width();
  //tft.setRotation(0);

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  int x = 1;
  int y = 1;

  
  Serial.println("Ready to type");
}

//------------------------------------------------------------------------------------------
void loop(void) {
  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
  bool pressed = false;
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    // Pressed will be set true is there is a valid touch on the screen
    t_x = p.x;
    t_y = p.y;

    //Serial.printf("%d %d\n", t_x, t_y);
    pressed = true;
  }

  // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < nr_buttons; b++) {
    if (pressed && key[b]->contains(t_x, t_y)) {
      key[b]->press(true);  // tell the button it is pressed
    } else {
      key[b]->press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < nr_buttons; b++) {
    if (key[b]->justPressed()) {
      if (key[b]->getCode() != '\0') {
        if (bleKeyboard.isConnected()) {
          Serial.println(key[b]->getCode()); // key[b]->getLabel());
          if (key[b]->getModifier() != 0 ) {
            if ( key[b]->getModifier() & CTRL_MOD) {bleKeyboard.press(KEY_LEFT_CTRL);Serial.println("ctrl pressed");}
            if ( key[b]->getModifier() & ALT_MOD) bleKeyboard.press(KEY_LEFT_ALT);
            if ( key[b]->getModifier() & SHIFT_MOD) bleKeyboard.press(KEY_LEFT_SHIFT);
            bleKeyboard.press(key[b]->getCode());
            bleKeyboard.release(key[b]->getCode());
            if ( key[b]->getModifier() & CTRL_MOD) bleKeyboard.release(KEY_LEFT_CTRL);
            if ( key[b]->getModifier() & ALT_MOD) bleKeyboard.release(KEY_LEFT_ALT);
            if ( key[b]->getModifier() & SHIFT_MOD) bleKeyboard.release(KEY_LEFT_SHIFT);
          
          }else {
          bleKeyboard.write(key[b]->getCode());
          }
          
          delay(500);
        }

        delay(10); // UI debouncing
      }
    }
  }
}
//------------------------------------------------------------------------------------------
void loadPNGFile(){
// Scan LittleFS and load any *.png files
  File root = LittleFS.open("/", "r");
  while (File file = root.openNextFile()) {
    String strname = file.name();
    strname = "/" + strname;
    Serial.println(file.name());
    
    // If it is not a directory and filename ends in .png then load it
    if (!file.isDirectory() && strname.endsWith(".png")) {
      // Pass support callback function names to library
      Serial.println("loading image");
      int16_t rc = png.open(strname.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);
      if (rc == PNG_SUCCESS) {
        tft.startWrite();
        Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        uint32_t dt = millis();
        if (png.getWidth() > MAX_IMAGE_WIDTH) {
          Serial.println("Image too wide for allocated line buffer size!");
        }
        else {
          rc = png.decode(NULL, 0);
          png.close();
        }
        tft.endWrite();
        // How long did rendering take...
        Serial.print(millis() - dt); Serial.println("ms");
        return;
      }
    }
  }
}


//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
  delay(1);
}
