/*
 * I2C.c
 *
 *  Created on: Jun 6, 2015
 *      Author: Warren Woolsey
 *
 *      Contains all necessary functions to set up, read, and write I2C data with the
 *      MSP430G2553 and similar family micro processors.
 *
 *      MSP430G2553 Launchpad pin designations
 *      P1.6 SCL
 *      P1.7 SDA
 */
#include <msp430.h>
#include <intrinsics.h>
#include "I2C.h"

// receiver variables
unsigned char i2cIsrRxByte = 0;
unsigned char i2cIsrRxByteReceived = 0;
unsigned char i2cRxExpectingMoreData = 0;

// transmitter variables
unsigned char i2cIsrTxByte = 0;
unsigned char i2cTxDataToSend = 0;

// timer variables
unsigned int i2cTime = 0;


/*
 * Initializes the MSP430G2553 I2C module for use with the MPU6050.
 */
void InitI2C()
{
	// set up clock registers
    WDTCTL 	= 	WDTPW | WDTHOLD;				// Stop watchdog timer
	DCOCTL	&=	0;								// reset DCO
	DCOCTL	|=	CALDCO_16MHZ;
	BCSCTL1	&=	0;								// reset BCSCTL1
	BCSCTL1	|=	CALBC1_16MHZ;
	BCSCTL2	&=	0;								// reset BCSCTL2
	BCSCTL2	|=	SELM_0	|	DIVM_0	|	DIVS_0;	// MCLK and SMCLK run from the DCO @ 16 MHz

	// set up the GPIO ports to enable I2C functionality
	P1DIR  &= ~BIT6;
	P1DIR  &= ~BIT7;
	P1IE   &= ~BIT6;
	P1IE   &= ~BIT7;
	P1SEL  |= BIT6 + BIT7;
	P1SEL2 |= BIT6 + BIT7;

	UCB0CTL1 |= UCSWRST;					// enable reset so we can edit the register
	UCB0CTL0 |=	UCMST + UCMODE_3 + UCSYNC;	// single master, i2c, syncronous transmission
	UCB0CTL1 |=	UCSSEL_2 + UCMST;			// SMCLK, I2C Master mode
	UCB0CTL1 &= ~UCTXNACK;
	UCB0BR1	 =	0;
	UCB0BR0	 =	40;						// 16,000,000 / 40 = 400,000 Hz
	UCB0CTL1 &=	~UCSWRST;					// exit reset state

	// set up timer A so we can use it as the I2C timer
	TACTL		= TACLR;
	TACTL		|= TASSEL_2 + MC_1;
	TA0CCR0		= 16000;					// interrupt at 1 ms
	TA0CCTL0	&= ~CCIFG;
	TA0CCTL0	|= CCIE;

	_enable_interrupts();
}

/*
 * Will send the start transmission sequence to access the MPU6050 register address located at
 * mpuRegAddr.
 * Returns true if the I2C module is not busy.
 */
unsigned char StartTransmission(unsigned char slaveAddr, unsigned char mpuRegAddr)
{
	// check if the I2C bus is busy
	SetI2CTime(0);
	if (UCB0STAT & UCBBUSY)
	{
		if (GetI2CTime() >= 50)
			return 0;
	}

	SetI2CTime(0);
	IFG2 &= ~UCB0TXIFG;
	UCB0I2CSA = slaveAddr;
	UCB0CTL1 |= UCTR;
	UCB0CTL1 |= UCTXSTT;
	while (!(IFG2 & UCB0TXIFG))
	{
		if (GetI2CTime() >= 50)
			return 0;
	}

	SetI2CTime(0);
	UCB0TXBUF = mpuRegAddr;
	while (!(IFG2 & UCB0TXIFG))
	{
		if (GetI2CTime() >= 50)
			return 0;
	}

	return 1;
}

/*
 * Will attempt to write len worth of txBytes to the MPU6050 register address located at mpuRegAddr.
 * Returns true if the transmission succeeded and false otherwise.
 */
