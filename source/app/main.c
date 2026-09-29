#include "board.h"

int main(void) {
    board_init();

    irq_init();
    tick_init();

    // Route MIO 24 to the GPIO controller and make it an output.

    // All MIO mux selects at 0 select the GPIO function; the remaining
    // fields configure the IO buffer only. SLCR writes must be wrapped
    // in unlock/lock.
    SLCR->UNLOCK = SLCR_UNLOCK_KEY;
    SLCR->MIO_PIN_24 = MIO_PIN_OPT_IO_CMOS_3V3;  // GPIO function, LVCMOS 3.3V, slew slow, receiver on
    SLCR->LOCK = SLCR_LOCK_KEY;

    // GPIO bank 0 covers MIO 0..31: set direction and enable the output
    // driver for MIO 24.
    GPIO->DIRM_0 |= LED_BIT;  // output
    GPIO->OEN_0 |= LED_BIT;   // output enable

    // Blink: slow when the PL came up, fast when it did not (read
    // pl_status for the reason; a debugger can halt here and inspect it).
    for (;;) {
        GPIO->DATA_0 ^= LED_BIT;
        tick_mdelay(pl_status == DEVCFG_OK ? 500 : 100);
    }
}
