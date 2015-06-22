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

	if (GyroAccelXSelfTest())
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	if (GyroAccelYSelfTest())
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	if (GyroAccelZSelfTest())
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	if (EnableMPUInterrupt(ENABLE_DATA_RDY_INTERRUPT))
		P1OUT |= BIT0;
	else
		P1OUT &= ~BIT0;

	while (1)
	{
		if (GetMPUIntStatus() == MPU6050_DATA_RDY_INT)
		{
			GetGyroVals();
			GetAccelVals();
			P1OUT |= BIT0;
		}
		else
		{
			P1OUT &= ~BIT0;
		}
	}
}
