#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "lmt87lpm.h"

const uint16_t lmt87_temp_table[2][16] = 
                {
                    {10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25},
                    {3102,3085,3069,3051,3035,3019,3002,2986,2968,2952,
                     2935,2919,2901,2885,2868,2852}
                };

