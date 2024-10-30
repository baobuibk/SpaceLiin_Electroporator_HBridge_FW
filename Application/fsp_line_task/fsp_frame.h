#ifndef _FSP_FRAME_H

#define _FSP_FRAME_H

// Commands
// FROM GPC TO GPP
#define FSP_CMD_SET_PULSE_COUNT         0x01       /**< Set number of pulse. */		//
#define FSP_CMD_SET_PULSE_DELAY         0x02
#define FSP_CMD_SET_PULSE_HV	        0x03       /**< Set hs pulse on time and off time. */
#define FSP_CMD_SET_PULSE_LV            0x04       /**< Set ls pulse on time and off time. */
#define FSP_CMD_SET_PULSE_CONTROL		0x05       /**< Start pulsing. */

#define FSP_CMD_GET_PULSE_COUNT         0x06       /**< Set number of pulse. */		//
#define FSP_CMD_GET_PULSE_DELAY         0x07
#define FSP_CMD_GET_PULSE_HV	        0x08       /**< Set hs pulse on time and off time. */
#define FSP_CMD_GET_PULSE_LV            0x09       /**< Set ls pulse on time and off time. */
#define FSP_CMD_GET_PULSE_CONTROL		0x0A       /**< Start pulsing. */
#define FSP_CMD_GET_PULSE_ALL			0x0B

#define FSP_CMD_SET_RELAY_POLE	        0x0C       /**< Stop pulsing. */
#define FSP_CMD_SET_RELAY_CONTROL       0x0D       /**< Stop cuvette. */

#define FSP_CMD_GET_RELAY_POLE	        0x0E       /**< Stop pulsing. */
#define FSP_CMD_GET_RELAY_CONTROL       0x0F       /**< Stop cuvette. */
#define FSP_CMD_GET_RELAY_ALL       	0x10       /**< Stop cuvette. */

#define FSP_CMD_CHANNEL_SET             0x11
#define FSP_CMD_CHANNEL_CONTROL         0x12
#define FSP_CMD_GET_CURRENT				0x13		//Get current
#define FSP_CMD_GET_IMPEDANCE			0x14

// FROM GPP TO GPC
#define FSP_CMD_SEND_PULSE_COUNT        0x15       /**< Set number of pulse. */		//
#define FSP_CMD_SEND_PULSE_DELAY        0x16
#define FSP_CMD_SEND_PULSE_HV	        0x17       /**< Set hs pulse on time and off time. */
#define FSP_CMD_SEND_PULSE_LV           0x18       /**< Set ls pulse on time and off time. */
#define FSP_CMD_SEND_PULSE_CONTROL		0x19       /**< Start pulsing. */
#define FSP_CMD_SEND_PULSE_ALL			0x1A       /**< Start pulsing. */

#define FSP_CMD_SEND_RELAY_POLE	        0x1B       /**< Stop pulsing. */
#define FSP_CMD_SEND_RELAY_CONTROL      0x1C       /**< Stop cuvette. */
#define FSP_CMD_SEND_RELAY_ALL      	0x1D       /**< Stop cuvette. */

#define FSP_CMD_HANDSHAKE				0x1E
#define FSP_CMD_SENT_CURRENT			0x1F
#define FSP_CMD_SENT_IMPEDANCE			0x20
#define FSP_CMD_GET_BMP390				0x21
#define FSP_CMD_GET_I2C_SENSOR			0x22
#define FSP_CMD_STOP_GET_SENSOR			0x23
typedef struct _COMMON_FRAME_
{
	uint8_t Cmd;
}COMMON_FRAME;

typedef struct _FSP_PULSE_COUNT_
{
	uint8_t Cmd;            /* The command class */
	uint8_t HV_pos_count;		/* hv pulse count */
	uint8_t HV_neg_count;		/* hv pulse count */
	uint8_t LV_pos_count;		/* lv pulse count */
	uint8_t LV_neg_count;		/* lv pulse count */
} FSP_PULSE_COUNT;

typedef struct _FSP_PULSE_DELAY_
{
	uint8_t Cmd;           	/* The command class */
	uint8_t HV_delay;
	uint8_t LV_delay;
	uint8_t Delay_high;		  	/* Delay time */
	uint8_t Delay_low;		  	/* Delay time */
} FSP_PULSE_DELAY;

typedef struct _FSP_PULSE_HV_
{
	uint8_t Cmd;            /* The command class */
	uint8_t OnTime;      	/* HV On time */
	uint8_t OffTime;      	/* HV Off time */
} FSP_PULSE_HV;

typedef struct _FSP_PULSE_LV_
{
	uint8_t Cmd;            /* The command class */
	uint8_t OnTime_high;      	/* LV On time */
	uint8_t OnTime_low;      	/* LV On time */
	uint8_t OffTime_high;      	/* LV Off time */
	uint8_t OffTime_low;      	/* LV Off time */
} FSP_PULSE_LV;

typedef struct _FSP_PULSE_CONTROL_
{
	uint8_t Cmd;            /* The command class */
	uint8_t State;      	/* 0: OFF, 1: ON */
} FSP_PULSE_CONTROL;

