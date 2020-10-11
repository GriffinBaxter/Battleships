/**
 * @file   ship_place.c
 * @author Griffin Baxter (grb96), Mitchell Veale (mjv43)
 * @date   11 Oct 2020
 * @brief  Module responsible for functions regarding the moving and
 * placement of ships during the setup phase of the game.
 */

#include "ship_place.h"
#include "matrix_display.h"
#include "navswitch.h"
#include "ledmat.h"
#include "led.h"
#include "pacer.h"
#include "button.h"


/**
 * Places ship of given length and given position in given bitmask, ensuring that the given position is valid and would
 * not result in the ship being placed of screen by modifying the position as needed
 * @param mask the bitmask the place the ship into
 * @param position the position to place the ship, this also stores the rotation value
 * @param length the length of the ship to place
 */
void placeShip(uint8_t *mask, uint8_t *position, uint8_t length)
{
    if (!IS_VERTICAL(*position)) { // Ship is horizontal.
        
        // Checking if the ship is out of the left bound.
        if ((length == SHIP2_LENGTH || length == SHIP1_LENGTH) && *position % TOTAL_COLS == 0) {
            *position += 1;
        } else if ((length == SHIP3_LENGTH || length == SHIP2_LENGTH) && *position % TOTAL_COLS == TOTAL_COLS - 1) {
            *position -= 1;
        } else if (length == SHIP3_LENGTH && *position % TOTAL_COLS >= TOTAL_COLS - 2) {
            *position -= 3 - (TOTAL_COLS - (*position % TOTAL_COLS));
        }
        if (length == SHIP3_LENGTH) {
            for (int i = *position % TOTAL_COLS; i < length + (*position % TOTAL_COLS); i++) {
                mask[i] |= BIT_LEFT(*position / TOTAL_COLS);
            }
        } else {
            for (int i = (*position % TOTAL_COLS) - 1; i < length + (*position % TOTAL_COLS) - 1; i++) {
                mask[i] |= BIT_LEFT(*position / TOTAL_COLS);
            }
        }
    } else { // Ship is vertical.
        
        // Checking if the ship is out of the top bound.
        if ((length == SHIP2_LENGTH || length == SHIP1_LENGTH) && *position - ROTATION_BIT < TOTAL_COLS) {
            *position += TOTAL_COLS;
        } else if ((length == SHIP3_LENGTH || length == SHIP2_LENGTH) && *position - ROTATION_BIT > 29) {
            *position -= TOTAL_COLS;
        } else if (length == SHIP1_LENGTH && *position - ROTATION_BIT > 24) {
            *position -= TOTAL_COLS * (3 - (TOTAL_ROWS - ((*position - ROTATION_BIT) / TOTAL_COLS)));
        }
        if (length == SHIP3_LENGTH) {
            for (int i = (*position - ROTATION_BIT) / TOTAL_COLS; i < ((*position - ROTATION_BIT) / TOTAL_COLS) + length; i++) {
                mask[(*position - ROTATION_BIT) % TOTAL_COLS] |= BIT_LEFT(i);
            }
        } else {
            for (int i = ((*position - ROTATION_BIT) / TOTAL_COLS) - 1; i < length + ((*position - ROTATION_BIT) / TOTAL_COLS) - 1; i++) {
                mask[(*position - ROTATION_BIT) % TOTAL_COLS] |= BIT_LEFT(i);
            }
        }
    }
}

/**
 * Moves the all pixels in the given bitmask up, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipUp(uint8_t *mask, uint8_t *position)
{
    // Checking if the ship can be moved.
    for (int i = 0; i < TOTAL_COLS; i++) {
        if (mask[i] & 1) {
            return;
        }
    }

    *position -= TOTAL_COLS;
    for (int i = 0; i < TOTAL_COLS; i++) {
        mask[i] >>= 1;
    }
}

/**
 * Moves the all pixels in the given bitmask down, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipDown(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    for (int i = 0; i < TOTAL_COLS; i++) {
        if (mask[i] & BIT_LEFT(6)) {
            return;
        }
    }

    *position += TOTAL_COLS;
    for (int i = 0; i < TOTAL_COLS; i++) {
        mask[i] <<= 1;
    }
}

/**
 * Moves the all pixels in the given bitmask left, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipLeft(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    if (mask[0] != 0) {
        return;
    }

    *position -= 1;
    for (int i = 1; i < TOTAL_COLS; i++) {
        mask[i - 1] = mask[i];
    }
    mask[TOTAL_COLS - 1] = 0;
}

/**
 * Moves the all pixels in the given bitmask right, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipRight(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    if (mask[TOTAL_COLS - 1] != 0) {
        return;
    }

    *position += 1;
    for (int i = TOTAL_COLS - 1; i > 0; i--) {
        mask[i] = mask[i - 1];
    }
    mask[0] = 0;
}

/**
 * Compares two bitmasks and determines if there is a collision between them
 * @param mask1 the first mask to compare
 * @param mask2 the second mask to compare
 * @return 1 if a collision is detected else 0
 */
char checkBitmaskCollision(uint8_t *mask1, uint8_t *mask2)
{
    for (int i = 0; i < TOTAL_COLS; i++) {
        if ((mask1[i] & mask2[i]) != 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Renders the ship the user is placing as well as previously placed ships and allows the user to move the current ship
 * and place is as long as it is not overlapping an existing ship
 * @param shipLength the length of the ship being placed
 * @param shipMask the bitmask of the previously placed ships
 */
void movePlaceShip(uint8_t shipLength, uint8_t *shipMask)
{
    ledmat_init();
    pacer_init(500);

    //led_set(0, 1);
    uint8_t currentColumn = 0;

    //bright frame
    uint8_t currentShipMask[5] = {0, 0, 0, 0, 0};

    // first bit is for rotation, second is 0, rest is location
    uint8_t shipPosition = 0b00011110;

    // first battle ship
    placeShip(currentShipMask, &shipPosition, shipLength);

    while (1) {
        system_init();
        pacer_wait();

        clearScreen();
        currentColumn++;
        if (currentColumn > TOTAL_COLS - 1) {
            currentColumn = 0;
        }

        navswitch_update();
        if (navswitch_push_event_p(NAVSWITCH_NORTH)) {
            moveShipUp(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
            moveShipDown(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_EAST)) {
            moveShipRight(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_WEST)) {
            moveShipLeft(currentShipMask, &shipPosition);
        }

        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            shipPosition ^= BIT_LEFT(7);
            for (int i = 0; i < TOTAL_COLS; i++) {
                currentShipMask[i] = 0;
            }
            placeShip(currentShipMask, &shipPosition, shipLength);
        }

        // Display shipFrame for a split second so that it is very dim
        displayColumn(shipMask[currentColumn], currentColumn);
        clearScreen();

        displayColumn(currentShipMask[currentColumn], currentColumn);

        if (checkBitmaskCollision(shipMask, currentShipMask)) {
            led_set(0, 0);
        } else {
            led_set(0, 1);
        }

        button_update();
        if (button_push_event_p(0) && !checkBitmaskCollision(shipMask, currentShipMask)) {
            for (int i = 0; i < TOTAL_COLS; i++) {
                shipMask[i] |= currentShipMask[i];
            }
            break;
        }
    }
}
