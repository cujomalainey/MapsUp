#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_TSL2561_U.h>
#include "mduino.h"
#include <pb_common.h>
#include <pb_decode.h>
#include "GoogleMapsDirection.pb.h"
#include "images.h"

#define PIN 6
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define LUX_SAMPLE_PERIOD 200
#define MATRIX_MAX_BRIGHTNESS 100
#define MATRIX_MIN_BRIGHTNESS 1
#define MATRIX_MOVING_AVERAGE_QUEUE_LENGTH 20
#define MATRIX_UPDATE_PERIOD 100
#define ARROW_IMAGE_OFFSET 8

#define DEMO_MODE
#define DEBUG

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

bool decode_distance(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
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
  static uint32_t delay = millis() + 3000;
  uint32_t t = millis();
  uint16_t width = strlen((char*)messageBuffer)*6;
  if (t - last_sample > MATRIX_UPDATE_PERIOD && bufferLoaded)
  {
    last_sample = t;
    matrix.fillScreen(0);
    pos_x--;
    // Serial.print("X:");
    // Serial.print(pos_x);
    // Serial.print(" W:");
    // Serial.println(width);
    if (pos_x + width < ARROW_IMAGE_OFFSET) {
      pos_x = MATRIX_WIDTH;
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

}

void setup()
{
  // init matrix
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(1);
  matrix.setTextColor(matrix.Color(255, 255, 255));

  //init comms
  Serial1.begin(9600);
  m.setSerial(Serial1);
  message.distance.funcs.decode  = &decode_distance;
  message.direction.funcs.decode = &decode_direction;
  message.eta.funcs.decode       = &decode_eta;

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
    Serial.begin(9600);
    // delay(5000);
    matrix.show();
  #endif
}

void loop()
{
  serviceComms();
  serviceLux();
  drawMatrix();
}