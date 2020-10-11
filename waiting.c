#include "waiting.h"
#include "matrix_display.h"

#include "ir_uart.h"
#include "tinygl.h"

/**
 * Waits until the UCFK receives confirmation from the other UCFK about whether the fired shot hit or missed
 * @param numHits pointer to number of successful hits, incremented if hit is confirmed
 */
void waitHitConfirmation(uint8_t* numHits)
{
    while (1) {
        if (ir_uart_read_ready_p()) {
            tinygl_clear();
            if (ir_uart_getc() == 1) {
                displayText("HIT!");
                *numHits += 1;
            } else {
                displayText("MISS");
            }
            break;
        }
    }
}

/**
 * Waits until other player has completed their turn by continually polling the IR Receiver for information about the
 * enemy's shot position
 * @param shotRow pointer to the value of show row
 * @param shotCol pointer to the value of shot column
 */
void waitTurn(uint8_t *shotRow, uint8_t *shotCol)
{
    uint8_t rowCol = 0;
    while (1) {
        if (ir_uart_read_ready_p()) {
            rowCol = ir_uart_getc();
            break;
        }
    }
    *shotRow = rowCol / 5;
    *shotCol = rowCol % 5;
}
