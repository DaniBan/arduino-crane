#include <VarSpeedServo.h>
#include "HX711.h"

const int VRx = A0; // Connect to Analog Pin 0
const int VRy = A1; // Connect to Analog Pin 1
const int SW = 2; // Connect to Digital Pin 2

int angle = 90;

// dc motor pinout
int in1 = 7;
int in2 = 8;
int ena = 6;
int speed = 0;

int in3 = 12;
int in4 = 11;
int enb = 3;
volatile int mode = 0;

// servo
VarSpeedServo s;
int step = 5;

//buzzer
int buzzer = 13;
//int melody = 262;
//int noteDuration = 250;

int led = 5;
int ledState = 0;

int modeLed = 9;

//weight sensor
uint8_t dataPin = A2;
uint8_t clockPin = A3;
HX711 scale;

float calibration_factor = -1965;
float units;

void setup() {
  pinMode(SW, INPUT);
  digitalWrite(SW, HIGH);

  //interrupt
  attachInterrupt(digitalPinToInterrupt(2), my_isr, FALLING);

  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(led, LOW);

  pinMode(modeLed, OUTPUT);
  pinMode(modeLed, LOW);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena, OUTPUT);

  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enb, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  analogWrite(ena, speed);
  analogWrite(enb, 100);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  s.attach(10);

  Serial.begin(9600);
  Serial.println("HX711 weighing");

  scale.begin(dataPin, clockPin);

  scale.set_scale(calibration_factor);
  scale.tare();
  Serial.println("Readings:");
}

int  sw_state;
volatile int currentTime = 0, prevTime = 0;
volatile int yTilt, xTilt;

void loop() {

  //read weight
  currentTime = millis();
  if (currentTime - prevTime >= 1000) {
    Serial.print("Reading:");
    units = scale.get_units(), 10;
    if (units < 0)
    {
      units = 0.00;
    }
    Serial.print(units);
    Serial.println(" grams");
    prevTime = currentTime;
  }

  xTilt = analogRead(VRx);
  yTilt = analogRead(VRy);
  //Serial.println(yTilt);
  sw_state = digitalRead(SW);

  if(units < 3.5){
    if (xTilt > 760) {
      if (mode == 0) {
        currentTime = millis();
        if (currentTime - prevTime >= 500) {
          angle -= step;
          if (angle < 0) {
            angle = 0;
          }
          prevTime = currentTime;
        }
      } else {
        analogWrite(enb, 100);
        digitalWrite(in3, HIGH);
        digitalWrite(in4, LOW);
      }
    } else if (xTilt < 299) {
      if (mode == 0) {
        currentTime = millis();
        if (currentTime - prevTime >= 500) {
          angle += step;
          if (angle > 180) {
            angle = 180;
          }
          prevTime = currentTime;
        }
      } else {
        analogWrite(enb, 100);
        digitalWrite(in3, LOW);
        digitalWrite(in4, HIGH);
      }
    } else {
      if (mode == 1) {
        analogWrite(enb, 0);
        digitalWrite(in3, LOW);
        digitalWrite(in4, LOW);
      }
    }
    //Serial.println(angle);
  
    s.write(angle, 5);
  
    analogWrite(ena, speed);
  
    if (yTilt > 760) {
      currentTime = millis();
      if (currentTime - prevTime > 300) {
        if (ledState == 0) {
          ledState = 1;
          //       tone(buzzer, 300);
          digitalWrite(buzzer, HIGH);
        } else {
          ledState = 0;
          //        noTone(buzzer);
          digitalWrite(buzzer, LOW);
        }
  
        prevTime = currentTime;
      }
  
      speed = 100;
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
    } else if (yTilt < 299) {
      ledState = 0;
      //    noTone(buzzer);
      digitalWrite(buzzer, LOW);
  
      speed = 100;
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    } else {
      ledState = 0;
      //    noTone(buzzer);
      digitalWrite(buzzer, LOW);
  
      speed = 0;
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
    }
  
    digitalWrite(led, ledState);
    digitalWrite(modeLed, mode);
  } else {
    digitalWrite(led, HIGH);
  }

}

void my_isr() {
  if (mode == 0) {
    mode = 1;
  } else {
    mode = 0;
  }
}
