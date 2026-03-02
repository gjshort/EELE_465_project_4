#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "float.h"

/**
 * Reverses the digit order of an integer.
 * @param num - the integer to be reversed
 *
 * @return the reversed number
 *
 * Credit: 'kartik' on GeeksForGeeks.com
 * https://www.geeksforgeeks.org/dsa/write-a-program-to-reverse-digits-of-a-number/
 */
static uint16_t reverse_digits(uint16_t num) {
    uint16_t rev_num = 0;
    while (num > 0) {
        rev_num = rev_num * 10 + num % 10;
        num /= 10;
    }
    return rev_num;
}

/**
 * Converts a signed floating point number to a
 * a string with 2 places of truncated precision.
 * @param data - floating point value to be converted
 * @param result - pointer to string buffer. USER MUST
 *                 ENSURE IT IS LARGE ENOUGH.
 *
 * Example:
 * -23.4578 ---> [-,2,3,.,4,5,\0]
 */
void ftoa_2(float data, char result[16])
{
    uint8_t digit_idx = 0;

    // Sign
    if(data < 0)
    {
        result[digit_idx++] = '-';
        data *= -1;
    } 

    // Integer part: reverse, then push digits to buffer
    uint16_t int_part = (uint16_t)data;
    uint16_t rev_int_part = reverse_digits(int_part);
    while(rev_int_part > 0)
    {
        result[digit_idx++] = (rev_int_part % 10) + '0';
        rev_int_part /= 10;
    }

    // Decimal point
    result[digit_idx++] = '.';

    // Fractional part (2 places truncated of precision)
    uint16_t frac_part = (uint16_t)((data - int_part) * 100);  // 0.12345 --> 12
    uint16_t rev_frac_part = reverse_digits(frac_part);

    if(rev_frac_part == 0)
    {
        result[digit_idx++] = '0';
    }

    while(rev_frac_part > 0)
    {
        result[digit_idx++] = (rev_frac_part % 10) + '0';
        rev_frac_part /= 10;
    }

    // String null termination
    result[digit_idx] = '\0';
}
