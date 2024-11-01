/*
 * fsp_line_task.h
 *
 *  Created on: Sep 30, 2024
 *      Author: thanh
 */

#ifndef FSP_LINE_TASK_H_
#define FSP_LINE_TASK_H_

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Include ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "app.h"
#include "board.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fsp.h"
#include "crc.h"
#include "fsp_frame.h"
#include "stm32f0xx_ll_gpio.h"
#include "command.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

extern fsp_packet_t		    s_GPC_FSP_Packet;
extern fsp_packet_t		    s_GPP_FSP_Packet;
extern GPC_FSP_Payload  	*pu_GPC_FSP_Payload;		//for RX
extern GPP_FSP_Payload		*pu_GPP_FSP_Payload;		//for TX
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
void FSP_Line_Task_Init();

/* :::::::::: CMD Line Task ::::::::::::: */
void FSP_Line_Task(void);

/* :::::::::: IRQ Handler ::::::::::::: */
void GPC_UART_IRQHandler(void);

void FSP_Line_Process();
void send_i2c_sensor_data_task(void *);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#endif /* FSP_LINE_TASK_H_ */

