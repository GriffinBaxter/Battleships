#include "system.h"
#include "navswitch.h"
#include "ledmat.h"
#include "led.h"
#include "pacer.h"
#include "pio.h"
#include "button.h"
#include "ir_uart.h"

#define BUTTON_PIO PD7_PIO

/** Define PIO pins driving LED matrix rows.  */
static const pio_t rows[] = {
        LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO,
        LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
        LEDMAT_ROW7_PIO
};


/** Define PIO pins driving LED matrix columns.  */
static const pio_t cols[] = {
        LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
        LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};

static void display_column(uint8_t row_pattern, uint8_t current_column)
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

static void clearScreen(void)
{
    for (int i = 0; i < 7; i++) {
        pio_output_high(rows[i]);
    }
}

static void placeShip(uint8_t *frame, uint8_t *position, uint8_t length)
{
    //ship is horizontal
    if (!(*position >> 7)) {
        // is the ship out of the left bound?
        if ((length == 3 || length == 4) && *position % 5 == 0) {
            *position += 1;
        } else if ((length == 2 || length == 3) && *position % 5 == 4) {
            *position -= 1;
        } else if (length == 4 && *position % 5 >= 3) {
            *position -= 3 - (5 - (*position % 5));
        }
        if (length == 2) {
            for (int i = *position % 5; i < length + (*position % 5); i++) {
                frame[i] |= (1 << (*position / 5));
            }
        } else {
            for (int i = (*position % 5) - 1; i < length + (*position % 5) - 1; i++) {
                frame[i] |= (1 << (*position / 5));
            }
        }
    } else { /* Ship is vertical */
        // is the ship out of the top bound
        if ((length == 3 || length == 4) && *position - 128 < 5) {
            *position += 5;
        } else if ((length == 2 || length == 3) && *position - 128 > 29) {
            *position -= 5;
        } else if (length == 4 && *position - 128 > 24) {
            *position -= 5 * (3 - (7 - ((*position - 128) / 5)));
        }
        if (length == 2) {
            for (int i = (*position - 128) / 5; i < ((*position - 128) / 5) + length; i++) {
                frame[(*position - 128) % 5] |= (1 << i);
            }
        } else {
            for (int i = ((*position - 128) / 5) - 1; i < length + ((*position - 128) / 5) - 1; i++) {
                frame[(*position - 128) % 5] |= (1 << i);
            }
        }
    }
}

static void moveShipUp(uint8_t *frame, uint8_t *position)
{
    // first check if we can move the ship
    for (int i = 0; i < 5; i++) {
        if (frame[i] & 1) {
            return;
        }
    }

    *position -= 5;
    for (int i = 0; i < 5; i++) {
        frame[i] >>= 1;
    }
}

static void moveShipDown(uint8_t *frame, uint8_t *position)
{
    // first check if we can move the ship
    for (int i = 0; i < 5; i++) {
        if (frame[i] & (1 << 6)) {
            return;
        }
    }

    *position += 5;
    for (int i = 0; i < 5; i++) {
        frame[i] <<= 1;
    }
}

static void moveShipLeft(uint8_t *frame, uint8_t *position)
{
    // first check if we can move the ship
    if (frame[0] != 0) {
        return;
    }

    *position -= 1;
    for (int i = 1; i < 5; i++) {
        frame[i - 1] = frame[i];
    }
    frame[4] = 0;
}

static void moveShipRight(uint8_t *frame, uint8_t *position)
{
    // first check if we can move the ship
    if (frame[4] != 0) {
        return;
    }

    *position += 1;
    for (int i = 4; i > 0; i--) {
        frame[i] = frame[i - 1];
    }
    frame[0] = 0;
}

static char checkFrameCollision(uint8_t *frame1, uint8_t *frame2)
{
    for (int i = 0; i < 5; i++) {
        if ((frame1[i] & frame2[i]) != 0)
            return 1;
    }
    return 0;
}


uint8_t movePlaceShip(int shipLength, uint8_t *shipFrame)
{
    ledmat_init();
    pacer_init(500);

    //led_set(0, 1);
    uint8_t current_column = 0;

    //bright frame
    uint8_t frame2[5] = {0, 0, 0, 0, 0};

    // first bit is for rotation, second is 0, rest is location
    uint8_t shipPosition = 0b00011110;

    // TODO: Figure out a better name for this
    uint8_t frameNumber = 0;
    // first battle ship
    placeShip(frame2, &shipPosition, shipLength);

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
            moveShipUp(frame2, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
            moveShipDown(frame2, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_EAST)) {
            moveShipRight(frame2, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_WEST)) {
            moveShipLeft(frame2, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            shipPosition ^= 1 << 7;
            for (int i = 0; i < 5; i++) {
                frame2[i] = 0;
            }
            placeShip(frame2, &shipPosition, shipLength);
        }

        // Display shipFrame for a split second so that it is very dim
        display_column(shipFrame[current_column], current_column);
        clearScreen();

        display_column(frame2[current_column], current_column);

        if (checkFrameCollision(shipFrame, frame2))
            led_set(0, 0);
        else
            led_set(0, 1);

        if (pio_input_get(BUTTON_PIO) && !checkFrameCollision(shipFrame, frame2)) {
            for (int i = 0; i < 5; i++) {
                shipFrame[i] |= frame2[i];
            }
            break;
        }
    }

    return shipPosition;
}


int main(void)
{
    led_init();
    navswitch_init();
    pio_config_set(BUTTON_PIO, PIO_INPUT);
    ir_uart_init();

    // ships
    uint8_t frame1[5] = {0, 0, 0, 0, 0};

    uint8_t shipPosition4 = movePlaceShip(4, frame1);
    uint8_t shipPosition3 = movePlaceShip(3, frame1);
    uint8_t shipPosition2 = movePlaceShip(2, frame1);

    uint8_t enemyShipPosition4 = 0;

    // send and retrieve the first ship position, not working correctly or tested yet
    while (1) {
        ir_uart_putc((char) shipPosition4);

        if (ir_uart_read_ready_p()) {
            enemyShipPosition4 = ir_uart_getc();
            break;
        }
    }
}
