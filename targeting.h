/**
 * @file   targeting.h
 * @author Griffin Baxter, Mitchell Veale
 * @date   11 Oct 2020
 * @brief  Module responsible for functions regarding the targeting for
 * shooting the other player's ships and viewing your own ships and
 * their currently inflicted hits.
 */

#ifndef TARGETING_H
#define TARGETING_H

#include "system.h"

/**
 * Displays the player's ships and shows what parts of ships have been sunk
 * @param shipMask bitmask of the player's ships
 * @param enemyHitMask bitmask of the shots fired by the enemy
 */
void viewShips(uint8_t* shipMask, uint8_t* enemyHitMask);

/**
 * enters targeting mode, allowing the user to select a location on the LED mat to fire on. Displays the user's previous
 * shots and disallowing them from shooting a location that they have previously fired on
 * @param shotRow pointer to the selected row
 * @param shotCol pointer to the selected column
 * @param shotMask bitmask of the player's previous shots
 * @param shipMask bitmask of the player's ships
 * @param enemyHitMask bitmask of the enemy's previous shots
 */
void shoot(uint8_t *shotRow, uint8_t *shotCol, uint8_t *shotMask, uint8_t* shipMask, uint8_t* enemyHitMask);

#endif
