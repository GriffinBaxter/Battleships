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
    //ship is horizontal
    if (!(*position >> 7)) {
        // is the ship out of the left bound?
        if ((length == SHIP2_LENGTH || length == SHIP1_LENGTH) && *position % 5 == 0) {
            *position += 1;
        } else if ((length == SHIP3_LENGTH || length == SHIP2_LENGTH) && *position % 5 == 4) {
            *position -= 1;
        } else if (length == SHIP3_LENGTH && *position % 5 >= 3) {
            *position -= 3 - (5 - (*position % 5));
        }
        if (length == 2) {
            for (int i = *position % 5; i < length + (*position % 5); i++) {
                mask[i] |= (1 << (*position / 5));
            }
        } else {
            for (int i = (*position % 5) - 1; i < length + (*position % 5) - 1; i++) {
                mask[i] |= (1 << (*position / 5));
            }
        }
    } else { /* Ship is vertical */
        // is the ship out of the top bound
        if ((length == SHIP2_LENGTH || length == SHIP1_LENGTH) && *position - 128 < 5) {
            *position += 5;
        } else if ((length == SHIP3_LENGTH || length == SHIP2_LENGTH) && *position - 128 > 29) {
            *position -= 5;
        } else if (length == SHIP1_LENGTH && *position - 128 > 24) {
            *position -= 5 * (3 - (7 - ((*position - 128) / 5)));
        }
        if (length == 2) {
            for (int i = (*position - 128) / 5; i < ((*position - 128) / 5) + length; i++) {
                mask[(*position - 128) % 5] |= (1 << i);
            }
        } else {
            for (int i = ((*position - 128) / 5) - 1; i < length + ((*position - 128) / 5) - 1; i++) {
                mask[(*position - 128) % 5] |= (1 << i);
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
    // first check if we can move the ship
    for (int i = 0; i < 5; i++) {
        if (mask[i] & 1) {
            return;
        }
    }

    *position -= 5;
    for (int i = 0; i < 5; i++) {
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
    for (int i = 0; i < 5; i++) {
        if (mask[i] & (1 << 6)) {
            return;
        }
    }

    *position += 5;
    for (int i = 0; i < 5; i++) {
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
    for (int i = 1; i < 5; i++) {
        mask[i - 1] = mask[i];
    }
    mask[4] = 0;
}

/**
 * Moves the all pixels in the given bitmask right, providing none would be placed off-screen
 * @param mask the bitmask containing the ship pixel information
 * @param position the position and rotation of the ship
 */
void moveShipRight(uint8_t *mask, uint8_t *position)
{
    // first check if we can move the ship
    if (mask[4] != 0) {
        return;
    }

    *position += 1;
    for (int i = 4; i > 0; i--) {
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
char checkFrameCollision(uint8_t *mask1, uint8_t *mask2)
{
    for (int i = 0; i < 5; i++) {
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
    uint8_t current_column = 0;

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
        current_column++;
        if (current_column > 4) {
            current_column = 0;
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
            shipPosition ^= 1 << 7;
            for (int i = 0; i < 5; i++) {
                currentShipMask[i] = 0;
            }
            placeShip(currentShipMask, &shipPosition, shipLength);
        }

        // Display shipFrame for a split second so that it is very dim
        display_column(shipMask[current_column], current_column);
        clearScreen();

        display_column(currentShipMask[current_column], current_column);

        if (checkFrameCollision(shipMask, currentShipMask)) {
            led_set(0, 0);
        } else {
            led_set(0, 1);
        }

        button_update();
        if (button_push_event_p(0) && !checkFrameCollision(shipMask, currentShipMask)) {
            for (int i = 0; i < 5; i++) {
                shipMask[i] |= currentShipMask[i];
            }
            break;
        }
    }
}
