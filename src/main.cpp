#include <Arduino.h>

// Pins connected to the 74HC595 shift register
const int dataPin = D4;   // GPIO14 - DS
const int latchPin = D5;  // GPIO12 - STCP

// Shared clock pin
const int clockPin = D3;  // GPIO5 - SHCP (74HC595) / GPIO2 - CLK (74HC165)

// Pins connected to the 74HC165 shift register
const int shiftLoadPin = D7; // GPIO1 - SH/LD (parallel load)
const int QHPin = D8; // GPIO9 - QH (serial data out)

// Buzzer pin
const int buzzerPin = D6;  // GPIO12 - buzzer

// Notes array
const int notes[4] = { 262, 294, 330, 349 }; // Do, Re, Mi, Fa

// last byte
byte lastData = 0;

void setup() {
  // Pins connected to the 74HC595 shift register
  Serial.begin(115200);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // Shared clock pin
  pinMode(clockPin, OUTPUT);

  // Pins connected to the 74HC165 shift register
  pinMode(shiftLoadPin, OUTPUT);
  pinMode(QHPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void checkButtonsAndPlayNote(byte data) {
  for (int i = 0; i < 4; i++) {
    if (data & (1 << i)) {
      tone(buzzerPin, notes[i], 200); 
      delay(200);                     
      noTone(buzzerPin);              
      break;
    }
  }
}

void Output_74HC595(byte byte) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, byte);
    digitalWrite(latchPin, HIGH);
    delay(10);
}

byte Input_74HC165() {
  // Capture input states into shift register
    digitalWrite(shiftLoadPin, LOW);
    delayMicroseconds(5);
    digitalWrite(shiftLoadPin, HIGH);  // Latch data for serial read

    byte data = 0;
    for (int i = 7; i >= 0; i--) {
      int bit = digitalRead(QHPin);
      data |= (bit << i);
      digitalWrite(clockPin, HIGH);
      delayMicroseconds(2);
      digitalWrite(clockPin, LOW);
    }
    return data;
}

void loop() {
byte currentData = Input_74HC165();

  if (currentData != lastData) {
    Output_74HC595(currentData);
    checkButtonsAndPlayNote(currentData);
    lastData = currentData;
  }

  delay(10);
}