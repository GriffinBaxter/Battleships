#ifndef SHIPPLACE_H
#define SHIPPLACE_H

#include "system.h"
#include "pio.h"

extern const pio_t rows[];

extern const pio_t cols[];

void display_column(uint8_t row_pattern, uint8_t current_column);

void clearScreen(void);

void placeShip(uint8_t *mask, uint8_t *position, uint8_t length);

void moveShipUp(uint8_t *mask, uint8_t *position);

void moveShipDown(uint8_t *mask, uint8_t *position);

void moveShipLeft(uint8_t *mask, uint8_t *position);

void moveShipRight(uint8_t *mask, uint8_t *position);

char checkFrameCollision(uint8_t *mask1, uint8_t *mask2);

void movePlaceShip(uint8_t shipLength, uint8_t *shipMask);

#endif