unsigned char WriteI2CBytes(unsigned char slaveAddr, unsigned char mpuRegAddr, unsigned char *txBytes, unsigned char len)
{
	// try to start the transmission up to 5 times
	unsigned char tries = 0;
	for (; !(StartTransmission(slaveAddr, mpuRegAddr)) && tries < 5; tries++);

	// start transmission failed
	if (tries == 5)
	{
		return 0;
	}

	// wait for line to hold up
	SetI2CTime(0);
	while (!(UCB0STAT & UCSCLLOW))
	{
		if (GetI2CTime() >= 10)
			return 0;
	}
	SetI2CTime(0);
	unsigned char i = 0;
	for (; i < len;)
	{
		while (!(IFG2 & UCB0TXIFG))
		{
			if (GetI2CTime() >= 100)
				return 0;
		}
		UCB0TXBUF = *(txBytes + i);
		i++;
	}

	while (!(IFG2 & UCB0TXIFG))
	{
		if (GetI2CTime() >= 100)
			return 0;
	}

	UCB0CTL1 |= UCTXSTP;

	return 1;
}

/*
 * Will attempt to read len worth of rxBytes from the MPU6050 register address located at mpuRegAddr.
 * Returns true of the read succeeded and false otherwise.
 */
unsigned char ReadI2CBytes(unsigned char slaveAddr, unsigned char mpuRegAddr, unsigned char *rxBytes, unsigned char len)
{
	// try to start the transmission up to 5 times
	unsigned char tries = 0;
	for (; !(StartTransmission(slaveAddr, mpuRegAddr)) && tries < 5; tries++);

	// start transmission failed
	if (tries == 5)
	{
		return 0;
	}

	// wait for line to hold up
	SetI2CTime(0);
	while (!(UCB0STAT & UCSCLLOW))
	{
		if (GetI2CTime() >= 100)
			return 0;
	}

	// start read
	IFG2 &= ~UCB0TXIFG;
	UCB0CTL1 &= ~UCTR;
	UCB0CTL1 |= UCTXSTT;
	while (!(IFG2 & UCB0RXIFG))
	{
		if (GetI2CTime() >= 100)
			return 0;
	}

	*(rxBytes) = UCB0RXBUF;

	return 1;
}

/*
 * Will attempt to read a single byte from the MPU6050 register address located at mpuRegAddr.
 * Returns true if the read succeeded and false otherwise.
 */
unsigned char ReadI2CByte(unsigned char slaveAddr, unsigned char mpuRegAddr, unsigned char *rxBytes)
{
	// try to start the transmission up to 5 times
	unsigned char tries = 0;
	for(; !(StartTransmission(slaveAddr, mpuRegAddr)) && tries < 5; tries++)
	{
		_delay_cycles(1600);
	}

	// start transmission failed
	if (tries == 5)
	{
		return 0;
	}

	// wait for line to hold up
	SetI2CTime(0);
	while(!(UCB0STAT & UCSCLLOW))
	{
		if (GetI2CTime() >= 100)
			return 0;
	}

	// start read
	IFG2 &= ~UCB0TXIFG;
	UCB0CTL1 &= ~UCTR;
	UCB0CTL1 |= UCTXSTT;
	while (UCB0CTL1 & UCTXSTT)
	{
		if (GetI2CTime() >= 100)
			return 0;
	}

	UCB0CTL1 |= UCTXNACK + UCTXSTP;
	while (!(IFG2 & UCB0RXIFG))
	{
		if (GetI2CTime() >= 100)
			return 0;
	}

	*(rxBytes) = UCB0RXBUF;

	return 1;
}

/*
 * Will set the value of i2cTime
 */
void SetI2CTime(unsigned int val)
{
	i2cTime = val;
}

/*
 * Will return the value of i2cTime
 */
unsigned int GetI2CTime()
{
	return i2cTime;
}

/*
 * updates the i2cTime variable at 1000 Hz
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void I2CTimerISR(void)
{
	i2cTime++;
	TA0CCTL0 &= ~CCIFG;
}
