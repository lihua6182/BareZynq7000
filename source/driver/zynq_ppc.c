/**
 * @file zynq_ppc.c
 * @brief Blocking helpers for the proto_coproc protocol co-processor.
 */

#include "zynq_ppc.h"

#include "zynq_tick.h"

/**
 * @brief  Wait until the co-processor executed its HLT instruction.
 * @param  c           Controller.
 * @param  timeout_ms  Give up after this many milliseconds.
 * @return True when halted in time, false on timeout.
 */
bool coproc_wait_halt(COPROC_Type *c, uint32_t timeout_ms)
{
    uint32_t t0 = tick_get();
    while (!coproc_is_halted(c))
    {
        if ((tick_get() - t0) > timeout_ms)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief  Push one word, waiting while the TX FIFO is full.
 * @return True on success, false on timeout.
 */
bool coproc_tx_push_wait(COPROC_Type *c, uint32_t w, uint32_t timeout_ms)
{
    uint32_t t0 = tick_get();
    while (!coproc_tx_ready(c))
    {
        if ((tick_get() - t0) > timeout_ms)
        {
            return false;
        }
    }
    coproc_tx_push(c, w);
    return true;
}
