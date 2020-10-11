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

    for (int i = 0; i < 5; i++) {
        pio_output_high(cols[i]);
    }
}

static void placeShip(uint8_t *mask, uint8_t *position, uint8_t length)
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
                mask[i] |= (1 << (*position / 5));
            }
        } else {
            for (int i = (*position % 5) - 1; i < length + (*position % 5) - 1; i++) {
                mask[i] |= (1 << (*position / 5));
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
                mask[(*position - 128) % 5] |= (1 << i);
            }
        } else {
            for (int i = ((*position - 128) / 5) - 1; i < length + ((*position - 128) / 5) - 1; i++) {
                mask[(*position - 128) % 5] |= (1 << i);
            }
        }
    }
}

static void moveShipUp(uint8_t *mask, uint8_t *position)
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

static void moveShipDown(uint8_t *mask, uint8_t *position)
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

static void moveShipLeft(uint8_t *mask, uint8_t *position)
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

static void moveShipRight(uint8_t *mask, uint8_t *position)
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

static char checkFrameCollision(uint8_t *mask1, uint8_t *mask2)
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


char setupPlayerOrder(void)
{
    char playerNum = 0;

    if (ir_uart_read_ready_p()) {
        playerNum = ir_uart_getc();

        ir_uart_putc(0);
    }

    if (playerNum == 0) {
        ir_uart_putc(1);

        while (1) {
            if (ir_uart_read_ready_p()) {
                if (ir_uart_getc() == 0) {
                    break;
                }
            }
        }
    }

    return playerNum;
}


void shoot(uint8_t *shotRow, uint8_t *shotCol, uint8_t *shotMask)
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


        display_column(shotMask[currentMaskDisplayColumn], currentMaskDisplayColumn);
        clearScreen();

        if ((shotMask[currentCol] >> currentRow) & 1) {
            led_set(0, 0);
        } else {
            led_set(0, 1);
        }

    }
}


void sendPos(uint8_t *shotRow, uint8_t *shotCol)
{
    ir_uart_putc(*shotRow * 5 + *shotCol);
}


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


void waitHitConfirmation(uint8_t* win)
{
    while (1) {
        if (ir_uart_read_ready_p()) {
            tinygl_clear();
            if (ir_uart_getc() == 1) {
                displayText("HIT!");
                *win = 1;
            } else {
                displayText("MISS");
            }
            break;
        }
    }
}


void waitTurn(uint8_t *shotRow, uint8_t *shotCol)
{
    uint8_t rowCol = 0;
    while (1) {
        if (ir_uart_read_ready_p()) {
            rowCol = ir_uart_getc();
            break;
        }
    }
    *shotRow = rowCol / 5;
    *shotCol = rowCol % 5;
}


void checkHit(uint8_t *shotRow, uint8_t *shotCol, uint8_t *shipMask)
{
    ir_uart_putc((shipMask[*shotCol] >> *shotRow) & 1);
}


void changePlayerNum(uint8_t *playerNum)
{
    if (*playerNum == 0) {
        *playerNum = 1;
    } else {
        *playerNum = 0;
    }
}


int main(void)
{
    led_init();
    navswitch_init();
    pio_config_set(BUTTON_PIO, PIO_INPUT);
    ir_uart_init();

    // ships
    uint8_t shipMask[5] = {0, 0, 0, 0, 0};

    uint8_t shotMask[5] = {0, 0, 0, 0, 0};

    movePlaceShip(4, shipMask);
    movePlaceShip(3, shipMask);
    movePlaceShip(2, shipMask);

    // below onwards, not tested properly yet

    uint8_t playerNum = setupPlayerOrder();

    // TEST
    //uint8_t playerNum = 0;

    uint8_t shotRow = 0;
    uint8_t shotCol = 0;

    uint8_t win = 0;

    // will change to a "end game" condition rather than infinite loop
    while (win == 0) {
        clearScreen();
        if (playerNum == 0) {
            shoot(&shotRow, &shotCol, shotMask);
            shotMask[shotCol] |= (1 << shotRow);
            sendPos(&shotRow, &shotCol);
            waitHitConfirmation(&win);
        } else {
            waitTurn(&shotRow, &shotCol);
            checkHit(&shotRow, &shotCol, shipMask);
        }
        changePlayerNum(&playerNum);
    }

    if (playerNum == 1) {
        displayText("WIN!");
    } else {
        displayText("LOSS!");
    }
}
