#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "ring_buffer.h"

/**
 * Pushes a new value to the ring buffer.
 * Minimal error checking, so user beware.
 * @param ring_buf - pointer to a previously created ring buffer
 * @param data - new value to be pushed to buffer
 * 
 * If ring buffer head is at the end of the allocated array,
 * head will be reset to index 0 and the data will be pushed there.
 *
 */
void ring_buf_push(ring_buffer* ring_buf, uint16_t data)
{
    uint8_t new_head = ring_buf->head + 1;
    if(new_head >= RING_BUF_SIZE)   // At end of array
    {
        new_head = 0;
    }

    ring_buf->data[new_head] = data;
    ring_buf->head = new_head;
}

/**
 * Computes the average of the last __ values
 * in the ring buffer. 
 * @param ring_buf - pointer to a previously created ring buffer
 * @param window - number of values in the buffer to average
 *
 * Starting at and including the buffer's head, sum over the 
 * number of values specified by 'window' by moving backwards
 * through the buffer. If it is required to wrap back around
 * from index 0 to RING_BUF_SIZE, the unsigned index variable
 * will overflow to 255 when decremented from 0.
 *
 */
float ring_buf_average(ring_buffer* ring_buf, uint8_t window)
{
    // Bound window size
    uint8_t window_cpy = window;
    if(window_cpy > RING_BUF_SIZE)
    {
        window_cpy = RING_BUF_SIZE;
    }
    else if(window_cpy < 1) 
    {
        window_cpy = 1;
    }
    
    // Sum backwards through buffer 
    uint8_t count;
    uint8_t idx = ring_buf->head;
    uint16_t sum = 0;
    for(count = 0; count < window_cpy; count++)
    {
        sum += ring_buf->data[idx--]; 
        if(idx > RING_BUF_SIZE)         // Unsigned overflow indicates index < 0
        {
            idx = RING_BUF_SIZE - 1;
        }
    }

    return ((float)sum) / window;
}

