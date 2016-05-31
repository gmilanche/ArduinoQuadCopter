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

float val_x;
float val_y;
float val_r;
float val_p;

int esc[4];

unsigned long t0;
unsigned long t1;
unsigned long td;

RF24 radio(CE_PIN, CS_PIN);
const uint64_t pipe = 0xABCDABCD11LL;

void setup() {
  Serial.begin(9600);
  printf_begin();
  
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
  radio.printDetails();
  
  t0 = millis();
}

void loop() {
  val_p = map(analogRead(PIN_P), PIN_MIN, PIN_MAX, VAL_MIN, VAL_MAX);
  val_x = (calibratedAnalogRead(PIN_X) - VAL_MID) * val_p * 0.00012;
  val_y = (calibratedAnalogRead(PIN_Y) - VAL_MID) * val_p * 0.00012;
  val_r = (calibratedAnalogRead(PIN_R) - VAL_MID) * val_p * 0.00012;
  val_p = val_p * 0.58;  // max output per esc 580+3x60=760
  
  /*
  Serial.print(val_x);
  Serial.print("\t");
  Serial.print(val_y);
  Serial.print("\t");
  Serial.print(val_r);
  Serial.print("\t");
  Serial.println(val_p);
  */

  esc[0] = (int) (val_p +     val_x  + neg(val_y) +     val_r );
  esc[1] = (int) (val_p + neg(val_x) + neg(val_y) + neg(val_r));
  esc[2] = (int) (val_p +     val_x  +     val_y  + neg(val_r));
  esc[3] = (int) (val_p + neg(val_x) +     val_y  +     val_r );

  Serial.print(esc[0]);
  Serial.print("\t");
  Serial.print(esc[1]);
  Serial.print("\t");
  Serial.print(esc[2]);
  Serial.print("\t");
  Serial.print(esc[3]);

  byte data[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (int i = 0; i < 4; i++){
    int n = 0;
    int esc_val = esc[i];
    while (esc_val > 255){
      data[i * 3 + n] = 255;
      esc_val = esc_val - 255;
      n++;
    }
    data[i * 3 + n] = esc_val;
  }

  radio.writeFast(&data, 12); 
  radio.txStandBy();

  t1  = millis();
  td = t1 - t0;
  Serial.print("\t");
  Serial.print(td);
  Serial.println("ms");
  t0 = t1;
}

float neg(float value) {
  float result = value - (value * 2);
  return result;
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
