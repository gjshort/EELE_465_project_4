/***********************************
* Author:   Gabe Story
* Date:     02.26.2026
* Class:    EELE 465
* Purpose:  This c file will hold all the functions needed
            to operate the WS2812B LED stick

            We will be able to set the color for individual
            pixels, and change brightness, as well as map LED
            pixels to potentiometer.
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

unsigned int LEDpot;

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
        colorStateSingle();                     // IF not, just change the color of the first pixel
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

    /**
        This function only works with r, g, b selected
        one at a time, not exactly in the req doc so
        just for funsies
    **/
    if((g != 0) && (r == 0) && (b == 0)) {      // if first pixel is green
        for(g = 255; g > 0; g--) {
            setColor(0, g, 0, 0);
            sendStick();
            __delay_cycles(400000);

        }
    }

    if((g == 0) && (r != 0) && (b == 0)) {    // if first pixel is red
        for(r = 255; r > 0; r--) {
            setColor(0, r, 0, 0);
            sendStick();
            __delay_cycles(400000);

        }
    }

    if((g == 0) && (r == 0) && (b != 0)) {    // if first pixel is blue
        for(b = 255; b > 0; b--) {
            setColor(0, b, 0, 0);
            sendStick();
            __delay_cycles(400000);

        }
    }

}

void potStick(u_int n, u_char g, u_char r, u_char b) {

        ADCCTL0 |= ADCENC | ADCSC;          // enable & start conversion
        while((ADCIFG & ADCIFG0) == 0) {}
        LEDpot = ADCMEM0;

        /* This if/else statement reads voltage on pin 6
           and then changes the compare value, 
           lighting up a pixel */
        if(LEDpot <= 50) {
            setColor(0, 25, 25, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 100) {
            setColor(0, 50, 50, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 150) {
            setColor(0, 75, 75, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 200) {
            setColor(0, 100, 100, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 250) {
            setColor(0, 125, 125, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 300) {
            setColor(0, 150, 150, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 350) {
            setColor(0, 175, 175, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 400) {
            setColor(0, 200, 200, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 450) {
            setColor(0, 225, 225, 0);
            setColor(1, 0, 0, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 500) {
            setColor(0, 250, 250, 0);
            setColor(1, 25, 25, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 550) {
            setColor(0, 250, 250, 0);
            setColor(1, 50, 50, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 600) {
            setColor(0, 250, 250, 0);
            setColor(1, 75, 75, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 650) {
            setColor(0, 250, 250, 0);
            setColor(1, 100, 100, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 700) {
            setColor(0, 250, 250, 0);
            setColor(1, 125, 125, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 750) {
            setColor(0, 250, 250, 0);
            setColor(1, 150, 150, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 800) {
            setColor(0, 250, 250, 0);
            setColor(1, 175, 175, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 850) {
            setColor(0, 250, 250, 0);
            setColor(1, 200, 200, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 900) {
            setColor(0, 250, 250, 0);
            setColor(1, 225, 225, 0);
            setColor(2, 0, 0, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 950) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 12, 25, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1000) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 24, 50, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1050) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 36, 75, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1100) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 48, 100, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1150) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 60, 125, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1200) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 72, 150, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1250) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 84, 175, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1300) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 96, 200, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1350) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 108, 225, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1400) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 120, 250, 0);
            setColor(3, 0, 0, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1450) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 120, 250, 0);
            setColor(3, 12, 25, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1500) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 24, 50, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1550) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 36, 75, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1600) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 48, 100, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1650) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 60, 125, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1700) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 72, 150, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1750) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 84, 175, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 1800) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 96, 200, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if (LEDpot <= 1850) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 108, 225, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        }

       //--------------------------------------- //

        
        else if(LEDpot <= 1900) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 0, 0, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);    
        }  else if(LEDpot <= 1950) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 10, 25, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2000) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 20, 50, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2050) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 20, 75, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2100) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 20, 100, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2150) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 20, 125, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2200) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 20, 150, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2250) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 20, 175, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2300) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 40, 200, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2350) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 100, 225, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2400) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 0, 0, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2450) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 10, 25, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2500) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 20, 50, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2550) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 20, 75, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2600) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 20, 100, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2650) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 20, 125, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2700) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 20, 150, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2750) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 20, 175, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2800) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 40, 200, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2850) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 100, 225, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2900) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 0, 0, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 2950) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 10, 25, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3000) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 20, 50, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3050) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 20, 75, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3100) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 20, 100, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3150) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 20, 125, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3200) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 20, 150, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3250) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 20, 175, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3300) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 40, 200, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3350) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 100, 225, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3400) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 0, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3450) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 25, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3500) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 50, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3550) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 75, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3600) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 100, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3650) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 125, 0);
            setColor(8, 0, 0, 0);
        } else if (LEDpot <= 3700) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 150, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3750) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 175, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3800) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 200, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3850) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 225, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3900) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 250, 0);
            setColor(8, 0, 0, 0);
        } else if(LEDpot <= 3950) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 250, 0);
            setColor(8, 0, 100, 0);
        } else if(LEDpot <= 4000) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 250, 0);
            setColor(8, 0, 150, 0);
        } else if(LEDpot <= 4050) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 250, 0);
            setColor(8, 0, 200, 0);
        } else if (LEDpot <= 4100) {
            setColor(0, 250, 250, 0);
            setColor(1, 250, 250, 0);
            setColor(2, 132, 250, 0);
            setColor(3, 120, 250, 0);
            setColor(4, 165, 250, 0);
            setColor(5, 165, 250, 0);
            setColor(6, 165, 250, 0);
            setColor(7, 0, 250, 0);
            setColor(8, 0, 250, 0);
        }
        
    sendStick();
        
}

/* References:   
I want to cite these sources for inspiration & useful how-to
that helped me get started.
               
                - Mjmeli: MSP430 | NeoPixel library https://github.com/mjmeli/MSP430-NeoPixel-WS2812-Library/tree/master
*/
