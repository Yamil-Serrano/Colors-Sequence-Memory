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

byte sequence[10];
int currentSequenceSize = 0;

byte playerpattern[10];
int playerpatternindex = 0;

// last byte
byte lastData = 0;

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

byte RandomColorGenerator() {
  // NOTE: When you write 00001000 in the code, the compiler interprets it as an octal (base 8) number 
  // due to the leading 0. So: 00001000 in octal is equivalent to 512 in decimal.
  // Since the value 512 is larger than the range of a byte (which goes from 0 to 255), 
  // it overflows and gets truncated. In contrast, when you use the hexadecimal notation like 0x08, you are explicitly specifying 
  // a base 16 number, which fits within a byte. The value 0x08 is simply 8 in decimal, which is well within the byte's range.

  int randomNumber = random(0, 4);

  switch (randomNumber) {
    case 0:
      return 0x01; // Red
    case 1:
      return 0x02; // Yellow
    case 2:
      return 0x04; // Green
    case 3:
      return 0x08; // Blue
    default:
      return 0x00; 
  }
}

void ShowPattern() {
  for (int i = 0; i < currentSequenceSize; i++) {
    Output_74HC595(sequence[i]);
    checkButtonsAndPlayNote(sequence[i]);
    delay(100);
  }
}

byte ReadButtons() {
  byte currentData = Input_74HC165();

  if (currentData != lastData) {
    Output_74HC595(currentData);
    checkButtonsAndPlayNote(currentData);
    lastData = currentData;
  }

  return currentData;
}

bool isValidSingleInput(byte value) {
  // Only accept inputs where exactly one bit is set: 0x01, 0x02, 0x04, 0x08
  return value == 0x01 || value == 0x02 || value == 0x04 || value == 0x08;
}

void PlayerTurn() {
  // Clear the playerpattern array
  for (int i = 0; i < 10; i++) {
    playerpattern[i] = 0;
  }

  delay(1000); // Short pause before accepting input
  playerpatternindex = 0;

  // Read inputs until the full sequence is entered
  while (playerpatternindex < currentSequenceSize) {
    byte playerInput = ReadButtons();

    if (playerInput != 0x00 && isValidSingleInput(playerInput)) {
      delay(10); // Debounce delay
      playerpattern[playerpatternindex] = playerInput;
      playerpatternindex++;

      // Wait for button release to avoid double registration
      while (ReadButtons() != 0x00) {
        delay(10);
      }
    }

    delay(50); // Minor delay between checks
  }
}


bool compareArrays(const byte* a, const byte* b, int len) {
  for (int i = 0; i < len; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void ResetPattern() {
  currentSequenceSize = 0;
    for (int i = 0; i < 4; i++) {
      sequence[i] = RandomColorGenerator();
      currentSequenceSize++;
    }
}

void Win() {
  // Correct sequence
    Output_74HC595(0x04); // Green LED 
    delay(1000);

    if (currentSequenceSize >= 10) {
      ResetPattern();
    }

    else {
      sequence[currentSequenceSize] = RandomColorGenerator();
      currentSequenceSize++;
    }
}

void Loose() {
  // Incorrect sequence
    Output_74HC595(0x01); // Red LED 
    delay(1000);

    ResetPattern();
}


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

  // Random generator
  randomSeed(analogRead(0));

  for(int i = 0; i < 4; i++) {
    sequence[i] = RandomColorGenerator();
    currentSequenceSize ++;
  }
}

void loop() {
  ShowPattern();
  PlayerTurn();
  if (compareArrays(sequence, playerpattern, currentSequenceSize)) {
    Win();
  } 
  
  else {
    Loose();
  }

  delay(1000); // Short pause before next round
}