/**
 * @file  sysmem.c
 * @brief newlib heap: _sbrk() over the linker-reserved heap region.
 */

#include <errno.h>
#include <stdint.h>

/* Heap region reserved by flash_xip.ld. Fixed size, bounded above by the
   exception stacks, so malloc can never grow into them. */
extern uint32_t __heap_start__;
extern uint32_t __heap_end__;

/**
 * @brief  Grow the newlib heap.
 * @param  incr Bytes to add.
 * @return Previous heap end, or (void*)-1 with errno = ENOMEM when the
 *         request does not fit the reserved region.
 */
void* _sbrk(ptrdiff_t incr) {
    static uint8_t* heap_end;
    uint8_t* prev;

    if (heap_end == 0) {
        heap_end = (uint8_t*)&__heap_start__;
    }

    if ((heap_end + incr) > (uint8_t*)&__heap_end__) {
        errno = ENOMEM;
        return (void*)-1;
    }

    prev = heap_end;
    heap_end += incr;
    return prev;
}
