// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_TSL2561_U.h>
#include "mduino.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "GoogleMapsDirection.pb.h"
#include "images.h"

// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

#define PIN 0
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define LUX_SAMPLE_PERIOD 200
#define MATRIX_MAX_BRIGHTNESS 100
#define MATRIX_MIN_BRIGHTNESS 1
#define MATRIX_MOVING_AVERAGE_QUEUE_LENGTH 20
#define MATRIX_UPDATE_PERIOD 100
#define ARROW_IMAGE_OFFSET 8
#define DELAY_PERIOD 3000

#define DEMO_MODE
#define DEBUG

// LED pin
#define LED_PIN   LED_BUILTIN

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
sensor_t sensor;

uint8_t messageBuffer[256];
bool bufferLoaded = false;
OutgoingMessage om = OutgoingMessage();
mduino m = mduino();
GoogleMapsDirection message = GoogleMapsDirection_init_default;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN,
  #if defined(DEMO_MODE)
    NEO_MATRIX_BOTTOM  + NEO_MATRIX_RIGHT +
  #else
    NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  #endif
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

RingBuffer ring = RingBuffer();

// create peripheral instance, see pinouts above
BLEPeripheral           blePeripheral        = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
BLEService              ledService           = BLEService("713d0000503e4c75ba943148f18d941e");

// create switch characteristic
BLECharacteristic   switchCharacteristic = BLECharacteristic("713d0003503e4c75ba943148f18d941e", BLERead | BLEWrite, 255);

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print(F("Connected event, central: "));
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print(F("Disconnected event, central: "));
  Serial.println(central.address());
}

void switchCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print(F("Characteristic event, writen: "));
  uint8_t len = switchCharacteristic.valueLength();
  Serial.println(len);
  const unsigned char * data = switchCharacteristic.value();
  for (int i = 0; i < len; i++)
  {
    ring.store_char(data[i]);
  }

  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

bool decode_distance(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    memset(messageBuffer, 0, sizeof(messageBuffer));
    /* We could read block-by-block to avoid the large buffer... */
    if (stream->bytes_left > sizeof(messageBuffer) - 1)
        return false;

    if (!pb_read(stream, messageBuffer, stream->bytes_left))
        return false;

    /* Print the string, in format comparable with protoc --decode.
     * Format comes from the arg defined in main().
     */
    return true;
}

void drawMatrix()
{
  static uint32_t last_sample = 0;
  static int16_t pos_x = ARROW_IMAGE_OFFSET;
  static uint32_t delay = millis() + DELAY_PERIOD;
  uint32_t t = millis();
  uint16_t width = strlen((char*)messageBuffer)*6;
  if (t - last_sample > MATRIX_UPDATE_PERIOD && bufferLoaded)
  {
    last_sample = t;
    matrix.fillScreen(0);
    if (millis() > delay)
    {
        pos_x--;
        if (pos_x + width < ARROW_IMAGE_OFFSET) {
          pos_x = MATRIX_WIDTH;
        }
        if (pos_x == ARROW_IMAGE_OFFSET)
        {
          delay = millis() + DELAY_PERIOD;
        }
    }
    if (width < MATRIX_WIDTH - ARROW_IMAGE_OFFSET && pos_x == ARROW_IMAGE_OFFSET)
    {
      pos_x = ARROW_IMAGE_OFFSET;
    }
    matrix.setCursor(pos_x, 0);
    matrix.print(F(messageBuffer));
    matrix.fillRect(0, 0, 8, 8, 0);
    matrix.drawBitmap(0 + image_index[message.tdirection].x_offset,0, image_index[message.tdirection].img, image_index[message.tdirection].w, image_index[message.tdirection].h, matrix.Color(255, 255, 255));
    matrix.show();
  }
}

void serviceLux()
{
  static uint32_t last_sample = 0;
  static float movingAverage[MATRIX_MOVING_AVERAGE_QUEUE_LENGTH] = {0};
  static int pos = 0;
  uint32_t t = millis();
  if (t - last_sample > LUX_SAMPLE_PERIOD)
  {
    sensors_event_t event;
    last_sample = t;
    tsl.getEvent(&event);
    if (pos >= MATRIX_MOVING_AVERAGE_QUEUE_LENGTH)
    {
      pos = 0;
    }
    movingAverage[pos] = event.light;
    pos++;
    double avg = 0;
    for (int i = 0; i < MATRIX_MOVING_AVERAGE_QUEUE_LENGTH; i++)
    {
      avg += movingAverage[i];
    }
    avg = avg / MATRIX_MOVING_AVERAGE_QUEUE_LENGTH;
    avg = map(avg, sensor.min_value, sensor.max_value, MATRIX_MIN_BRIGHTNESS, MATRIX_MAX_BRIGHTNESS);
    // only exception to drawing outside of drawMatrix
    matrix.setBrightness(avg);
    matrix.show();
  }
}

void serviceComms()
{
  m.readPacket();
  if (m.getResponse().isAvailable()) {
    Serial.println(m.getResponse().getFrameLength());
    pb_istream_t stream = pb_istream_from_buffer(m.getResponse().getFrameData(), m.getResponse().getFrameLength());
    if (!pb_decode(&stream, GoogleMapsDirection_fields, &message))
    {
      Serial.print("Decode failed: ");
      Serial.println(PB_GET_ERROR(&stream));
    }
    bufferLoaded = true;
  }
  else if (m.getResponse().isError())
  {
    Serial.print("mduino error: ");
    Serial.println(m.getResponse().getErrorCode());
  }

}

void setup()
{
    Serial.begin(115200);
#if defined (__AVR_ATmega32U4__)
  delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
#endif

  // set LED pin to output mode
  pinMode(LED_PIN, OUTPUT);

  // set advertised local name and service UUID
  blePeripheral.setLocalName("TXRX");
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());

  // add service and characteristic
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(switchCharacteristic);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);

  // begin initialization
  blePeripheral.begin();

  Serial.println(F("BLE LED Peripheral"));

  // init matrix
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(1);
  matrix.setTextColor(matrix.Color(255, 255, 255));

  //init comms
  m.setSerial(ring);
  message.distance.funcs.decode  = &decode_distance;

  // init lux sensor
  if(!tsl.begin())
  {
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  tsl.enableAutoRange(true);
  tsl.getSensor(&sensor);

  #if defined(DEBUG)
    // delay(5000);
    matrix.show();
  #endif
}

void loop()
{
  serviceComms();
  serviceLux();
  drawMatrix();
  blePeripheral.poll();
}