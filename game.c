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
#include "shipPlace.h"

#define BUTTON_PIO PD7_PIO

#define SHIP1_LENGTH 4
#define SHIP2_LENGTH 3
#define SHIP3_LENGTH 2

#define NUM_HIT_WIN 9


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

void viewShips(uint8_t* shipMask, uint8_t* enemyHitMask){
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


void waitHitConfirmation(uint8_t* numHits)
{
    while (1) {
        if (ir_uart_read_ready_p()) {
            tinygl_clear();
            if (ir_uart_getc() == 1) {
                displayText("HIT!");
                *numHits += 1;
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


void checkHit(uint8_t* shotRow, uint8_t* shotCol, uint8_t* shipMask, uint8_t* enemyHits)
{
    uint8_t hit = (shipMask[*shotCol] >> *shotRow) & 1;
    ir_uart_putc(hit);
    if (hit) {
        *enemyHits += 1;
    }
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
    button_init();
    pio_config_set(BUTTON_PIO, PIO_INPUT);
    ir_uart_init();

    // ships
    uint8_t shipMask[5] = {0, 0, 0, 0, 0};

    uint8_t shotMask[5] = {0, 0, 0, 0, 0};
    uint8_t enemyShotMask[5] = {0, 0, 0, 0, 0};

    movePlaceShip(SHIP1_LENGTH, shipMask);
    movePlaceShip(SHIP2_LENGTH, shipMask);
    movePlaceShip(SHIP3_LENGTH, shipMask);

    // below onwards, not tested properly yet

    uint8_t playerNum = setupPlayerOrder();

    // TEST
    //uint8_t playerNum = 0;

    uint8_t shotRow = 0;
    uint8_t shotCol = 0;

    uint8_t numHits = 0;
    uint8_t enemyHits = 0;

    // will change to a "end game" condition rather than infinite loop
    while (numHits != NUM_HIT_WIN && enemyHits != NUM_HIT_WIN) {
        clearScreen();
        if (playerNum == 0) {
            shoot(&shotRow, &shotCol, shotMask, shipMask, enemyShotMask);
            shotMask[shotCol] |= (1 << shotRow);
            sendPos(&shotRow, &shotCol);
            waitHitConfirmation(&numHits);
        } else {
            waitTurn(&shotRow, &shotCol);
            enemyShotMask[shotCol] |= (1 << shotRow);
            checkHit(&shotRow, &shotCol, shipMask, &enemyHits);
        }
        changePlayerNum(&playerNum);
    }

    if (numHits == NUM_HIT_WIN) {
        displayText("WIN!");
    } else {
        displayText("LOSS!");
    }
}
