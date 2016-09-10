#include <msp430.h>
#include "ws2812.h"

typedef struct {
	u_char red;
	u_char green;
	u_char blue;
} LED;

#define OUTPUT_PIN	(0x80)			// Set to whatever UCB0SIMO is on your processor (Px.7 here)

#define NUM_LEDS	(5)			// NUMBER OF LEDS IN YOUR STRIP

LED leds[NUM_LEDS] = {{0,0,0}};


/*
 * Timing Analysis :
 *
 *  Per WS2812B :
 *
 *  				0				1				End
 *  		Min		0.2us			0.75us			1.1us
 *  		Max		0.5us			1.05us			1.4us
 *
 *  8 Bit 16Mhz/3	0b11 0.375us	0b1111 0.75us	1.5us
 *  4 Bit 16Hmz/5	0b1  0.313us	0b111 0.938us	1.25us	2 bit by spi send
 */


// Initializes everything needed to use this library. This clears the strip.
void initStrip(){
	P1SEL |= OUTPUT_PIN;			// configure output pin as SPI output
	P1SEL2 |= OUTPUT_PIN;
	UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; 	// 3-pin, MSB, 8-bit SPI master
	UCB0CTL1 |= UCSSEL_2;			// SMCLK source (16 MHz)
	UCB0BR0 = 5;					// 16 MHz / 5 = 0.313 us / bit
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;			// Initialize USCI state machine

	clearStrip();					// clear the strip
}

// Sets the color of a certain LED (0 indexed)
void setLEDColor(u_int p, u_char r, u_char g, u_char b){
	leds[p].red = r;
	leds[p].green = g;
	leds[p].blue = b;
}

// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip(){
	__bic_SR_register(GIE);       	// disable interrupts
	
	// send RGB color for every LED
	int i, j;
	u_char output;
	for (i = 0; i < NUM_LEDS; i++){
		u_char rgb[3] = {leds[i].green, leds[i].red, leds[i].blue};	// get RGB color for this LED

		// send green, then red, then blue
		for (j = 0; j < 3; j++){
			u_char mask = 0x80;					// b1000000

			// check each of the 8 bits
			while(mask != 0){

				// Creating output from 2 bits
				if (rgb[j] & mask){			// most significant bit first
					output = UPPERBIT(HIGH_CODE);		// send 1
				}
				else {
					output = UPPERBIT(LOW_CODE);		// send 0
				}
				mask >>= 1;						// check next bit
				if (rgb[j] & mask){			// most significant bit first
					output |= (HIGH_CODE);		// send 1
				}
				else {
					output |= (LOW_CODE);		// send 0
				}

				while (!(IFG2 & UCB0TXIFG));	// wait to transmit

				UCB0TXBUF = output;

				mask >>= 1;						// check next bit
			}
		}
	}

	// send RES code for at least 50 us (800 cycles at 16 MHz)
	_delay_cycles(800);

	__bis_SR_register(GIE);       	// enable interrupts
}

// Clear the color of all LEDs (make them black/off)
void clearStrip(){
	fillStrip(0x00, 0x00, 0x00);	// black
}

// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b){
	int i;
	for (i = 0; i < NUM_LEDS; i++){
		setLEDColor(i, r, g, b);		// set all LEDs to specified color
	}
	showStrip();						// refresh strip
}
