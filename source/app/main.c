#include <stdio.h>

#include "board.h"
#include "zynq_tick.h"

int main(void)
{
    board_init();

    printf("Hello, world!\n");

    for (;;)
    {
        board_led_toggle();
        printf("tick %lu\n", (unsigned long)tick_get());
        tick_mdelay(500);
    }
}
