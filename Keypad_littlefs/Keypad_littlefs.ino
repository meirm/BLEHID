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
#include <Arduino.h>
#include <Streaming.h>
#include <Vector.h>
#include "buttons.h"
PNG png;

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#include <Adafruit_FT6206.h>


#include <BleKeyboard.h>

std::string deviceName = "TFT keypad";
std::string vendorName = "Riunx";

BleKeyboard bleKeyboard(deviceName, vendorName, 100);

Adafruit_FT6206 ts = Adafruit_FT6206();


TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

const int ELEMENT_COUNT_MAX = 20;
typedef Vector<BUTTON *> KEYS;
BUTTON * storage_array[ELEMENT_COUNT_MAX];

//------------------------------------------------------------------------------------------

int16_t xpos = 0;
int16_t ypos = 0;

KEYS key;
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
  key.setStorage(storage_array);
  Serial << "key.max_size(): " << key.max_size() << endl;

  Serial << "key.size(): " << key.size() << endl;


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

  loadPNGFile();
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
  for (uint8_t b = 0; b < key.size(); b++) {
    if (pressed && storage_array[b]->contains(t_x, t_y)) {
      storage_array[b]->press(true);  // tell the button it is pressed
    } else {
      storage_array[b]->press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < key.size(); b++) {
    if (storage_array[b]->justPressed()) {
      if (storage_array[b]->getCode() != '\0') {
        if (bleKeyboard.isConnected()) {
          Serial.println(storage_array[b]->getCode()); // key[b]->getLabel());
          if (storage_array[b]->getModifier() != 0 ) {
            if ( storage_array[b]->getModifier() & CTRL_MOD) {
              bleKeyboard.press(KEY_LEFT_CTRL);
              Serial.println("ctrl pressed");
            }
            if ( storage_array[b]->getModifier() & ALT_MOD) bleKeyboard.press(KEY_LEFT_ALT);
            if ( storage_array[b]->getModifier() & SHIFT_MOD) bleKeyboard.press(KEY_LEFT_SHIFT);
            bleKeyboard.press(storage_array[b]->getCode());
            bleKeyboard.release(storage_array[b]->getCode());
            if ( storage_array[b]->getModifier() & CTRL_MOD) bleKeyboard.release(KEY_LEFT_CTRL);
            if ( storage_array[b]->getModifier() & ALT_MOD) bleKeyboard.release(KEY_LEFT_ALT);
            if ( storage_array[b]->getModifier() & SHIFT_MOD) bleKeyboard.release(KEY_LEFT_SHIFT);

          } else {
            bleKeyboard.write(storage_array[b]->getCode());
          }

          delay(500);
        }

        delay(10); // UI debouncing
      }
    }
  }
}
//------------------------------------------------------------------------------------------
void loadPNGFile() {
  // Scan LittleFS and load any *.png files
  File root = LittleFS.open("/", "r");
  while (File file = root.openNextFile()) {
    String strname = file.name();
    //strname = "/" + strname;
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
      }
    } else if (!file.isDirectory() && strname.endsWith(".csv")) {
      File csv = LittleFS.open(strname.c_str(), "r");
      Serial.println("loading csv file");
      char buffer[128];
      while (csv.available()) {
        int l = csv.readBytesUntil('\n', buffer, sizeof(buffer) -1);
        buffer[l] = 0;
        if (buffer[0] == '#') continue; // comment line
        String pieces = buffer;
        Serial.println(pieces);
        uint16_t x, y, w, h;
        uint8_t code = 0;
        uint8_t mod = 0;
        int lastComma = 0;
        x = (uint16_t) pieces.substring(0,pieces.indexOf(",")).toInt();
        lastComma = pieces.indexOf(",");
        y = (uint16_t) pieces.substring(lastComma + 1,pieces.indexOf(",",lastComma + 1)).toInt();
        lastComma = pieces.indexOf(",", lastComma + 1);
        w = (uint16_t) pieces.substring(lastComma + 1,pieces.indexOf(",",lastComma + 1)).toInt();
        lastComma = pieces.indexOf(",", lastComma + 1);
        h = (uint16_t) pieces.substring(lastComma + 1,pieces.indexOf(",",lastComma + 1)).toInt();
        lastComma = pieces.indexOf(",", lastComma + 1);
        String label = pieces.substring(lastComma + 2,pieces.indexOf(",",lastComma + 1) - 1);
        lastComma = pieces.indexOf(",", lastComma + 1);
        String s_code = pieces.substring(lastComma + 1,pieces.indexOf(",",lastComma + 1));
        s_code.trim();
        lastComma = pieces.indexOf(",", lastComma + 1);
        if (s_code.startsWith("\"")) {
          code = s_code.substring(0,s_code.indexOf("\""))[0];
        }else if (s_code.startsWith("'")) {
          code = s_code.substring(0,s_code.indexOf("'"))[0];
        }else if (s_code.startsWith("KEY_NUM_PLUS")) {
          code = KEY_NUM_PLUS;
        }else if (s_code.startsWith("KEY_NUM_MINUS")) {
          code = KEY_NUM_MINUS;
        }else if (s_code.startsWith("KEY_NUM_SLASH")) {
          code = KEY_NUM_SLASH;
        }else if (s_code.startsWith("KEY_NUM_PERIOD")) {
          code = KEY_NUM_PERIOD;
        }else if (s_code.startsWith("KEY_NUM_0")) {
          code = KEY_NUM_0;
        }else if (s_code.startsWith("KEY_NUM_1")) {
          code = KEY_NUM_1;
        }else if (s_code.startsWith("KEY_NUM_2")) {
          code = KEY_NUM_2;
        }else if (s_code.startsWith("KEY_NUM_3")) {
          code = KEY_NUM_3;
        }else if (s_code.startsWith("KEY_NUM_4")) {
          code = KEY_NUM_4;
        }else if (s_code.startsWith("KEY_NUM_5")) {
          code = KEY_NUM_5;
        }else if (s_code.startsWith("KEY_NUM_6")) {
          code = KEY_NUM_6;
        }else if (s_code.startsWith("KEY_NUM_7")) {
          code = KEY_NUM_7;
        }else if (s_code.startsWith("KEY_NUM_8")) {
          code = KEY_NUM_8;
        }else if (s_code.startsWith("KEY_NUM_9")) {
          code = KEY_NUM_9;
        }else {
          code = s_code.charAt(0);
        }
        if (lastComma > -1){
          String s_mod = pieces.substring(lastComma + 1);
          if (s_mod.indexOf("CTRL_MOD") > -1){
            mod += 1;
          }
          if (s_mod.indexOf("ALT_MOD") > -1){
            mod += 2;
          }
          if (s_mod.indexOf("SHIFT_MOD") > -1){
            mod += 4;
          }
        }
        label.toCharArray(buffer,5);
         Serial << "x: " << x << ", y: " << y  << endl;
        key.push_back(new BUTTON(x, y, w, h, buffer ,code, mod));
      }
      csv.close();
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
