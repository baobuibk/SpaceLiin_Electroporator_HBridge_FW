/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "app.h"
#include "stm32f0xx_ll_gpio.h"

#include "h_bridge_driver.h"
#include "v_switch_driver.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef enum
{
    H_BRIDGE_STOP_STATE,
    H_BRIDGE_HV_1_STATE,
    H_BRDIGE_HV_2_STATE,
    H_BRIDGE_LV_1_STATE,
    H_BRIDGE_LV_2_STATE,
} H_Bridge_State_typedef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
H_Bridge_State_typedef H_Bridge_State = H_BRIDGE_STOP_STATE;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
bool        is_h_bridge_enable          = false;

uint8_t     pulse_delay_ms              = 2;

uint8_t     hv_pulse_count              = 0;
uint8_t     hv_on_time_ms               = 1;
uint8_t     hv_off_time_ms              = 1;

uint8_t     lv_pulse_count              = 0;
uint8_t     lv_on_time_ms               = 1;
uint8_t     lv_off_time_ms              = 1;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: H Bridge Task Init :::::::: */
void H_Bridge_Task_Init(void)
{
    ;
}

void H_Bridge_Pulse_Control(bool pulse_state)
{
    is_h_bridge_enable = true;
    SchedulerTaskEnable(0, 1);
}

void H_Bridge_Set_HV_Timing(uint8_t on_time_ms, uint8_t off_time_ms)
{
    ;
}

/* :::::::::: H Bridge Task ::::::::::::: */
void H_Bridge_Task(void*)
{
    switch (H_Bridge_State)
    {
    case H_BRIDGE_STOP_STATE:
        if(is_h_bridge_enable == false)
        {
            V_Switch_Set_Mode(V_SWITCH_MODE_ALL_OFF);

            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);

            SchedulerTaskDisable(0);
        }
        else if(is_h_bridge_enable == true)
        {
            V_Switch_Set_Mode(V_SWITCH_MODE_HV_ON);

            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Pulse_Timing(&H_Bridge_1, pulse_delay_ms, hv_on_time_ms, hv_off_time_ms, hv_pulse_count);
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_PULSE);

            H_Bridge_State = H_BRIDGE_HV_1_STATE;
        }
        break;
    case H_BRIDGE_HV_1_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State  = H_BRIDGE_STOP_STATE;
        }
        else if(H_Bridge_1.pulse_count >= (H_Bridge_1.set_pulse_count * 2))
        {
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Pulse_Timing(&H_Bridge_2, pulse_delay_ms, hv_on_time_ms, hv_off_time_ms, hv_pulse_count);
            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_PULSE);

            H_Bridge_State = H_BRDIGE_HV_2_STATE;
        }
        break;
    case H_BRDIGE_HV_2_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        else if(H_Bridge_2.pulse_count >= (H_Bridge_2.set_pulse_count * 2))
        {
            V_Switch_Set_Mode(V_SWITCH_MODE_LV_ON);

            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Pulse_Timing(&H_Bridge_1, pulse_delay_ms, lv_on_time_ms, lv_off_time_ms, lv_pulse_count);
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_PULSE);

            H_Bridge_State = H_BRIDGE_LV_1_STATE;
        }
        break;
    case H_BRIDGE_LV_1_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        else if(H_Bridge_1.pulse_count >= (H_Bridge_1.set_pulse_count * 2))
        {
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Pulse_Timing(&H_Bridge_2, pulse_delay_ms, lv_on_time_ms, lv_off_time_ms, lv_pulse_count);
            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_PULSE);

            H_Bridge_State = H_BRIDGE_LV_2_STATE;
        }
        break;
    case H_BRIDGE_LV_2_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        else if(H_Bridge_2.pulse_count >= (H_Bridge_2.set_pulse_count * 2))
        {
            /*
            V_Switch_Set_Mode(V_SWITCH_MODE_HV_ON);

            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Pulse_Timing(&H_Bridge_1, pulse_delay_ms, hv_on_time_ms, hv_off_time_ms, hv_pulse_count);
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_PULSE);

            H_Bridge_State = H_BRIDGE_HV_1_STATE;
            */
        
            is_h_bridge_enable = false;
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        break;

    default:
        break;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
