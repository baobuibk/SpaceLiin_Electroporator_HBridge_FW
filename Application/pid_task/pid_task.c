/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#include "pid.h"
#include "pwm.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* :::::::::: PID Task State ::::::::::::: */
typedef enum
{
    PID_CAP_CHARGE_STATE,
    PID_H_BRIDGE_STATE,
} PID_State_typedef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static PID_State_typedef PID_State;

static PWM_TypeDef 	Flyback_300V_Switching_PWM;
static PWM_TypeDef	Flyback_50V_Switching_PWM;

static uint8_t 		PID_300V_PWM_duty;
static uint16_t 	PID_300V_set_voltage = 2850; //300
//static uint16_t PID_set_voltage = 2350; //250
//static uint16_t PID_set_voltage = 936; //100V
//static uint16_t PID_set_voltage = 1236; //130V
//static uint16_t PID_set_voltage = 1436; //150V
//static uint16_t PID_set_voltage = 636; //70V
//static uint16_t PID_set_voltage = 1898; //200V

static uint8_t 		PID_50V_PWM_duty;
static uint16_t 	PID_50V_set_voltage = 436; //50

static PID_TypeDef Charge_300V_Cap_PID =
{
	.PID_Mode 		= 	_PID_MODE_AUTOMATIC,
	.PON_Type 		= 	_PID_P_ON_E,
	.PID_Direction 	=	_PID_CD_DIRECT,
	//.Kp				= 	(0.04 + 0.36),
	.Kp				= 	(0.04 + 0.26),
	.Ki				= 	0.001,
	.Kd 			=	0.0,
	.MyInput		=	&g_Feedback_Voltage[0],
	.MyOutput		= 	&PID_300V_PWM_duty,
	.MySetpoint		=	&PID_300V_set_voltage,
	//.Output_Min		= 	10,
	.Output_Min		= 	6,
	//.Output_Max		=	10,
	.Output_Max		=	40,
	//.Output_Min		= 	18,
	//.Output_Max		=	50,
};

static PID_TypeDef Charge_50V_Cap_PID =
{
	.PID_Mode 		= 	_PID_MODE_AUTOMATIC,
	.PON_Type 		= 	_PID_P_ON_E,
	.PID_Direction 	=	_PID_CD_DIRECT,
	//.Kp				= 	(0.04 + 0.36),
	.Kp				= 	(0.04 + 0.26),
	.Ki				= 	0.001,
	.Kd 			=	0.0,
	.MyInput		=	&g_Feedback_Voltage[1],
	.MyOutput		= 	&PID_50V_PWM_duty,
	.MySetpoint		=	&PID_50V_set_voltage,
	//.Output_Min		= 	10,
	.Output_Min		= 	6,
	//.Output_Max		=	10,
	.Output_Max		=	40,
	//.Output_Min		= 	18,
	//.Output_Max		=	50,
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: PID Init ::::::::::::: */
void PID_Task_Init(void)
{
	PID_Init(&Charge_300V_Cap_PID);
	PID_Init(&Charge_50V_Cap_PID);

	PWM_Init(	&Flyback_300V_Switching_PWM, FLYBACK_SW1_HANDLE, FLYBACK_SW1_CHANNEL,
				LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_LOW);

	PWM_Set_Freq(&Flyback_300V_Switching_PWM, 60000);
    PWM_Enable(&Flyback_300V_Switching_PWM);

	PWM_Init(	&Flyback_50V_Switching_PWM, FLYBACK_SW2_HANDLE, FLYBACK_SW2_CHANNEL,
				LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_LOW);

	PWM_Set_Freq(&Flyback_50V_Switching_PWM, 60000);
    PWM_Enable(&Flyback_50V_Switching_PWM);
}

/* :::::::::: PID Task ::::::::::::: */
//TODO: Adaptive PID, Ki, or sync uart command to change Ki.
void PID_Task(void*)
{
    switch (PID_State)
    {
    case PID_CAP_CHARGE_STATE:
		/*
		if ((g_Feedback_Voltage >= PID_set_voltage) && (H_Bridge_State == H_BRIDGE_POS_STATE))
        {
            PID_State = PID_H_BRIDGE_STATE;
            PID_SetTunings1(&Charge_Cap_PID, (0.04 + 0.26), 0.03, 0.0);
			//PID_SetTunings1(&Charge_Cap_PID, (0.05 + 0.35), 0.03, 0.0);
			//PID_SetOutputLimits(&Charge_Cap_PID, 10, 40);
            SchedulerTaskEnable(2, true);
            break;
        }
		*/
        
        PID_Compute(&Charge_300V_Cap_PID);
        PWM_Set_Duty(&Flyback_300V_Switching_PWM, PID_300V_PWM_duty);

		PID_Compute(&Charge_50V_Cap_PID);
        PWM_Set_Duty(&Flyback_50V_Switching_PWM, PID_50V_PWM_duty);
        break;
    case PID_H_BRIDGE_STATE:
		/*
		if(H_Bridge_State == H_BRIDGE_WAIT)
		{
			PID_State = PID_CAP_CHARGE_STATE;
			PID_SetTunings1(&Charge_Cap_PID, (0.04 + 0.26), 0.001, 0.0);
			//PID_SetOutputLimits(&Charge_Cap_PID, 5, 40);
			break;
		}

        PID_Compute(&Charge_Cap_PID);
        PWM_Set_Duty(&Switching_PWM, PID_PWM_duty);
		*/

        break;

    default:
        break;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
