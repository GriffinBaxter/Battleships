/**
 * @file   targeting.c
 * @author Griffin Baxter, Mitchell Veale
 * @date   11 Oct 2020
 * @brief  Module responsible for functions regarding the targeting for
 * shooting the other player's ships and viewing your own ships and
 * their currently inflicted hits.
 */

#include "targeting.h"
#include "matrix_display.h"
#include "navswitch.h"
#include "ledmat.h"
#include "led.h"
#include "pacer.h"
#include "button.h"
#include "ir_uart.h"


/**
 * Displays the player's ships and shows what parts of ships have been sunk
 * @param shipMask bitmask of the player's ships
 * @param enemyHitMask bitmask of the shots fired by the enemy
 */
void viewShips(uint8_t* shipMask, uint8_t* enemyHitMask) {
    pacer_init(500);
    uint8_t current_column = 0;
    clearScreen();

    while (1){
        pacer_wait();
        clearScreen();

        // display dead bits of ship
        displayColumn(shipMask[current_column] & enemyHitMask[current_column], current_column);
        clearScreen();

        current_column++;
        if (current_column > 4) {
            current_column = 0;
        }

        button_update();
        if (button_push_event_p(0)) {
            clearScreen();
            break;
        }

        // display alive bits of ship
        displayColumn(shipMask[current_column] & ~(enemyHitMask[current_column]), current_column);
    }
}

/**
 * enters targeting mode, allowing the user to select a location on the LED mat to fire on. Displays the user's previous
 * shots and disallowing them from shooting a location that they have previously fired on
 * @param shotRow pointer to the selected row
 * @param shotCol pointer to the selected column
 * @param shotMask bitmask of the player's previous shots
 * @param shipMask bitmask of the player's ships
 * @param enemyHitMask bitmask of the enemy's previous shots
 */
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

        displayColumn(shotMask[currentMaskDisplayColumn], currentMaskDisplayColumn);
        clearScreen();

        if ((shotMask[currentCol] >> currentRow) & 1) {
            led_set(0, 0);
        } else {
            led_set(0, 1);
        }

    }
}
