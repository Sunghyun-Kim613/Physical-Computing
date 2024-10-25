#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define F_CPU 16000000UL // 16 MHz clock speed

// matrix col and row
const uint8_t col[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const uint8_t row[8] = {10, 11, 12, 13, A2, A3, A4, A5};

// letter O
const uint8_t O_shape[8] PROGMEM = {
  0b01111110,
  0b11100111,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11100111,
  0b01111110
};

// K
const uint8_t K_shape[8] PROGMEM = {
  0b01100011,
  0b01100110,
  0b01101100,
  0b01111000,
  0b01101100,
  0b01100110,
  0b01100011,
  0b01100001
};

void setup() {
  // set output
  for (int i = 0; i < 8; i++) {
    pinMode(col[i], OUTPUT);
    digitalWrite(col[i], HIGH); // col high = off
  }
  
  for (int i = 0; i < 8; i++) {
    pinMode(row[i], OUTPUT);
    digitalWrite(row[i], LOW); // row high = off
  }
}

void displayPattern(const uint8_t pattern[8], bool inverse) {
  unsigned long startTime = millis();
  // show 1 second
  while (millis() - startTime < 1000) {
    for (int i = 0; i < 8; i++) {
      uint8_t rowPattern = pgm_read_byte(&pattern[i]);
      if (inverse) rowPattern = ~rowPattern; // inverse
      
      // row high = print
      digitalWrite(row[i], HIGH);
      
      // set row
      for (int j = 0; j < 8; j++) {
        digitalWrite(col[j], !(rowPattern & (1 << (7 - j))));
      }
      
      _delay_us(1000); // delay
      
      // row low = off
      digitalWrite(row[i], LOW);
    }
  }
}

void loop() {
  // O 
  displayPattern(O_shape, false);
  // O inverse
  displayPattern(O_shape, true);
  
  // K 
  displayPattern(K_shape, false);
  // K inverse
  displayPattern(K_shape, true);
}