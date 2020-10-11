#ifndef WAITING_H
#define WAITING_H

#include "system.h"
#include "pio.h"

void waitHitConfirmation(uint8_t* numHits);

void waitTurn(uint8_t *shotRow, uint8_t *shotCol);

#endif
