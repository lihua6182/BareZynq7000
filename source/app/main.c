#include "zynq_delay.h"
#include "zynq_devcfg.h"
#include "zynq_fiq.h"
#include "zynq_soc_ip.h"

#define LED_MIO_PIN (24u)                /* MIO pin number of the LED            */
#define LED_BIT     (1UL << LED_MIO_PIN) /* LED bit within GPIO bank 0 (MIO 0..31) */

int main(void) {
    /* Shared FIQ infrastructure (GIC -> FIQ, per-source weak handlers),
       then the 1 ms private-timer tick that the blink delay relies on.
       The PL was programmed by zynq_devcfg_boot() on the _start path, so
       by the time we get here the PL is up or g_pl_status says why not. */
    zynq_fiq_init();
    zynq_delay_init();

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
    // g_pl_status for the reason; a debugger can halt here and inspect it).
    for (;;) {
        GPIO->DATA_0 ^= LED_BIT;
        zynq_delay_ms(g_pl_status == ZYNQ_DEVCFG_OK ? 500 : 100);
    }
}
