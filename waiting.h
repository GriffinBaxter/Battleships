/**
 * @file   waiting.h
 * @author Griffin Baxter, Mitchell Veale
 * @date   11 Oct 2020
 * @brief  Module responsible for functions regarding waiting for the
 * other player to shoot their shot at one of your ships, and
 * subsequently displaying a hit confirmation for a fired hit.
 */

#ifndef WAITING_H
#define WAITING_H

#include "system.h"

/**
 * Waits until the UCFK receives confirmation from the other UCFK about whether the fired shot hit or missed
 * @param numHits pointer to number of successful hits, incremented if hit is confirmed
 */TOTAL_COLS
void waitHitConfirmation(uint8_t* numHits);

/**
 * Waits until other player has completed their turn by continually polling the IR Receiver for information about the
 * enemy's shot position
 * @param shotRow pointer to the value of show row
 * @param shotCol pointer to the value of shot column
 */
void waitTurn(uint8_t *shotRow, uint8_t *shotCol);

#endif
