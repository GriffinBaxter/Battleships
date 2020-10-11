#include "targeting.h"
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


void viewShips(uint8_t* shipMask, uint8_t* enemyHitMask) {
    pacer_init(500);
    uint8_t current_column = 0;
    clearScreen();

    while (1){
        pacer_wait();
        clearScreen();

        // display dead bits of ship
        display_column(shipMask[current_column] & enemyHitMask[current_column], current_column);
        clearScreen();

        current_column++;
        if (current_column > 4) {
            current_column = 0;
        }

        button_update();
        if (button_push_event_p(0)){
            clearScreen();
            break;
        }

        // display alive bits of ship
        display_column(shipMask[current_column] & ~(enemyHitMask[current_column]), current_column);
    }
}


void shoot(uint8_t *shotRow, uint8_t *shotCol, uint8_t *shotMask, uint8_t* shipMask, uint8_t* enemyHitMask)
{
    uint8_t currentRow = 6;
    uint8_t currentCol = 0;
    uint8_t currentMaskDisplayColumn = 0;

    ledmat_init();
    pacer_init(500);

    while (1) {

        pio_output_low(rows[currentRow]);
        pio_output_low(cols[currentCol]);

        pacer_wait();

        clearScreen();

        currentMaskDisplayColumn++;
        if (currentMaskDisplayColumn > 4) {
            currentMaskDisplayColumn = 0;
        }

        navswitch_update();

        if (navswitch_push_event_p(NAVSWITCH_NORTH)) {
            if (currentRow != 0) {
                pio_output_high(rows[currentRow]);
                currentRow--;
            }
        }

        if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
            if (currentRow != 6) {
                pio_output_high(rows[currentRow]);
                currentRow++;
            }
        }

        if (navswitch_push_event_p(NAVSWITCH_EAST)) {
            if (currentCol != 4) {
                pio_output_high(cols[currentCol]);
                currentCol++;
            }
        }

        if (navswitch_push_event_p(NAVSWITCH_WEST)) {
            if (currentCol != 0) {
                pio_output_high(cols[currentCol]);
                currentCol--;
            }
        }

        if (navswitch_push_event_p(NAVSWITCH_PUSH) && !((shotMask[currentCol] >> currentRow) & 1)) {
            pio_output_high(rows[currentRow]);
            pio_output_high(cols[currentCol]);

            *shotRow = currentRow;
            *shotCol = currentCol;

            break;
        }

        button_update();
        if (button_push_event_p(0)){
            viewShips(shipMask, enemyHitMask);
        }

        display_column(shotMask[currentMaskDisplayColumn], currentMaskDisplayColumn);
        clearScreen();

        if ((shotMask[currentCol] >> currentRow) & 1) {
            led_set(0, 0);
        } else {
            led_set(0, 1);
        }

    }
}
