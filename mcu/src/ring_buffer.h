#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdint.h>
#include <stdbool.h>

#define RING_BUF_SIZE 16    // Max = 254 for downward wrap-around 
                            // overflow checking

typedef struct ring_buffer
{
    uint16_t data[RING_BUF_SIZE];
    uint8_t head;
} ring_buffer;

void ring_buf_push(ring_buffer*, uint16_t);
float ring_buf_average(ring_buffer*, uint8_t);


#endif
