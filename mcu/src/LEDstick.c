/***********************************
* Author:   Gabe Story
* Date:     02.26.2026
* Class:    EELE 456
* Purpose:  This c file will hold all the functions needed
            to operate the WS2812B LED stick

            We will be able to set the color for individual
            pixels, and change brightness
**********************************************************************/

#include    <msp430fr2153.h>
#include    <math.h>
#include    <stdint.h>
#include    "LEDstick.h"

// Struct for LED color
typedef struct {

    u_char  green;
    u_char  red;
    u_char  blue;

} GRB;

static GRB ledStick[PixNumber] = { {0, 0, 0} };
int color_state = 0;
int color_state2 = 0;

// Initialize main clk to 16MHz
void init_CLK() {

    FRCTL0 = FRCTLPW | NWAITS_1;
    __bis_SR_register(SCG0);

    CSCTL3 = SELREF__REFOCLK;
    CSCTL1 = DCORSEL_5;
    CSCTL2 = FLLD_0 + 487;
    __delay_cycles(3);

    __bic_SR_register(SCG0);
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;

}

// Initialize SPI for blasting that LED Stick
void init_SPI() {

    // Set pin for SPI SIMO
    P1SEL0 |= BIT2;
    P1DIR |= BIT2;

    UCB0CTLW0 |= UCSWRST;           // enable software reset

    UCB0CTLW0 &= ~UCCKPL;
    UCB0CTLW0 &= ~UCCKPH;
    UCB0CTLW0 |= UCMSB;             // configure to 3-pin SPI, master, tx MSB first
    UCB0CTLW0 |= UCMST;
    UCB0CTLW0 |= UCSYNC;

    UCB0CTLW0 |= UCSSEL__SMCLK;     // prescale to 2.667 MHz
    UCB0BRW = 6;

    UCB0CTLW0 &= ~UCSWRST;          // take out of software reset

}

// Set the color for an individual pixel
void setColor(u_int n, u_char g, u_char r, u_char b) {

    ledStick[n].green = g;
    ledStick[n].red   = r;
    ledStick[n].blue  = b;

}

// Send color to the Stick
void sendStick() {

    __bic_SR_register(GIE);                         // disable interrupts

    // Send GRB color to each pixel
    unsigned int i, j;
    for(i = 0; i < PixNumber; i++) {
        u_char *grb = (u_char *) & ledStick[i];     // get grb color for i-th pixel

        // Send green values, red values, then blue values
        for(j = 0; j < 3; j++) {
            u_char mask = 0x80;                     // 0b1000000

            // check each bit
            while(mask != 0) {
                while(!(UCB0IFG & UCTXIFG));        // wait to tx
                if(grb[j] & mask) {
                    UCB0TXBUF = highTide;           // send 1
                } else {
                    UCB0TXBUF = lowTide;            // send 0
                }
                mask >>= 1;                         // check next bit
            }
        }
    }

    __delay_cycles(800);            // send reset signal
    __bis_SR_register(GIE);

}

// Clear the Stick, turn all pixels off
void clearStick() {

    StickFiller(0x00, 0x00, 0x00);

}

// Populate whole Stick with one color
void StickFiller(u_char g, u_char r, u_char b) {

    int i;
    for(i = 0; i < PixNumber; i++) {
        setColor(i, g, r, b);
    }
    sendStick();
}

/********************  
    This was the button used for switching
    between RTC registers and LED bar patterns.

    Now it will be used to change color of LED stick
************************************************************/
void init_LEDstick_color_button() {

    P3SEL0 &= ~BIT5;    // Set to Digital IO
    P3SEL1 &= ~BIT5;

    P3DIR &= ~BIT5;     // Set as input
    P3REN |= BIT5;      // Enable resistor
    P3OUT &= ~BIT5;     // Pull down resistor

}

// Button connected to P3.5 is pressed we change the color of the first LED
void stickColor_change() {

    __delay_cycles(4000000);                    // is button still pressed?

    // IF so, cahnge the color of the WHOLE stick
    if((P3IN & BIT5) != 0) {
        colorStateWhole();
    } else if((P3IN & BIT5) == 0){
        colorStateSingle();                     // IF not, just chnage the color of the first pixel
    }
    
}

void colorStateSingle() {

    color_state++;
    switch(color_state) {
        case 1:     setColor(0, 0, 55, 0);      // red
        break;

        case 2:     setColor(0, 55, 0, 0);      // green
        break;

        case 3:     setColor(0, 0, 0, 55);      // blue
        break;

        case 4:     setColor(0, 55, 55, 55);    // white
                    color_state = 0;
        break;

        default:    break;
    }
    __delay_cycles(4000000);     // 0.25s delay
    sendStick();

}

void colorStateWhole() {

        __delay_cycles(4000000);
        color_state2++;
        switch(color_state2) {
            case 1:     StickFiller( 0, 55, 0);         // red
            break;

            case 2:     StickFiller(55, 0, 0);          // green
            break;

            case 3:     StickFiller(0, 0, 55);          // blue
            break;

            case 4:     StickFiller(55, 55, 55);        // white
                        color_state2 = 0;
            break;

            default:    break;
        }

    __delay_cycles(4000000);     // 0.25s delay
    sendStick();

}

void gradient(u_char g, u_char r, u_char b) {

    for(g = 255; g > 0; g--) {
        setColor(0, g, 0, 0);
        sendStick();
        __delay_cycles(400000);

    }

    }

/* References:   
I want to cite these sources for inspiration & useful how-to
that helped me get started.
               
                - Mjmeli: MSP430 | NeoPixel library https://github.com/mjmeli/MSP430-NeoPixel-WS2812-Library/tree/master
*/
