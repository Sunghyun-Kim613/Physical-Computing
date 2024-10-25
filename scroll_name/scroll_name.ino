#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define F_CPU 16000000UL // 16 MHz clock speed

const int col[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const int row[8] = {10, 11, 12, 13, A2, A3, A4, A5};

const uint8_t letterS[8] PROGMEM = {
  0b01111110,
  0b11000000,
  0b11000000,
  0b01111110,
  0b00000011,
  0b00000011,
  0b11000011,
  0b01111110
};

const uint8_t letterU[8] PROGMEM = {
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b01111110
};

const uint8_t letterN[8] PROGMEM = {
  0b11000011,
  0b11100011,
  0b11110011,
  0b11011011,
  0b11001111,
  0b11000111,
  0b11000011,
  0b11000011
};

const uint8_t letterG[8] PROGMEM = {
  0b01111110,
  0b11000011,
  0b11000000,
  0b11000000,
  0b11001111,
  0b11000011,
  0b11000011,
  0b01111110
};

uint8_t displayBuffer[8][32] = {0};

void setup() {
  // Set col as output
  DDRD |= 0b11111100;  // D 2~7
  DDRB |= 0b00000011;  // D 8~9
  
  // Set row as output
  DDRB |= 0b00111100;  // D10~3
  DDRC |= 0b00111100;  // A2~A5
  
  // Initialize all pins
  PORTD |= 0b11111100;  // col high = off
  PORTB |= 0b00000011;
  PORTB &= ~0b00111100; // row low = off
  PORTC &= ~0b00111100;

  // letter in buffer
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      displayBuffer[i][j] = pgm_read_byte(&letterS[i]) & (1 << (7-j)) ? 1 : 0;
      displayBuffer[i][j+8] = pgm_read_byte(&letterU[i]) & (1 << (7-j)) ? 1 : 0;
      displayBuffer[i][j+16] = pgm_read_byte(&letterN[i]) & (1 << (7-j)) ? 1 : 0;
    }
  }
}

// 나머지 함수들 (displayPattern, scrollLeft, main)은 변경 없이 그대로 유지

void displayPattern(int startCol) {
  for (int i = 0; i < 8; i++) {
    // off col
    PORTD |= 0b11111100;
    PORTB |= 0b00000011;
    
    // row
    if (i < 4) {
      PORTB |= (1 << (i + 2));
    } else {
      PORTC |= (1 << (i - 2));
    }
    
    // col with row
    for (int j = 0; j < 8; j++) {
      if (displayBuffer[i][startCol + j]) {
        if (j < 6) {
          PORTD &= ~(1 << (j + 2));
        } else {
          PORTB &= ~(1 << (j - 6));
        }
      }
    }
    
    _delay_us(100);  // delay letter movement
    
    // row off
    if (i < 4) {
      PORTB &= ~(1 << (i + 2));
    } else {
      PORTC &= ~(1 << (i - 2));
    }
  }
}

// move to left
void scrollLeft() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 32; j++) {
      displayBuffer[i][j] = displayBuffer[i][j + 1];
    }
    displayBuffer[i][32] = 0;
  }
}

int main(void) {
  setup();
  
  while (1) {
    for (int scroll = 0; scroll < 24; scroll++) {
      for (int repeat = 0; repeat < 50; repeat++) {
        displayPattern(0);
      }
      scrollLeft();
      _delay_ms(10);  // move speed
    }
    
    // reset the display buffer
    setup();
    _delay_ms(100);  // wait
  }
  
  return 0;
}