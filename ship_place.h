/**
 * @file   ship_place.h
 * @author Griffin Baxter, Mitchell Veale
 * @date   11 Oct 2020
 * @brief  Module responsible for functions regarding the moving and
 * placement of ships during the setup phase of the game.
 */

#ifndef SHIP_PLACE_H
#define SHIP_PLACE_H

#include "system.h"

#define SHIP1_LENGTH 4
#define SHIP2_LENGTH 3
#define SHIP3_LENGTH 2

#define IS_VERTICAL(X) ((X) >> 7)
#define ROTATION_BIT 128

#define BIT_LEFT(X) (1 << (X))

/**
 * Places ship of given length and given position in given bitmask, ensuring that the given position is valid and would
 * not result in the ship being placed of screen by modifying the position as needed
 * @param mask the bitmask the place the ship into
 * @param position the position to place the ship, this also stores the rotation value
 * @param length the length of the ship to place
 */
void placeShip(uint8_t *mask, uint8_t *position, uint8_t length);

/**
 * Moves the all pixels in the given bitmask up, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipUp(uint8_t *mask, uint8_t *position);

/**
 * Moves the all pixels in the given bitmask down, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipDown(uint8_t *mask, uint8_t *position);

/**
 * Moves the all pixels in the given bitmask left, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipLeft(uint8_t *mask, uint8_t *position);

/**
 * Moves the all pixels in the given bitmask right, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipRight(uint8_t *mask, uint8_t *position);

/**
 * Compares two bitmasks and determines if there is a collision between them
 * @param mask1 the first mask to compare
 * @param mask2 the second mask to compare
 * @return 1 if a collision is detected else 0
 */
char checkBitmaskCollision(uint8_t *mask1, uint8_t *mask2);

/**
 * Renders the ship the user is placing as well as previously placed ships and allows the user to move the current ship
 * and place is as long as it is not overlapping an existing ship
 * @param shipLength the length of the ship being placed
 * @param shipMask the bitmask of the previously placed ships
 */
void movePlaceShip(uint8_t shipLength, uint8_t *shipMask);

#endif
