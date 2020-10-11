#include "matrix_display.h"

#include "system.h"
#include "navswitch.h"
#include "ledmat.h"
#include "led.h"
#include "pacer.h"
#include "pio.h"
#include "button.h"
#include "ir_uart.h"
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

void display_column(uint8_t row_pattern, uint8_t current_column)
{
    if (current_column == 0) {
        pio_output_high(cols[4]);
    } else {
        pio_output_high(cols[current_column - 1]);
    }

    if (!(row_pattern << 1)) {
        return;
    }

    for (int i = 0; i < 7; i++) {
        (row_pattern >> i) & 1 ? pio_output_low(rows[i]) : pio_output_high(rows[i]);
    }

    pio_output_low(cols[current_column]);
}


void clearScreen(void)
{
    for (int i = 0; i < 7; i++) {
        pio_output_high(rows[i]);
    }

    for (int i = 0; i < 5; i++) {
        pio_output_high(cols[i]);
    }
}
