#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>

// col and row
const uint8_t col[8] = {2, 3, 4, 5, 6, 7, 8, 9};   // D2~7, D0~1
const uint8_t row[8] = {10, 11, 12, 13, A2, A3, A4, A5}; // A2~5, A2~5

// left and right button
#define left  PC0  // A0
#define right PC1  // A1

// player opsition
uint8_t player = 3;
// obstacles
uint8_t obstacles[8] = {0};

// score
uint16_t score = 0;

// game over -> print score -> press button -> play again (making)
uint8_t gameOver = 0;

// letters
const uint8_t S_shape[8] PROGMEM = {
    0b00111100,
    0b01100110,
    0b01100000,
    0b00111100,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00000000
};

const uint8_t C_shape[8] PROGMEM = {
    0b00111100,
    0b01100110,
    0b01100000,
    0b01100000,
    0b01100000,
    0b01100110,
    0b00111100,
    0b00000000
};

const uint8_t O_shape[8] PROGMEM = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};

const uint8_t R_shape[8] PROGMEM = {
    0b01111100,
    0b01100110,
    0b01100110,
    0b01111100,
    0b01101100,
    0b01100110,
    0b01100110,
    0b00000000
};

const uint8_t E_shape[8] PROGMEM = {
    0b01111110,
    0b01100000,
    0b01100000,
    0b01111100,
    0b01100000,
    0b01100000,
    0b01111110,
    0b00000000
};

// numbers
const uint8_t num_shapes[10][8] PROGMEM = {
    {0b00111100, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b00111100, 0b00000000},  // 0
    {0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00111100, 0b00000000},  // 1
    {0b00111100, 0b01100110, 0b00000110, 0b00001100, 0b00110000, 0b01100000, 0b01111110, 0b00000000},  // 2
    {0b00111100, 0b01100110, 0b00000110, 0b00011100, 0b00000110, 0b01100110, 0b00111100, 0b00000000},  // 3
    {0b00001100, 0b00011100, 0b00111100, 0b01101100, 0b01111110, 0b00001100, 0b00001100, 0b00000000},  // 4
    {0b01111110, 0b01100000, 0b01111100, 0b00000110, 0b00000110, 0b01100110, 0b00111100, 0b00000000},  // 5
    {0b00111100, 0b01100110, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},  // 6
    {0b01111110, 0b00000110, 0b00001100, 0b00011000, 0b00110000, 0b00110000, 0b00110000, 0b00000000},  // 7
    {0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},  // 8
    {0b00111100, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b01100110, 0b00111100, 0b00000000}   // 9
};

// display buffer
uint8_t displayBuffer[8][80];

void setup() {
    // row
    DDRD |= 0b11111100;
    DDRB |= 0b00000011;
    
    // col
    DDRB |= 0b00111100;
    DDRC |= 0b00111100;

    // button
    DDRC &= ~((1 << left) | (1 << right));
    PORTC |= (1 << left) | (1 << right);  // 풀업 저항 활성화

    // 난수 시드 설정
    srand(ADC);  // ADC 값을 시드로 사용
}

void setColumn(uint8_t colMask) {
    PORTD = (PORTD & 0b00000011) | (colMask << 2);
    PORTB = (PORTB & 0b11111100) | (colMask >> 6);
}

void setRow(uint8_t rowMask) {
    PORTB = (PORTB & 0b11000011) | ((rowMask & 0b00001111) << 2);
    PORTC = (PORTC & 0b11000011) | ((rowMask & 0b11110000) >> 2);
}

void showDisplay(int startCol) {
    for (uint8_t row_idx = 0; row_idx < 8; row_idx++) {
        setColumn(0xFF);  // off row
        uint8_t rowPattern = 0;

        for (uint8_t col_idx = 0; col_idx < 8; col_idx++) {
            if (!gameOver) {
                if (row_idx == 7 && col_idx == player) {
                    rowPattern |= (1 << col_idx);  // player move
                } else if (obstacles[row_idx] & (1 << col_idx)) {
                    rowPattern |= (1 << col_idx);  // obstacle
                }
            } else {
                if (displayBuffer[row_idx][startCol + col_idx]) {
                    rowPattern |= (1 << col_idx);
                }
            }
        }

        setRow(1 << row_idx);
        setColumn(~rowPattern);
        _delay_us(500);
    }
}

void updateGame() {
    // move
    if (!(PINC & (1 << left)) && player > 0) {
        player--;
    }
    if (!(PINC & (1 << right)) && player < 7) {
        player++;
    }

    // obstalce make and move
    for (int i = 7; i > 0; i--) {
        obstacles[i] = obstacles[i-1];
    }
    obstacles[0] = 0;
    if (rand() % 3 == 0) {
        obstacles[0] |= (1 << (rand() % 8));
        score++;
    }

    // hit -> game over
    if (obstacles[7] & (1 << player)) {
        gameOver = 1;
    }
}

void scrollLeft() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 79; j++) {
            displayBuffer[i][j] = displayBuffer[i][j + 1];
        }
        displayBuffer[i][79] = 0;
    }
}

void updateScoreDisplay() {
    // reset
    memset(displayBuffer, 0, sizeof(displayBuffer));

    // score
    const uint8_t* scoreText[5] = {S_shape, C_shape, O_shape, R_shape, E_shape};
    for (int i = 0; i < 5; i++) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                displayBuffer[row][i*8 + col] = (pgm_read_byte(&scoreText[i][row]) & (0x80 >> col)) ? 1 : 0; // based on condition, choose one
            }
        }
    }

    // show score
    int scoreTemp = score;
    int digitPos = 5;
    do {
        int digit = scoreTemp % 10;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                displayBuffer[row][(digitPos*8) + col] = (pgm_read_byte(&num_shapes[digit][row]) & (0x80 >> col)) ? 1 : 0;
            }
        }
        digitPos++;
        scoreTemp /= 10;
    } while (scoreTemp > 0);
}

uint8_t checkButtonPress() {
    return (!(PINC & ((1 << left) | (1 << right))));
}

void showScore() {
    updateScoreDisplay();
    for (int scroll = 0; scroll < 80; scroll++) {
        for (int repeat = 0; repeat < 25; repeat++) {
            showDisplay(scroll);
            // button -> game reset
            if (checkButtonPress()) {
                return;  
            }
        }
        scrollLeft();
        _delay_ms(5);
    }
}

void resetGame() {
    player = 3;
    memset(obstacles, 0, sizeof(obstacles));
    score = 0;
    gameOver = 0;
}

int main(void) {
    setup();

    while (1) {
        if (!gameOver) {
            updateGame();
            showDisplay(0);
            _delay_ms(100);
        } else {
            showScore();
            resetGame();
        }
    }

    return 0;
}