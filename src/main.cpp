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

// Game configuration
const int MAX_SEQUENCE_LENGTH = 10;
const int PATTERN_DISPLAY_DURATION = 400;  // LED on time during pattern display
const int PATTERN_PAUSE_DURATION = 200;    // Pause between pattern elements
const int DEBOUNCE_DELAY = 20;             // Button debounce delay
const int INPUT_TIMEOUT = 10000;           // 10 second timeout for player input

byte sequence[MAX_SEQUENCE_LENGTH];
int currentSequenceSize = 0;

byte playerpattern[MAX_SEQUENCE_LENGTH];
int playerpatternindex = 0;

// Last read input for change detection
byte lastData = 0;

// Non-blocking timing variables
unsigned long lastButtonTime = 0;
unsigned long playerTurnStartTime = 0;

void playNoteForButton(byte buttonMask) {
  for (int i = 0; i < 4; i++) {
    if (buttonMask & (1 << i)) {
      tone(buzzerPin, notes[i], 200); 
      break;
    }
  }
}

void Output_74HC595(byte data) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, data);
    digitalWrite(latchPin, HIGH);
    delayMicroseconds(10);  // Reduced from 10ms to 10us - sufficient for most shift registers
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
  delay(1000); // Brief pause before showing pattern
  
  for (int i = 0; i < currentSequenceSize; i++) {
    // Display LED
    Output_74HC595(sequence[i]);
    playNoteForButton(sequence[i]);
    delay(PATTERN_DISPLAY_DURATION);
    
    // Turn off LED
    Output_74HC595(0x00);
    noTone(buzzerPin);
    delay(PATTERN_PAUSE_DURATION);
  }
}

bool isValidSingleInput(byte value) {
  // Only accept inputs where exactly one bit is set: 0x01, 0x02, 0x04, 0x08
  return value == 0x01 || value == 0x02 || value == 0x04 || value == 0x08;
}

byte ReadButtonsWithDebounce() {
  byte currentData = Input_74HC165();
  
  // Only process if data changed and enough time has passed (debounce)
  if (currentData != lastData && (millis() - lastButtonTime) > DEBOUNCE_DELAY) {
    lastData = currentData;                                                                                                                                                                                                                          
    lastButtonTime = millis();
    
    // Provide immediate visual and audio feedback for valid single button presses
    if (isValidSingleInput(currentData)) {
      Output_74HC595(currentData);
      playNoteForButton(currentData);
    }
    
    return currentData;
  }

  return lastData; // Return last stable reading
}

bool PlayerTurn() {
  // Clear the playerpattern array
  memset(playerpattern, 0, sizeof(playerpattern));  // More efficient than loop

  delay(500); // Shorter pause before accepting input
  playerpatternindex = 0;
  playerTurnStartTime = millis();

  // Read inputs until the full sequence is entered or timeout
  while (playerpatternindex < currentSequenceSize) {
    // Check for timeout
    if (millis() - playerTurnStartTime > INPUT_TIMEOUT) {
      return false; // Timeout - treat as incorrect
    }
    
    byte playerInput = ReadButtonsWithDebounce();

    if (playerInput != 0x00 && isValidSingleInput(playerInput)) {
      playerpattern[playerpatternindex] = playerInput;
      playerpatternindex++;

      // Wait for button release to avoid double registration
      while (Input_74HC165() != 0x00) {
        delay(10);
        // Check timeout even while waiting for release
        if (millis() - playerTurnStartTime > INPUT_TIMEOUT) {
          return false;
        }
      }
      
      // Turn off LED after button release
      Output_74HC595(0x00);
      noTone(buzzerPin);
    }

    delay(10); // Reduced delay between checks for better responsiveness
  }
  
  return true; // Successfully completed input
}

bool compareArrays(const byte* a, const byte* b, int len) {
  return memcmp(a, b, len) == 0;  // More efficient than manual loop
}

void ResetPattern() {
  currentSequenceSize = 4;  // Always start with 4 elements
  for (int i = 0; i < currentSequenceSize; i++) {
    sequence[i] = RandomColorGenerator();
  }
}

void Win() {
  // Correct sequence - victory feedback
  for (int i = 0; i < 3; i++) {  // Flash green 3 times
    Output_74HC595(0x04); // Green LED 
    tone(buzzerPin, 523, 200); // Higher victory tone
    delay(200);
    Output_74HC595(0x00);
    noTone(buzzerPin);
    delay(100);
  }

  if (currentSequenceSize >= MAX_SEQUENCE_LENGTH) {
    // Player completed maximum sequence - full reset
    ResetPattern();
  } else {
    // Add one more element to sequence
    sequence[currentSequenceSize] = RandomColorGenerator();
    currentSequenceSize++;
  }
}

void Loose() {
  // Incorrect sequence - game over feedback
  for (int i = 0; i < 5; i++) {  // Flash red 5 times
    Output_74HC595(0x01); // Red LED 
    tone(buzzerPin, 131, 150); // Lower failure tone
    delay(150);
    Output_74HC595(0x00);
    noTone(buzzerPin);
    delay(100);
  }

  ResetPattern();
}

void setup() {
  Serial.begin(115200);
  
  // Configure 74HC595 pins
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // Shared clock pin
  pinMode(clockPin, OUTPUT);

  // Configure 74HC165 pins
  pinMode(shiftLoadPin, OUTPUT);
  pinMode(QHPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Initialize random generator with better entropy
  randomSeed(analogRead(0) + micros());

  // Initialize game with starting pattern
  ResetPattern();
  
  // Clear any initial state
  Output_74HC595(0x00);
  lastData = Input_74HC165();
}

void loop() {
  ShowPattern();
  
  bool inputSuccess = PlayerTurn();
  
  if (inputSuccess && compareArrays(sequence, playerpattern, currentSequenceSize)) {
    Win();
  } else {
    Loose();
  }

  delay(1000); // Pause before next round
}