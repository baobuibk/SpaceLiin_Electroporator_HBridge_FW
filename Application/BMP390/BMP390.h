/*
 * BMP390.h
 *
 *  Created on: Oct 3, 2024
 *      Author: Administrator
 */

#ifndef BMP390_BMP390_H_
#define BMP390_BMP390_H_
#include "main.h"
typedef struct _uncompensated_data_ {

	uint16_t NVM_PAR_T1;
	uint16_t NVM_PAR_T2;
	uint16_t NVM_PAR_T3;
	uint16_t NVM_PAR_P1;
	uint16_t NVM_PAR_P2;
	uint16_t NVM_PAR_P3;
	uint16_t NVM_PAR_P4;
	uint16_t NVM_PAR_P5;
	uint16_t NVM_PAR_P6;
	uint16_t NVM_PAR_P7;
	uint16_t NVM_PAR_P8;
	uint16_t NVM_PAR_P9;
	uint16_t NVM_PAR_P10;
	uint16_t NVM_PAR_P11;



} uncompensated_data;
typedef struct _compensated_data_ {

	double PAR_T1;
	double PAR_T2;
	double PAR_T3;
	double PAR_P1;
	double PAR_P2;
	double PAR_P3;
	double PAR_P4;
	double PAR_P5;
	double PAR_P6;
	double PAR_P7;
	double PAR_P8;
	double PAR_P9;
	double PAR_P10;
	double PAR_P11;


}  compensated_data;
typedef struct _data_ {
	uncompensated_data uncompensatedData;
	compensated_data compensatedData;
	double t_lin;
} data;
#define GYRO_SENSITIVITY_500DPS		17.50f
#define LSM6DSOX_ACCL_FS_8G			0.244f

typedef struct _Accel_Gyro_DataTypedef_
{
	int x;
	int y;
	int z;
} Accel_Gyro_DataTypedef;
void BMP390_init();
void BMP390_Task(void*) ;

#endif /* BMP390_BMP390_H_ */
