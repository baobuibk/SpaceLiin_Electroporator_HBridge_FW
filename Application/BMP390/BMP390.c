/*
 * BMP390.c
 *
 *  Created on: Oct 3, 2024
 *      Author: Administrator
 */
#include "BMP390.h"
#include <string.h>
#include <math.h>
#define atmPress 101325
static uint8_t read_uncompensated_value(uint32_t *pressure, uint32_t *temperature) ;
static double BMP309_temperature_compensate(uint32_t uncomp_temp, data *pBMP_data);
static double BMP_pressure_compensate(uint32_t uncomp_pressure, data *pBMP_data);
int16_t combine(uint8_t x, uint8_t y);
static float BMP390_GetAlt(void);
extern I2C_HandleTypeDef hi2c2;
static data BMP_data;
static uint8_t buffer[100];
static uint8_t buffer1[20];
uint32_t raw_temperature;
uint32_t raw_pressure;
double compensated_pressure;
double compensated_temperature;
Accel_Gyro_DataTypedef _gyro, _accel;
float altitudeValue;
static uint8_t read_uncompensated_value(uint32_t *pressure, uint32_t *temperature) {
	uint8_t status;
	 status= HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x04, 1, buffer , 6 , 100);
	 if (status == 0) {
		 *pressure =(uint32_t) ((buffer[2] <<16) | (buffer[1] <<8) | buffer[0]);

		 *temperature =(uint32_t) ((buffer[5] <<16) | (buffer[4] <<8) | buffer[3]);
	 }
	 return status;
}

void BMP390_init() {

	memset(&BMP_data, 0, sizeof(data));
	buffer[0] = 11;
	HAL_I2C_Mem_Write(&hi2c2, 0xEE, 0x1C, 1, buffer , 1 , 100);  //set oversampling
	HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x1C, 1, buffer , 1 , 100);


		HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x1F, 1, buffer , 1 , 100);  //set filter
		buffer[0] |= (1<<3);
		buffer[0] |= (1<<2);
		buffer[0] |= (1<<1);
		HAL_I2C_Mem_Write(&hi2c2, 0xEE, 0x1F, 1, buffer , 1 , 100);
		HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x1F, 1, buffer , 1 , 100);

	HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x31, 1, buffer , 21 , 100);    // read data
	BMP_data.uncompensatedData.NVM_PAR_T1 = (buffer[1] <<8) | buffer[0];
	BMP_data.uncompensatedData.NVM_PAR_T2 = (buffer[3] <<8) | buffer[2];
	BMP_data.uncompensatedData.NVM_PAR_T3 = buffer[4];
	BMP_data.uncompensatedData.NVM_PAR_P1 = (buffer[6] <<8) | buffer[5];
	BMP_data.uncompensatedData.NVM_PAR_P2 = (buffer[8] <<8) | buffer[7];
	BMP_data.uncompensatedData.NVM_PAR_P3 = buffer[9];
	BMP_data.uncompensatedData.NVM_PAR_P4 = buffer[10];
	BMP_data.uncompensatedData.NVM_PAR_P5 = (buffer[12] <<8) | buffer[11];
	BMP_data.uncompensatedData.NVM_PAR_P6 = (buffer[14] <<8) | buffer[13];
	BMP_data.uncompensatedData.NVM_PAR_P7 =  buffer[15];
	BMP_data.uncompensatedData.NVM_PAR_P8 = buffer[16];
	BMP_data.uncompensatedData.NVM_PAR_P9 = (buffer[18] <<8) | buffer[17];
	BMP_data.uncompensatedData.NVM_PAR_P10 = buffer[19];
	BMP_data.uncompensatedData.NVM_PAR_P11 =  buffer[20];
	double temp_var;
	 temp_var = 0.00390625f;
	 BMP_data.compensatedData.PAR_T1 = ((double)(BMP_data.uncompensatedData.NVM_PAR_T1)/temp_var);
	 temp_var = 1073741824.0f;
	 BMP_data.compensatedData.PAR_T2 = ((double)(BMP_data.uncompensatedData.NVM_PAR_T2)/temp_var);
	 temp_var = 281474976710656.0f;

	 BMP_data.compensatedData.PAR_T3 = ((double)(BMP_data.uncompensatedData.NVM_PAR_T3)/temp_var);
	 temp_var = 1048576.0f;

	 BMP_data.compensatedData.PAR_P1 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P1 - 16384)/temp_var);
	 temp_var = 536870912.0f;

	 BMP_data.compensatedData.PAR_P2 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P2 - 16384)/temp_var);

	 temp_var = 4294967296.0f;

	 BMP_data.compensatedData.PAR_P3 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P3)/temp_var);

	 temp_var = 137438953472.0f;

	 BMP_data.compensatedData.PAR_P4 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P4)/temp_var);

	 temp_var = 0.125f;

	 BMP_data.compensatedData.PAR_P5 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P5)/temp_var);

	 temp_var = 64.0f;

	 BMP_data.compensatedData.PAR_P6 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P6)/temp_var);

	 temp_var = 256.0f;

	 BMP_data.compensatedData.PAR_P7 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P7)/temp_var);

	 temp_var = 32768.0f;

	 BMP_data.compensatedData.PAR_P8 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P8)/temp_var);


	 temp_var = 281474976710656.0f;


	 BMP_data.compensatedData.PAR_P9 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P9)/temp_var);

	 BMP_data.compensatedData.PAR_P10 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P10)/temp_var);

	 temp_var = 36893488147419103232.0f;

	 BMP_data.compensatedData.PAR_P11 = ((double)(BMP_data.uncompensatedData.NVM_PAR_P11)/temp_var);

	//init LMD6 (0xD4 address)
	 HAL_I2C_Mem_Read(&hi2c2, 0xD4, 0x0f, 1, buffer1 , 1 , 100);
	 buffer1[0] = 0x58;
	HAL_I2C_Mem_Write(&hi2c2, 0xD4, 0x10, 1, buffer1 , 1 , 100);

	 buffer1[0] = 0x54;
	HAL_I2C_Mem_Write(&hi2c2, 0xD4, 0x11, 1, buffer1 , 1 , 100);
}
static double BMP309_temperature_compensate(uint32_t uncomp_temp, data *pBMP_data) {
	double partial_data1;
	double partial_data2;

	partial_data1 = (double)(uncomp_temp - pBMP_data->compensatedData.PAR_T1);
	partial_data2  = (double) (partial_data1 * pBMP_data->compensatedData.PAR_T2);

	pBMP_data->t_lin = partial_data2 + (partial_data1 * partial_data1) * pBMP_data->compensatedData.PAR_T3;
	compensated_temperature = pBMP_data->t_lin;
	return pBMP_data->t_lin;

}
static double BMP_pressure_compensate(uint32_t uncomp_pressure, data *pBMP_data) {
	double comp_pres;
	double partial_data1;
	double partial_data2;
	double partial_data3;
	double partial_data4;
	double partial_out1;
	double partial_out2;

	partial_data1= pBMP_data->compensatedData.PAR_P6 * pBMP_data->t_lin;
	partial_data2 = pBMP_data->compensatedData.PAR_P7 * (pBMP_data->t_lin * pBMP_data->t_lin);
	partial_data3 = pBMP_data->compensatedData.PAR_P8 * (pBMP_data->t_lin * pBMP_data->t_lin * pBMP_data->t_lin);
	partial_out1= pBMP_data->compensatedData.PAR_P5 + partial_data1 + partial_data2 + partial_data3;

	partial_data1= pBMP_data->compensatedData.PAR_P2 * pBMP_data->t_lin;
	partial_data2= pBMP_data->compensatedData.PAR_P3 * (pBMP_data->t_lin * pBMP_data->t_lin);
	partial_data3= pBMP_data->compensatedData.PAR_P4 * (pBMP_data->t_lin * pBMP_data->t_lin * pBMP_data->t_lin);
	partial_out2= (double)uncomp_pressure * (pBMP_data->compensatedData.PAR_P1 + partial_data1 + partial_data2+ partial_data3);

	partial_data1 = (double)uncomp_pressure * (double)uncomp_pressure;
	partial_data2= pBMP_data->compensatedData.PAR_P9 + pBMP_data->compensatedData.PAR_P10 * pBMP_data->t_lin;
	partial_data3= partial_data1 * partial_data2;
	partial_data4= partial_data3 + ((double)uncomp_pressure * (double)uncomp_pressure * (double)uncomp_pressure)* pBMP_data->compensatedData.PAR_P11;

	comp_pres = partial_out1 + partial_out2 + partial_data4;
	return comp_pres;



}

