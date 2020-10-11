/** @file   game.c
    @author Griffin Baxter, Mitchell Veale
    @date   11 Oct 2020
    @brief  The main module of the game, responsible for the main loop
    of the game including setup and turn-by-turn shooting of
    battleships.
*/

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

// Constant for the number of hits required to win the game.
#define NUM_HIT_WIN 9


/* Function for setting the player order of the main game loop, the
 * first funkit to reach this function (by completing the placing of
 * ships) will be player 0 and therefore go first, otherwise the funkit
 * will be player 1. */
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


/* Function for sending the row and column position of a shot to the
 * other funkit using a single integer. */
void sendPos(uint8_t *shotRow, uint8_t *shotCol)
{
    ir_uart_putc(*shotRow * 5 + *shotCol);
}


/* Function for checking whether one of the placed ships have been hit,
 * sending a 1 to the other funkit if there was a hit, and 0 otherwise.
 * Additionally, if a ship was hit, the enemy hit counter is incremented
 * by 1. */
void checkHit(uint8_t* shotRow, uint8_t* shotCol, uint8_t* shipMask, uint8_t* enemyHits)
{
    uint8_t hit = (shipMask[*shotCol] >> *shotRow) & 1;
    ir_uart_putc(hit);
    if (hit) {
        *enemyHits += 1;
    }
}


/* Function for changing the player number, simply swaps from 0 to 1, or
 * 1 to 0. */
void changePlayerNum(uint8_t *playerNum)
{
    if (*playerNum == 0) {
        *playerNum = 1;
    } else {
        *playerNum = 0;
    }
}


/* Main loop of the game, this initialises various funkit drivers, sets
 * variables and masks for positions of ships and shots, creates the
 * loop for the turn-by-turn shooting of ships, and ends the game when
 * one of the players has won. */
int main(void)
{
    // Setting up variables
    uint8_t shotRow = 0;
    uint8_t shotCol = 0;
    uint8_t numHits = 0;
    uint8_t enemyHits = 0;
    
    // Initialisers
    led_init();
    navswitch_init();
    button_init();
    pio_config_set(BUTTON_PIO, PIO_INPUT);
    ir_uart_init();

    // Ships
    uint8_t shipMask[5] = {0, 0, 0, 0, 0};
    uint8_t shotMask[5] = {0, 0, 0, 0, 0};
    uint8_t enemyShotMask[5] = {0, 0, 0, 0, 0};

    // Moving and placing ships
    movePlaceShip(SHIP1_LENGTH, shipMask);
    movePlaceShip(SHIP2_LENGTH, shipMask);
    movePlaceShip(SHIP3_LENGTH, shipMask);
    
    // Getting player number
    uint8_t playerNum = setupPlayerOrder();

    // Main loop
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

    // Win/loss conditions
    if (numHits == NUM_HIT_WIN) {
        displayText("WIN!");
    } else {
        displayText("LOSS");
    }
}
