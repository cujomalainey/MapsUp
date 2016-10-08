#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <neopixel.h>

#define PIN 3

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(20);
  matrix.setTextColor(matrix.Color(255, 0, 0));
}

void loop() {
  matrix.fillScreen(0);
  matrix.setCursor(0, 0);
  matrix.print(F("Jesse"));
  matrix.show();
  delay(100);
}