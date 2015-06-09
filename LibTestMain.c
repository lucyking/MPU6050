/*
 * LibTestMain.c
 *
 *  Created on: Jun 6, 2015
 *      Author: Warren Woolsey
 */
#include <msp430.h>
#include <intrinsics.h>
#include "MPU6050.h"
#include "I2C.h"

int main(void)
{
	// set up clock registers
    WDTCTL 	= 	WDTPW | WDTHOLD;				// Stop watchdog timer
	DCOCTL	&=	0;								// reset DCO
	DCOCTL	|=	CALDCO_16MHZ;
	BCSCTL1	&=	0;								// reset BCSCTL1
	BCSCTL1	|=	CALBC1_16MHZ;
	BCSCTL2	&=	0;								// reset BCSCTL2
	BCSCTL2	|=	SELM_0	|	DIVM_0	|	DIVS_0;	// MCLK and SMCLK run from the DCO @ 16 MHz
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	InitI2C();

	if (TestI2CConnection())
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	if (EnableSleepMode(0))
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	if (MPUSelfTest())
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	unsigned char gyroXFT = 0;
	if (GyroXSelfTest(&gyroXFT))
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	while (1);
}
