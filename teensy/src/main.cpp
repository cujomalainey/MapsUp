#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "mduino.h"
#include "pb_common.h"
#include "pb_decode.h"

#define PIN 6
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8

#define DEMO_MODE

char messageBuffer[256];
OutgoingMessage om = OutgoingMessage();
mduino m = mduino();

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN,
  #if defined(DEMO_MODE)
    NEO_MATRIX_BOTTOM  + NEO_MATRIX_RIGHT +
  #else
    NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  #endif
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

void setup() {
  matrix.begin();
  Serial1.begin(9600);
  m.setSerial(Serial1);
  matrix.setTextWrap(false);
  matrix.setBrightness(5);
  matrix.setTextColor(matrix.Color(255, 255, 255));
}

void loop() {
  m.readPacket();
  if (m.getResponse().isAvailable()) {
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    sprintf(messageBuffer, "%s", m.getResponse().getFrameData());
    messageBuffer[m.getResponse().getFrameLength()] = '\0';
    Serial.println(messageBuffer);
    matrix.print(F(messageBuffer));
    matrix.show();
  }
}

