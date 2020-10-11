#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H

#include "system.h"
#include "pio.h"

#define BUTTON_PIO PD7_PIO

/** Define PIO pins driving LED matrix rows.  */
extern const pio_t rows[];

/** Define PIO pins driving LED matrix columns.  */
extern const pio_t cols[];

/**
 * Displays a given pattern on a given row of the LED mat
 * @param row_pattern bit pattern to display
 * @param current_column the current column that is being displayed
 */
void display_column(uint8_t row_pattern, uint8_t current_column);

/**
 * Sets all LED mat rows and column to PIO signal high
 */
void clearScreen(void);

/**
 * Displays a scrolling string on the LED mat
 * @param text the string to display
 */
void displayText(char *text);

#endif
