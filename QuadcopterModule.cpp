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

GY91 gy91;

Kalman kalmanX;
Kalman kalmanY;

Servo motor[4];
int esc_pin[4] = { ESC_1_PIN, ESC_2_PIN, ESC_3_PIN, ESC_4_PIN };

RF24 radio(CE_PIN, CS_PIN);
const uint64_t radio_pipe = 0xABCDABCD11LL;
byte radio_data[12];

unsigned long timer_0;
unsigned long timer_n;

unsigned long radio_timer_0;
unsigned long radio_timer_n;

int task100counter;

double ctlX, ctlY, ctlR, ctlP;
double accX, accY, accZ, accR;
double gyroX, gyroY, gyroZ;
double roll, pitch;
double kalX, kalY;

void setup() {
  Serial.begin(115200);
  printf_begin();
  initESCs();
  initRadioModule();
  initSensor();

  unsigned long t0 = timer_0;

  task100counter = 0;
  timer_0 = micros();

  unsigned long t0 = timer_0; //only for debug za merenje task100
}

void loop() {
  timer_n = micros();


 
  if ((timer_n - timer_0) >= 10000) {
    task100counter++;
    task100();
    timer_0 = timer_n;
  }
  
  if (task100counter >= 100) {
    task100counter = 0;
    Serial.print(t0 - micros());   Serial.print("\t");
  }

  /*
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
  */
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
  radio.openReadingPipe(1, radio_pipe);
  radio.startListening();
  radio.printDetails();
  radio_timer_0 = millis();
}

void getRadioData() {
  while(radio.available()) {
    radio.read(&radio_data, 12);
    radio_timer_0 = millis();
  }
  radio_timer_n = millis();
  ctlX = (int) (radio_data[0] + radio_data[1] + radio_data[2] + MIN_SIGNAL);
  ctlY = (int) (radio_data[3] + radio_data[4] + radio_data[5] + MIN_SIGNAL);
  ctlR = (int) (radio_data[6] + radio_data[7] + radio_data[8] + MIN_SIGNAL);
  ctlP = (int) (radio_data[9] + radio_data[10] + radio_data[11] + MIN_SIGNAL);
}

void getSensorData() {
  gy91.getRawData_MPU9250(data);
  accX = data[0];
  accY = data[1];
  accZ = data[2];
  gyroX = data[4];
  gyroY = data[5];
  gyroZ = data[6];
}

void calcMotorData() {

}

void setMotorData() {

}


void task100() {
  getRadioData();

  Serial.print(ctlX);  Serial.print("\t");
  Serial.print(ctlY);  Serial.print("\t");
  Serial.print(ctlR);  Serial.print("\t");
  Serial.print(ctlP);  Serial.print("\t");
  
  getSensorData();
  //calcMotorData();
  //setMotorData();
     
}

void initSensor() {
  gy91.initialize();
  double sens = 16384000;

  accX = 0;
  accY = 0;
  accZ = 0;

  for (int i = 0; i < 1000; i++) {
    gy91.getRawData_MPU9250(data);

    accX += data[0];
    accY += data[1];
    accZ += data[2];
  }

  //osetljivost rezultata
  accX = accX / sens;
  accY = accY / sens;
  accZ = accZ / sens;

  //normalizovanje vektora
  accR = sqrt(accX*accX + accY*accY + accZ*accZ);
  accX = accX / accR;
  accY = accY / accR;
  accZ = accZ / accR;
  
  roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  pitch = atan2(-accX, accZ) * RAD_TO_DEG;

  kalmanX.setAngle(roll); // Set starting angle
  kalmanY.setAngle(pitch);
}

void invertPitch() {

}
