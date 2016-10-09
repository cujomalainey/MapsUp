#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_TSL2561_U.h>
#include "mduino.h"
#include <pb_common.h>
#include <pb_decode.h>
#include "GoogleMapsDirection.pb.h"

#define PIN 6
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8
#define LUX_SAMPLE_PERIOD 200
#define MATRIX_MAX_BRIGHTNESS 100
#define MATRIX_MIN_BRIGHTNESS 1
#define MATRIX_MOVING_AVERAGE_QUEUE_LENGTH 30

#define DEMO_MODE

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
sensor_t sensor;

char messageBuffer[256];
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

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    Serial.println("Decoding String");
    uint8_t buffer[128] = {0};

    /* We could read block-by-block to avoid the large buffer... */
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;

    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;

    /* Print the string, in format comparable with protoc --decode.
     * Format comes from the arg defined in main().
     */
    Serial.print("Decoding:");
    Serial.println((char*)buffer);
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print(F(buffer));
    matrix.show();
    return true;
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
      Serial.print(movingAverage[i]);
      Serial.print(", ");
      avg += movingAverage[i];
    }
    Serial.println();
    avg = avg / MATRIX_MOVING_AVERAGE_QUEUE_LENGTH;
    Serial.println(avg);
    avg = map(avg, sensor.min_value, sensor.max_value, MATRIX_MIN_BRIGHTNESS, MATRIX_MAX_BRIGHTNESS);
    Serial.println(avg);
    matrix.setBrightness(avg);
    matrix.show();
  }
}

void serviceComms()
{
  m.readPacket();
  if (m.getResponse().isAvailable()) {
    pb_istream_t stream = pb_istream_from_buffer(m.getResponse().getFrameData(), m.getResponse().getFrameLength());
    message.distance.funcs.decode = &decode_string;
    if (pb_decode(&stream, GoogleMapsDirection_fields, &message))
    {
      
    }
    else
    {
      Serial.println("Decoding Failed");
    }
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
    delay(5000);
  #endif
}

void loop()
{
  serviceComms();
  serviceLux();
}