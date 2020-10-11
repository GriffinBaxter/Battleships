#ifndef TARGETING_H
#define TARGETING_H

#include "system.h"
#include "pio.h"

void viewShips(uint8_t* shipMask, uint8_t* enemyHitMask);

void shoot(uint8_t *shotRow, uint8_t *shotCol, uint8_t *shotMask, uint8_t* shipMask, uint8_t* enemyHitMask);

#endif
