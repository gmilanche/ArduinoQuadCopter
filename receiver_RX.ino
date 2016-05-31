#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define ESC_1_PIN 5
#define ESC_2_PIN 6
#define ESC_3_PIN 9
#define ESC_4_PIN 10

#define CE_PIN 7
#define CS_PIN 8

#define MIN_SIGNAL 1000
#define MAX_SIGNAL 2000

Servo motor[4];
int esc[4];
int esc_pin[4] = { ESC_1_PIN, ESC_2_PIN, ESC_3_PIN, ESC_4_PIN };

RF24 radio(CE_PIN, CS_PIN);
const uint64_t pipe = 0xABCDABCD11LL;
byte data[12];

unsigned long t0;
unsigned long t1;

void setup(void) {
  Serial.begin(9600);
  printf_begin();
  delay(200);
  for (int i = 0; i < 4; i++){
    motor[i].attach(esc_pin[i]);
    motor[i].writeMicroseconds(MIN_SIGNAL);
    // todo: postaviti taster za test motora, led green-red.
  }
  delay(5000);
  radio.begin();
  radio.powerUp();
  radio.setChannel(108);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.disableCRC();
  radio.setRetries(2, 5);
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  radio.printDetails();
  t0 = millis();
}

void loop(void){
  while(radio.available()){       
    radio.read(&data, 12);
    t0 = millis();
  }

  t1 = millis();

  if (t1 - t0 > 1000) {
    for (int i = 0; i < 4; i++) {
      motor[i].writeMicroseconds(1000);
    }
    Serial.println("no signal");
    // todo: add alarm buzzer, red signal led
  } else {
    for (int i = 0; i < 4; i++) {
      esc[i] = (int) (data[i * 3 + 0] + data[i * 3 + 1] + data[i * 3 + 2] + MIN_SIGNAL);
      motor[i].writeMicroseconds(esc[i]);
    }
    Serial.print(esc[0]);
    Serial.print("\t");
    Serial.print(esc[1]);
    Serial.print("\t");
    Serial.print(esc[2]);
    Serial.print("\t");
    Serial.print(esc[3]);
    Serial.println();
  }
}
