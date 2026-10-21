#include <stdio.h>

#include "board.h"

int main(void)
{
    board_console_init();
    board_led_init();
    board_init();

    printf("Hello, world!\n");

    for (;;)
    {
    }
}
