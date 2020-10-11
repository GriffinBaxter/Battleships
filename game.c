#include "system.h"
#include "navswitch.h"
#include "ledmat.h"
#include "led.h"
#include "pacer.h"
#include "button.h"
#include "ir_uart.h"

#include "ship_place.h"
#include "matrix_display.h"
#include "targeting.h"
#include "waiting.h"

#define NUM_HIT_WIN 9

/**
 * Determines if this UCFK is player 0 or player 1 by communicating with another UCFK
 * @return resulting player number
 */
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

/**
 * Sends given shot position to another UCFK via the IR Transmitter
 * @param shotRow pointer to value of shot row
 * @param shotCol pointer to value of shot column
 */
void sendPos(uint8_t *shotRow, uint8_t *shotCol)
{
    ir_uart_putc(*shotRow * 5 + *shotCol);
}

/**
 * Checks the given shot row and column to deduce whether a shot has hit a ship
 * @param shotRow pointer to value of shot row
 * @param shotCol pointer to value of shot column
 * @param shipMask bitmask of the player's ships on the display
 * @param enemyHits pointer to value of successful enemy hits
 */
void checkHit(uint8_t* shotRow, uint8_t* shotCol, uint8_t* shipMask, uint8_t* enemyHits)
{
    uint8_t hit = (shipMask[*shotCol] >> *shotRow) & 1;
    ir_uart_putc(hit);
    if (hit) {
        *enemyHits += 1;
    }
}

/**
 * Toggles the player's number
 * @param playerNum pointer to the value of the player's number
 */
void changePlayerNum(uint8_t *playerNum)
{
    if (*playerNum == 0) {
        *playerNum = 1;
    } else {
        *playerNum = 0;
    }
}

/**
 * Main method. this is the entry point of the program
 * @return program exit code
 */
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

    uint8_t playerNum = setupPlayerOrder();

    uint8_t shotRow = 0;
    uint8_t shotCol = 0;

    uint8_t numHits = 0;
    uint8_t enemyHits = 0;

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
        displayText("LOSS");
    }
}
