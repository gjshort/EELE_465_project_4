/******************************
* Author:   Gabe Story
* Date:     02.19.2026
* Class:    EELE 465
* Purpose:  Header file for LEDstrip mechanism & policy
*************************************************************/

#pragma once            // include once, dont redefine else where

#define numPixels   10  // LED stick used for this code has 10 pixels

// Struct for colors
typedef struct RGB {

    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;

}   RGB;