typedef struct _FSP_PULSE_ALL_
{
	uint8_t Cmd;            /* The command class */
	uint8_t HV_pos_count;		/* hv pulse count */
	uint8_t HV_neg_count;		/* hv pulse count */
	uint8_t LV_pos_count;		/* lv pulse count */
	uint8_t LV_neg_count;		/* lv pulse count */

	uint8_t HV_delay;
	uint8_t LV_delay;
	uint8_t Delay_high;		  	/* Delay time */
	uint8_t Delay_low;		  	/* Delay time */

	uint8_t OnTime;      	/* HV On time */
	uint8_t OffTime;      	/* HV Off time */

	uint8_t OnTime_high;      	/* LV On time */
	uint8_t OnTime_low;      	/* LV On time */
	uint8_t OffTime_high;      	/* LV Off time */
	uint8_t OffTime_low;      	/* LV Off time */

	uint8_t State;      	/* 0: OFF, 1: ON */
} FSP_PULSE_ALL;

typedef struct _FSP_RELAY_SET_
{
	uint8_t Cmd;            /* The command class */
	uint8_t HvRelay;      	/* Choosing HV RELAY*/
	uint8_t LvRelay;      	/* Choosing LV RELAY*/
} FSP_RELAY_SET;

typedef struct _FSP_RELAY_CONTROL_
{
	uint8_t Cmd;            /* The command class */
	uint8_t State;      	/* 0: OFF, 1: ON */
} FSP_RELAY_CONTROL;

typedef struct _FSP_RELAY_ALL_
{
	uint8_t Cmd;            /* The command class */
	uint8_t HvRelay;      	/* Choosing HV RELAY*/
	uint8_t LvRelay;      	/* Choosing LV RELAY*/

	uint8_t State;      	/* 0: OFF, 1: ON */
} FSP_RELAY_ALL;

typedef struct _FSP_CHANNEL_SET_
{
	uint8_t Cmd;            /* The command class */
	uint8_t Channel;      	/* Choosing output channel*/
} FSP_CHANNEL_SET;

typedef struct _FSP_CHANNEL_CONTROL_
{
	uint8_t Cmd;            /* The command class */
	uint8_t State;      	/* 0: OFF, 1: ON */
} FSP_CHANNEL_CONTROL;

typedef struct _FSP_GET_CURRENT_
{
	uint8_t Cmd;            /* The command class */
} FSP_GET_CURRENT;

typedef struct _FSP_GET_IMPEDANCE_
{
	uint8_t Cmd;            /* The command class */
	uint8_t Period;
} FSP_GET_IMPEDANCE;

typedef struct _FSP_HANDSAKE_
{
	uint8_t Cmd;
	uint8_t Check;
} FSP_HANDSAKE;

typedef struct _COMMON_RESPONSE_FRAME_
{
	uint8_t Cmd;            /* The command class */
} COMMON_RESPONSE_FRAME;

typedef struct _GET_CURRENT_FRAME_
{
	uint8_t 	Cmd;
	uint8_t 	Value_high;
	uint8_t 	Value_low;
} GET_CURRENT_FRAME;

typedef struct _GET_IMPEDANCE_FRAME_
{
	uint8_t 	Cmd;
	uint8_t 	Value_high;
	uint8_t 	Value_low;
} GET_IMPEDANCE_FRAME;

typedef struct _GET_BMP390_ {
	uint8_t Cmd;
	uint8_t temp[5];
	uint8_t pressure[6];
} GET_BMP390;
typedef struct _GET_I2C_SENSOR_ {
	uint8_t Cmd;
	uint8_t temp[5];
	uint8_t pressure[7];
	uint8_t accel_x[4];
	uint8_t accel_y[4];
	uint8_t accel_z[4];
	uint8_t gyro_x[4];
	uint8_t gyro_y[4];
	uint8_t gyro_z[4];

} GET_I2C_SENSOR;

// Union to encapsulate all frame types
typedef union _GPC_FSP_Payload_ {
	COMMON_FRAME							commonFrame;
	FSP_PULSE_COUNT							set_pulse_count;
	FSP_PULSE_DELAY							set_pulse_delay;
	FSP_PULSE_HV							set_pulse_HV;
	FSP_PULSE_LV							set_pulse_LV;
	FSP_PULSE_CONTROL						set_pulse_control;

	FSP_RELAY_SET							set_relay_pole;
	FSP_RELAY_CONTROL						set_relay_control;

	FSP_CHANNEL_SET							channelSet;
	FSP_CHANNEL_CONTROL						channelControl;
	FSP_GET_CURRENT							currentGet;
	FSP_GET_IMPEDANCE						get_impedance;
	FSP_HANDSAKE							handshake;
} GPC_FSP_Payload;

typedef union _GPP_FSP_Payload_ {
	COMMON_RESPONSE_FRAME					commonFrame;
	FSP_PULSE_COUNT							send_pulse_count;
	FSP_PULSE_DELAY							send_pulse_delay;
	FSP_PULSE_HV							send_pulse_HV;
	FSP_PULSE_LV							send_pulse_LV;
	FSP_PULSE_CONTROL						send_pulse_control;
	FSP_PULSE_ALL							send_pulse_all;

	FSP_RELAY_SET							send_relay_pole;
	FSP_RELAY_CONTROL						send_relay_control;
	FSP_RELAY_ALL							send_relay_all;

	FSP_HANDSAKE							handshake;
	GET_CURRENT_FRAME						get_current;
	GET_IMPEDANCE_FRAME						get_impedance;
	GET_BMP390								getBMP390;
	GET_I2C_SENSOR							GET_I2C_SENSOR;
} GPP_FSP_Payload;


#endif
