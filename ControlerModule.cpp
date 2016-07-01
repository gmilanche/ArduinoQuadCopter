#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define PIN_X 0
#define PIN_Y 1
#define PIN_R 2
#define PIN_P 3

#define PIN_MIN 0
#define PIN_MAX 1023

#define VAL_MIN 0
#define VAL_MID 500
#define VAL_MAX 1000

#define CE_PIN 7
#define CS_PIN 8

#define DEBUG 1

float val[4]; // val[0] -> x; val[1] -> y; val[2] -> z; val[3] -> p

RF24 radio(CE_PIN, CS_PIN);
const uint64_t pipe = 0xABCDABCD11LL;

#if DEBUG 
  unsigned long t0;
  unsigned long t1;
#endif

void setup() {
  #if DEBUG
    Serial.begin(115200);
    printf_begin();
  #endif

  radio.begin();
  radio.powerUp();
  radio.setChannel(108);
  radio.setPayloadSize(12);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.disableCRC();
  radio.setRetries(2, 5);
  radio.openWritingPipe(pipe);
  
  #if DEBUG 
    radio.printDetails();
    t0 = millis();
  #endif
}

void loop() {
  val[0] = calibratedAnalogRead(PIN_X) - VAL_MID;
  val[1] = calibratedAnalogRead(PIN_Y) - VAL_MID;
  val[2] = calibratedAnalogRead(PIN_R) - VAL_MID;
  val[3] = map(analogRead(PIN_P), PIN_MIN, PIN_MAX, VAL_MIN, VAL_MAX);
 
  #if DEBUG
    Serial.print(val[0]);  Serial.print("\t");
    Serial.print(val[1]);  Serial.print("\t");
    Serial.print(val[2]);  Serial.print("\t");
    Serial.print(val[3]);  Serial.print("\t");
  #endif

  byte data[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (int i = 0; i < 4; i++){
    int n = 0;
    while (val[i] > 255){
      data[i * 3 + n] = 255;
      val[i] = val[i] - 255;
      n++;
    }
    data[i * 3 + n] = val[i];
  }

  radio.writeFast(&data, 12); 
  radio.txStandBy();

  #if DEBUG 
    t1  = millis();
    Serial.print(t1 - t0);
    Serial.println("ms");
    t0 = t1;
  #endif
}

float calibratedAnalogRead(int pin){
  int value = analogRead(pin);
  float result;
  if (value <= 63){
    result = 0;
  } else if (value >= 64 && value <= 447){
    result = (value - 64) * 1.3 + 1;
  } else if (value >= 448 && value <= 575){
    result = 500;
  } else if (value >= 576 && value <= 959){
    result = (value - 576) * 1.3 + 501;
  } else if (value >= 960){
    result = 1000;
  }
  return result;
}