/*
 * MPU6050.c
 *
 *  Created on: Jun 6, 2015
 *      Author: Warren Woolsey
 */
#include "MPU6050.h"
#include "I2C.h"

unsigned char byte;	// byte to send or receive

/*
 * Will attempt to read the MPU6050 WHO_AM_I register which contains the MPU6050's slave address.
 * Returns true if the read succeeded with the correct return value and false otherwise.
 */
unsigned char TestI2CConnection()
{
	byte = 0;

	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_WHO_AM_I, &byte))
	{
		if (byte == 0x68)
		{
			return 1;
		}
	}

	return 0;
}

/*
 * Will attempt to read the MPU6050 PWR_MGMT_1 register and either enable or disable the sleep mode bit
 * by writing the bit back to the register. Will return true upon a successful write and false otherwise.
 */
unsigned char EnableSleepMode(unsigned char en)
{
	byte = 0;

	// read the register to get the current value
	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_1, &byte))
	{
		if ((en) && !(byte & 0x40))
			byte |= 0x40;
		else if (!(en) && (byte & 0x40))
			byte &= ~0x40;
		else
			return 1;
	}
	else
		return 0;

	// write back the altered register value
	if (WriteI2CBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_1, &byte, 1))
	{
		return 1;
	}

	return 0;
}

/*
 * Will attempt to write to the MPU6050_RA_GYRO_CONFIG register and enable x, y, and z gyros to perform
 * a self test. Will set the full scale range to 250 deg / s.
 * Returns true if the write succeeded and false otherwise.
 */
unsigned char MPUSelfTest()
{
	byte = 0b11100000;
	if (WriteI2CBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_GYRO_CONFIG, &byte, 1))
	{
		return 1;
	}

	return 0;
}

/*
 * Will attempt to read the MPU6050_RA_XG_SELF_TEST register and acquire the factory trim
 * value for the x gyro. Will return true if the read is successful
 */
unsigned char GyroXSelfTest(unsigned char *gyroXFT)
{
	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_XG_SELF_TEST, gyroXFT))
	{
		*gyroXFT &= 0x0F;	// mask the lower 4 bits
		return 1;
	}

	return 0;
}