static float BMP390_GetAlt ()
{

	return 44330*(1-(pow(((float)compensated_pressure/(float)atmPress), 0.19029495718)));
}
void BMP390_Task(void *) {

	//1. write 0x33 to register 0x1B
	//2. if bit 5,6, of status register
	//3. compensate the value
	//1
	buffer[0] = 0x13;
	HAL_I2C_Mem_Write(&hi2c2, 0xEE, 0x1B, 1, buffer , 1 , 100);
	HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x1B, 1, buffer , 1 , 100);
	//2
	HAL_I2C_Mem_Read(&hi2c2, 0xEE, 0x03, 1, buffer , 1 , 100);
	if (buffer[0] & (1<<6)) {

		read_uncompensated_value(&raw_pressure, &raw_temperature);
		BMP309_temperature_compensate(raw_temperature, &BMP_data);
		compensated_pressure = BMP_pressure_compensate(raw_pressure, &BMP_data);
		altitudeValue= BMP390_GetAlt();
	}
	//2
	HAL_I2C_Mem_Read(&hi2c2, 0xD4, 0x1E, 1, buffer1 , 1 , 100);
	if ((buffer1[0] & (1<<0)) &&( buffer1[0] & (1<<1))) {       //if new data available




	HAL_I2C_Mem_Read(&hi2c2, 0xD4, 0x22, 1, buffer1 , 12 , 100);
	_gyro.x = (int)(combine(buffer1[1], buffer1[0]))* GYRO_SENSITIVITY_500DPS ;
	_gyro.y = (int)(combine(buffer1[3], buffer1[2])) * GYRO_SENSITIVITY_500DPS;
	_gyro.z = (int)(combine(buffer1[5], buffer1[4])) * GYRO_SENSITIVITY_500DPS;
	_accel.x =(int)(combine(buffer1[7], buffer1[6])) * LSM6DSOX_ACCL_FS_8G;
	_accel.y = (int)(combine(buffer1[9], buffer1[8]))  * LSM6DSOX_ACCL_FS_8G;
	_accel.z = (int)(combine(buffer1[11], buffer1[10]))* LSM6DSOX_ACCL_FS_8G;

	}
}
int16_t combine(uint8_t x, uint8_t y) {

	return (x<<8) | y;

}

























