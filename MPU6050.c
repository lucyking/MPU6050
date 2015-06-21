/*
 * MPU6050.c
 *
 *  Created on: Jun 6, 2015
 *      Author: Warren Woolsey
 */
#include "MPU6050.h"
#include "I2C.h"

unsigned char byte;	// byte to send or receive
unsigned char gyroXFT = 0;
unsigned char gyroYFT = 0;
unsigned char gyroZFT = 0;
unsigned char accelXFT = 0;
unsigned char accelYFT = 0;
unsigned char accelZFT = 0;
unsigned char gyroDataNdx = 0;
unsigned char gyroData[6];

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
 * Will attemp to write to the MPU6050_RA_ACCEL_CONFIG register and enable the x, y, and x accelts to perform
 * a self test. Will set the full scale range to +- 8 deg / s
 * Returns true if the write succeeded and false otherwise.
 */
unsigned char MPUSelfTest()
{
	unsigned char rtn = 0;
	byte = 0b11100000;
	if (WriteI2CBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_GYRO_CONFIG, &byte, 1))
	{
		rtn = 1;
	}
	else
	{
		rtn = 0;
	}

	byte = 0b11110000;
	if (WriteI2CBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_CONFIG, &byte, 1))
	{
		rtn = 1;
	}
	else
	{
		rtn = 0;
	}

	return rtn;;
}

/*
 * Will attempt to read the MPU6050_RA_X_SELF_TEST register and acquire the factory trim
 * value for the x gyro & accel. Will return true if the read is successful
 */
unsigned char GyroAccelXSelfTest()
{
	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_X_SELF_TEST, &byte))
	{
		gyroXFT = byte & 0x1F;	// mask the lower 5 bits
		accelXFT = byte & 0xE0;
		return 1;
	}

	return 0;
}

/*
 * WIll attemp to read the MPU6050_RA_Y_SELF_TEST register and acquire the factory trim
 * value for the y gyro and accel. Will return true if the read is successful
 */
unsigned char GyroAccelYSelfTest()
{
	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_Y_SELF_TEST, &byte))
	{
		gyroYFT = byte & 0x1F;	// mask the lower 5 bits
		accelYFT = byte & 0xE0;
		return 1;
	}

	return 0;
}

/*
 * WIll attemp to read the MPU6050_RA_Y_SELF_TEST register and acquire the factory trim
 * value for the y gyro and accel. Will return true if the read is successful
 */
unsigned char GyroAccelZSelfTest()
{
	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_Z_SELF_TEST, &byte))
	{
		gyroZFT = byte & 0x1F;	// mask the lower 5 bits
		accelZFT = byte & 0xE0;
		return 1;
	}

	return 0;
}

/*
 * Will attempt to write to the MPU6050_RA_INT_ENABLE register and enable whichever interrupt
 * is passed in through the argument inter. Will return true if the interrupt was enabled.
 */
unsigned char EnableMPUInterrupt(unsigned char inter)
{
	if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_INT_ENABLE, &byte))
	{
		byte |= inter;
		if (WriteI2CBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_INT_ENABLE, &byte, 1))
		{
			return 1;
		}
	}

	return 0;
}

/*
 * Will attempt to read the MPU6050_RA_INT_STATUS register and will return its value.
 */
unsigned char GetMPUIntStatus()
{
	ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_INT_STATUS, &byte);
	return byte;
}

/*
 * Will attemp to burst read the GRYO_OUT registers.
 * Returns true if the read was a success and false otherwise.
 */
unsigned char GetGyroVals()
{
	for (gyroDataNdx = 0; gyroDataNdx < 6; gyroDataNdx++)
	{
		if (ReadI2CByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_GYRO_XOUT_H + gyroDataNdx, &byte))
		{
			gyroData[gyroDataNdx] = byte;
		}
		else
		{
			return 0;
		}
	}
}
