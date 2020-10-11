/**
 * @file   game.h
 * @author Griffin Baxter (grb96), Mitchell Veale (mjv43)
 * @date   11 Oct 2020
 * @brief  The main module of the game, responsible for the main loop
 * of the game including setup and turn-by-turn shooting of
 * battleships.
 */

#include "system.h"

/** Constant for the number of hits required to win the game. */
#define NUM_HIT_WIN 9

/** Constants for the player numbers depending on the current turn. */
#define SHOOTING_PLAYER_NUM 0
#define WAITING_PLAYER_NUM 1

/** Macro for getting a single integer from row and column values. */
#define singleIntRowCol *shotRow * 5 + *shotCol

/**
 * Macro for checking if one of the current player's ships was hit by
 * the other player, generates 1 if hit, 0 otherwise.
 */
#define isHit (shipMask[*shotCol] >> *shotRow) & 1

/**
 * Macros for updating the masks/outlines of previous shots by yourself
 * or by ships hit by the other player.
 */
#define updateShotMask shotMask[shotCol] |= (1 << shotRow)
#define updateEnemyShotMask enemyShotMask[shotCol] |= (1 << shotRow)

/** 
 * Function for setting the player order of the main game loop, the
 * first funkit to reach this function (by completing the placing of
 * ships) will be player 0 and therefore go first, otherwise the funkit
 * will be player 1.
 * @return resulting player number.
 */
char setupPlayerOrder(void);

/**
 * Function for sending the row and column position of a shot to the
 * other funkit using a single integer.
 * @param shotRow pointer to value of shot row
 * @param shotCol pointer to value of shot column
 */
void sendPos(uint8_t *shotRow, uint8_t *shotCol);

/**
 * Function for checking whether one of the placed ships have been hit,
 * sending a 1 to the other funkit if there was a hit, and 0 otherwise.
 * Additionally, if a ship was hit, the enemy hit counter is incremented
 * by 1.
 * @param shotRow pointer to value of shot row
 * @param shotCol pointer to value of shot column
 * @param shipMask bitmask of the player's ships on the display
 * @param enemyHits pointer to value of successful enemy hits
 */
void checkHit(uint8_t* shotRow, uint8_t* shotCol, uint8_t* shipMask, uint8_t* enemyHits);

/**
 * Function for changing the player number, simply swaps from 0 to 1, or
 * 1 to 0.
 * @param playerNum pointer to the value of the player's number
 */
void changePlayerNum(uint8_t *playerNum);

/**
 * Main loop of the game, this initialises various funkit drivers, sets
 * variables and masks for positions of ships and shots, creates the
 * loop for the turn-by-turn shooting of ships, and ends the game when
 * one of the players has won.
 * @return program exit code
 */
int main(void);
