#include <Servo.h>
#include <SPI.h>
#include <Wire.h>

#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#include "GY91.h"

#include <Kalman.h>
#include <PID_v1.h>

#define ESC_1_PIN 5
#define ESC_2_PIN 6
#define ESC_3_PIN 9
#define ESC_4_PIN 10

#define CE_PIN 7
#define CS_PIN 8

#define MIN_SIGNAL 1000
#define MAX_SIGNAL 2000

#define DEBUG 1

Servo motor[4];
int esc_pin[4] = { ESC_1_PIN, ESC_2_PIN, ESC_3_PIN, ESC_4_PIN };

RF24 radio(CE_PIN, CS_PIN);
const uint64_t radio_pipe = 0xABCDABCD11LL;
byte radio_data[12];

unsigned long radio_timer_0;
unsigned long radio_timer_n;

double ctlX, ctlY, ctlR, ctlP;






void setup() {
  Serial.begin(115200);
  printf_begin();
  initESCs();
  initRadioModule();
}

void loop() {

}





void getRadioData(){
  while(radio.available()) {
    radio.read(&radio_data, 12);
    radio_timer_0 = millis();
  }
  radio_timer_n = millis();
}


void initESCs(void) {
  delay(200);
  for (int i = 0; i < 4; i++){
    motor[i].attach(esc_pin[i]);
    motor[i].writeMicroseconds(MIN_SIGNAL);
    // todo: postaviti taster za test motora, led green-red.
  }
  delay(5000);
}

void initRadioModule(void) {
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
