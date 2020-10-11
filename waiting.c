#include "waiting.h"
#include "matrix_display.h"

#include "ir_uart.h"
#include "tinygl.h"


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
