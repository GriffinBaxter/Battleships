#ifndef SHIP_PLACE_H
#define SHIP_PLACE_H

#include "system.h"

#define SHIP1_LENGTH 4
#define SHIP2_LENGTH 3
#define SHIP3_LENGTH 2

void placeShip(uint8_t *mask, uint8_t *position, uint8_t length);

void moveShipUp(uint8_t *mask, uint8_t *position);

void moveShipDown(uint8_t *mask, uint8_t *position);

void moveShipLeft(uint8_t *mask, uint8_t *position);

void moveShipRight(uint8_t *mask, uint8_t *position);

char checkFrameCollision(uint8_t *mask1, uint8_t *mask2);

void movePlaceShip(uint8_t shipLength, uint8_t *shipMask);

#endif
