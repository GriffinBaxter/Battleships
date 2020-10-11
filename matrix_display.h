#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include "system.h"
#include "pio.h"

#define BUTTON_PIO PD7_PIO

extern const pio_t rows[];

extern const pio_t cols[];

void display_column(uint8_t row_pattern, uint8_t current_column);

void clearScreen(void);

void displayText(char *text);

#endif
