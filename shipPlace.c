#include "shipPlace.h"

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

#define BUTTON_PIO PD7_PIO

#define SHIP1_LENGTH 4
#define SHIP2_LENGTH 3
#define SHIP3_LENGTH 2

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

void placeShip(uint8_t *mask, uint8_t *position, uint8_t length)
{
    //ship is horizontal
    if (!(*position >> 7)) {
        // is the ship out of the left bound?
        if ((length == SHIP2_LENGTH || length == SHIP1_LENGTH) && *position % 5 == 0) {
            *position += 1;
        } else if ((length == SHIP3_LENGTH || length == SHIP2_LENGTH) && *position % 5 == 4) {
            *position -= 1;
        } else if (length == SHIP3_LENGTH && *position % 5 >= 3) {
            *position -= 3 - (5 - (*position % 5));
        }
        if (length == 2) {
            for (int i = *position % 5; i < length + (*position % 5); i++) {
                mask[i] |= (1 << (*position / 5));
            }
        } else {
            for (int i = (*position % 5) - 1; i < length + (*position % 5) - 1; i++) {
                mask[i] |= (1 << (*position / 5));
            }
        }
    } else { /* Ship is vertical */
        // is the ship out of the top bound
        if ((length == SHIP2_LENGTH || length == SHIP1_LENGTH) && *position - 128 < 5) {
            *position += 5;
        } else if ((length == SHIP3_LENGTH || length == SHIP2_LENGTH) && *position - 128 > 29) {
            *position -= 5;
        } else if (length == SHIP1_LENGTH && *position - 128 > 24) {
            *position -= 5 * (3 - (7 - ((*position - 128) / 5)));
        }
        if (length == 2) {
            for (int i = (*position - 128) / 5; i < ((*position - 128) / 5) + length; i++) {
                mask[(*position - 128) % 5] |= (1 << i);
            }
        } else {
            for (int i = ((*position - 128) / 5) - 1; i < length + ((*position - 128) / 5) - 1; i++) {
                mask[(*position - 128) % 5] |= (1 << i);
            }
        }
    }
}

void moveShipUp(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    for (int i = 0; i < 5; i++) {
        if (mask[i] & 1) {
            return;
        }
    }

    *position -= 5;
    for (int i = 0; i < 5; i++) {
        mask[i] >>= 1;
    }
}

void moveShipDown(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    for (int i = 0; i < 5; i++) {
        if (mask[i] & (1 << 6)) {
            return;
        }
    }

    *position += 5;
    for (int i = 0; i < 5; i++) {
        mask[i] <<= 1;
    }
}

void moveShipLeft(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    if (mask[0] != 0) {
        return;
    }

    *position -= 1;
    for (int i = 1; i < 5; i++) {
        mask[i - 1] = mask[i];
    }
    mask[4] = 0;
}

void moveShipRight(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    if (mask[4] != 0) {
        return;
    }

    *position += 1;
    for (int i = 4; i > 0; i--) {
        mask[i] = mask[i - 1];
    }
    mask[0] = 0;
}

char checkFrameCollision(uint8_t *mask1, uint8_t *mask2)
{
    for (int i = 0; i < 5; i++) {
        if ((mask1[i] & mask2[i]) != 0) {
            return 1;
        }
    }
    return 0;
}


void movePlaceShip(uint8_t shipLength, uint8_t *shipMask)
{
    ledmat_init();
    pacer_init(500);

    //led_set(0, 1);
    uint8_t current_column = 0;

    //bright frame
    uint8_t currentShipMask[5] = {0, 0, 0, 0, 0};

    // first bit is for rotation, second is 0, rest is location
    uint8_t shipPosition = 0b00011110;

    // first battle ship
    placeShip(currentShipMask, &shipPosition, shipLength);

    while (1) {
        system_init();
        pacer_wait();

        clearScreen();
        current_column++;
        if (current_column > 4) {
            current_column = 0;
        }

        navswitch_update();
        if (navswitch_push_event_p(NAVSWITCH_NORTH)) {
            moveShipUp(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
            moveShipDown(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_EAST)) {
            moveShipRight(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_WEST)) {
            moveShipLeft(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            shipPosition ^= 1 << 7;
            for (int i = 0; i < 5; i++) {
                currentShipMask[i] = 0;
            }
            placeShip(currentShipMask, &shipPosition, shipLength);
        }

        // Display shipFrame for a split second so that it is very dim
        display_column(shipMask[current_column], current_column);
        clearScreen();

        display_column(currentShipMask[current_column], current_column);

        if (checkFrameCollision(shipMask, currentShipMask)) {
            led_set(0, 0);
        } else {
            led_set(0, 1);
        }

        if (pio_input_get(BUTTON_PIO) && !checkFrameCollision(shipMask, currentShipMask)) {
            for (int i = 0; i < 5; i++) {
                shipMask[i] |= currentShipMask[i];
            }
            break;
        }
    }
}