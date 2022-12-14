/**
 * @file   matrix_display.c
 * @author Griffin Baxter (grb96), Mitchell Veale (mjv43)
 * @date   11 Oct 2020
 * @brief  Module responsible for functions and constants regarding the
 * matrix display, which is used for displaying ships, shot markers and
 * text.
 */

#include "matrix_display.h"
#include "ledmat.h"
#include "pacer.h"
#include "tinygl.h"
#include "../../fonts/font5x7_1.h"


/** Define PIO pins driving LED matrix rows.  */
const pio_t rows[] = {
        LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO,
        LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
        LEDMAT_ROW7_PIO
};


/** Define PIO pins driving LED matrix columns.  */
const pio_t cols[] = {
        LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
        LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};

/**
 * Displays a given pattern on a given row of the LED mat
 * @param rowPattern bit pattern to display
 * @param currentColumn the current column that is being displayed
 */
void displayColumn(uint8_t rowPattern, uint8_t currentColumn)
{
    if (currentColumn == 0) {
        pio_output_high(cols[4]);
    } else {
        pio_output_high(cols[currentColumn - 1]);
    }

    if (!(rowPattern << 1)) {
        return;
    }

    for (int i = 0; i < TOTAL_ROWS; i++) {
        (rowPattern >> i) & 1 ? pio_output_low(rows[i]) : pio_output_high(rows[i]);
    }

    pio_output_low(cols[currentColumn]);
}

/**
 * Sets all LED mat rows and column to PIO signal high
 */
void clearScreen(void)
{
    for (int i = 0; i < TOTAL_ROWS; i++) {
        pio_output_high(rows[i]);
    }

    for (int i = 0; i < TOTAL_COLS; i++) {
        pio_output_high(cols[i]);
    }
}

/**
 * Displays a scrolling string on the LED mat
 * @param text the string to display
 */
void displayText(char *text)
{
    tinygl_init(500);
    tinygl_font_set(&font5x7_1);
    tinygl_text_speed_set(10);
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text(text);

    pacer_init(500);

    uint16_t count = 0;

    while (1) {
        pacer_wait();
        tinygl_update();

        if (count++ >= 2500) {
            break;
        }
    }

    clearScreen();
}
