#include <msp430.h> 
#include "ws2812.h"

void gradualFill(u_int n, u_char r, u_char g, u_char b);

int main(void) {
	WDTCTL = WDTPW + WDTHOLD;				// Stop WDT
	if (CALBC1_16MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                			// do not load, trap CPU!!
	}

	// configure clock to 16 MHz
	BCSCTL1 = CALBC1_16MHZ;       			// DCO = 16 MHz
	DCOCTL = CALDCO_16MHZ;

	_delay_cycles(1000000);         // lazy delay

	// initialize LED strip
	initStrip();			// ***** HAVE YOU SET YOUR NUM_LEDS DEFINE IN WS2812.C? ******

#define PWR 0x10

	// set strip color red
	fillStrip(PWR, 0x00, 0x00);

	// show the strip
	showStrip();

	_delay_cycles(1000000);         // lazy delay


	// gradually fill for ever and ever
	u_int numLEDs = NUM_LEDS;
	while(1){
		gradualFill(numLEDs, 0x00, PWR, 0x00);		// green
		gradualFill(numLEDs, 0x00, 0x00, PWR);		// blue
		gradualFill(numLEDs, PWR, 0x00, PWR);		// magenta
		gradualFill(numLEDs, PWR, PWR, 0x00);		// yellow
		gradualFill(numLEDs, 0x00, PWR, PWR);		// cyan
		gradualFill(numLEDs, PWR, 0x00, 0x00);		// red
	}
}

void gradualFill(u_int n, u_char r, u_char g, u_char b){
	int i;
	for (i = 0; i < n; i++){			// n is number of LEDs
		setLEDColor(i, r, g, b);
		showStrip();
		_delay_cycles(1000000);			// lazy delay
	}
}
