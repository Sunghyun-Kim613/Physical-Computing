#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define F_CPU 16000000UL // 16 MHz clock speed

const int col[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const int row[8] = {10, 11, 12, 13, A2, A3, A4, A5};

const uint8_t letterO[8] PROGMEM = {
  0b01111110,
  0b11100111,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11000011,
  0b11100111,
  0b01111110
};

const uint8_t letterK[8] PROGMEM = {
  0b01100011,
  0b01100110,
  0b01101100,
  0b01111000,
  0b01101100,
  0b01100110,
  0b01100011,
  0b01100001
};

uint8_t displayBuffer[8][24] = {0};

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
      displayBuffer[i][j+8] = pgm_read_byte(&letterO[i]) & (1 << (7-j)) ? 1 : 0;
      displayBuffer[i][j+16] = pgm_read_byte(&letterK[i]) & (1 << (7-j)) ? 1 : 0;
    }
  }
}

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
    
    _delay_us(80);  // delay letter movement
    
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
    for (int j = 0; j < 23; j++) {
      displayBuffer[i][j] = displayBuffer[i][j + 1];
    }
    displayBuffer[i][23] = 0;
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
    _delay_ms(20);  // wait
  }
  
  return 0;
}