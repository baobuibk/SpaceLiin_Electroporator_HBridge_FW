#ifndef _FSP_FRAME_H

#define _FSP_FRAME_H

// Commands
#define FSP_CMD_PULSE_COUNT             0x01       /**< Set number of pulse. */		//
#define FSP_CMD_PULSE_DELAY             0x02
#define FSP_CMD_PULSE_HV	            0x03       /**< Set hs pulse on time and off time. */
#define FSP_CMD_PULSE_LV                0x04       /**< Set ls pulse on time and off time. */
#define FSP_CMD_PULSE_CONTROL		    0x05       /**< Start pulsing. */
#define FSP_CMD_RELAY_SET	            0x06       /**< Stop pulsing. */
#define FSP_CMD_RELAY_CONTROL           0x07       /**< Stop cuvette. */
#define FSP_CMD_CHANNEL_SET             0x08
#define FSP_CMD_CHANNEL_CONTROL         0x09
#define FSP_CMD_GET_CURRENT				0x0A	//Get current
#define FSP_CMD_HANDSHAKE				0x0B


typedef struct _COMMON_FRAME_
{
	uint8_t Cmd;
}COMMON_FRAME;

typedef struct _FSP_PULSE_COUNT_
{
	uint8_t Cmd;            /* The command class */
	uint8_t HV_count;		/* hv pulse count */
	uint8_t LV_count;		/* lv pulse count */
} FSP_PULSE_COUNT;

typedef struct _FSP_PULSE_DELAY_
{
	uint8_t Cmd;           	/* The command class */
	uint8_t Delay;		  	/* Delay time */
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
	uint8_t OnTime;      	/* LV On time */
	uint8_t OffTime;      	/* LV Off time */
} FSP_PULSE_LV;

typedef struct _FSP_PULSE_CONTROL_
{
	uint8_t Cmd;            /* The command class */
	uint8_t State;      	/* 0: OFF, 1: ON */
} FSP_PULSE_CONTROL;

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

typedef struct _FSP_HANDSAKE_
{
	uint8_t Cmd;
	uint8_t Check;
} FSP_HANDSAKE;

typedef struct _COMMON_RESPONSE_FRAME_
{
	uint8_t Cmd;            /* The command class */
} COMMON_RESPONSE_FRAME;

typedef struct _CURRENT_RESPONSE_FRAME_
{
	uint8_t Cmd;
	uint16_t Current;
} CURRENT_RESPONSE_FRAME;

// Union to encapsulate all frame types
typedef union _GPP_Sfp_Payload_ {
	COMMON_FRAME							commonFrame;
	FSP_PULSE_COUNT							pulseCount;
	FSP_PULSE_DELAY							pulseDelay;
	FSP_PULSE_HV							pulseHV;
	FSP_PULSE_LV							pulseLV;
	FSP_PULSE_CONTROL						pulseControl;
	FSP_RELAY_SET							relaySet;
	FSP_RELAY_CONTROL						relayControl;
	FSP_CHANNEL_SET							channelSet;
	FSP_CHANNEL_CONTROL						channelControl;
	FSP_GET_CURRENT							currentGet;
	FSP_HANDSAKE							handshake;
} GPC_Sfp_Payload;

typedef union _GPC_Sfp_Payload_ {
	COMMON_RESPONSE_FRAME					commonFrame;
	CURRENT_RESPONSE_FRAME					currentResponse;
	FSP_HANDSAKE							handshake;
} GPP_Sfp_Payload;


#endif
