/*
 * I2C.h
 *
 *  Created on: Jun 6, 2015
 *      Author: Warren Woolsey
 */

#ifndef I2C_H_
#define I2C_H_

#define I2C_TX_TIMEOUT	100

void InitI2C(void);
unsigned char StartTransmission(unsigned char slaveAddr, unsigned char mpuRegAddr);
unsigned char WriteI2CBytes(unsigned char slaveAddr, unsigned char mpuRegAddr, unsigned char *txBytes, unsigned char len);
unsigned char ReadI2CBytes(unsigned char slaveAddr, unsigned char mpuRegAddr, unsigned char *rxBytes, unsigned char len);
unsigned char ReadI2CByte(unsigned char slaveAddr, unsigned char mpuRegAddr, unsigned char *rxBytes);
void SetI2CTime(unsigned int val);
unsigned int GetI2CTime();
#endif /* I2C_H_ */
