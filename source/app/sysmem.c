#include <errno.h>
#include <stdint.h>

extern uint32_t __heap_start__;
extern uint32_t __heap_end__;

void *_sbrk(ptrdiff_t incr)
{
    static uint8_t *heap_end;
    uint8_t *prev;

    if (heap_end == 0)
    {
        heap_end = (uint8_t *)&__heap_start__;
    }

    if ((heap_end + incr) > (uint8_t *)&__heap_end__)
    {
        errno = ENOMEM;
        return (void *)-1;
    }

    prev = heap_end;
    heap_end += incr;
    return prev;
}
