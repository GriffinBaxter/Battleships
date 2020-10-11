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

#include "ship_place.h"
#include "matrix_display.h"
#include "targeting.h"

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
