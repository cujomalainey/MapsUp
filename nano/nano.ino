#include "mduino.h"

#define PIN 3
uint8_t outDataBuffer[5];
OutgoingMessage om = OutgoingMessage();
mduino m = mduino();

void setup() {
  Serial.begin(9600);
  m.setSerial(Serial);
  om.setData(outDataBuffer);
  om.setDataLength(sizeof(outDataBuffer));
  om.setMsgId(0);
}

void loop() {
  m.send(om);
  outDataBuffer[0]++;
  // delay(300);
}