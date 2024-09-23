/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "app.h"

#include "h_bridge_task.h"

#include "scheduler.h"
#include "pwm.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
PWM_TypeDef H_Bridge_1_PWM;
PWM_TypeDef H_Bridge_2_PWM;
PWM_TypeDef V_Switch_1_PWM;
PWM_TypeDef V_Switch_2_PWM;

H_Bridge_State_typedef H_Bridge_State = H_BRIDGE_STOP_STATE;

static          bool        is_h_bridge_set_up      = false;

volatile        uint16_t    high_side_pulse_count   = 0;
volatile        uint16_t    low_side_pulse_count    = 0;

static volatile bool        is_5s_set_up            = false;
static volatile bool        is_5s_finished          = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
bool        is_h_bridge_enable          = false;

uint8_t     hs_on_time_ms               = 1;
uint8_t     hs_off_time_ms              = 1;
uint8_t     hs_duty                     = 0;
uint32_t    hs_freq                     = 0;

uint8_t     ls_on_time_ms               = 1;
uint8_t     ls_off_time_ms              = 1;
uint8_t     ls_duty                     = 0;
uint32_t    ls_freq                     = 0;

uint16_t    high_side_set_pulse_count   = 0;
uint16_t    low_side_set_pulse_count    = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: H Bridge Task Init :::::::: */
void H_Bridge_Task_Init(void)
{
    // H bridge highside
    PWM_Init(&H_Bridge_1_PWM, H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_HIGH);
    PWM_Disable(&H_Bridge_1_PWM);
    LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);

    // Enable Counter interupt for H bridge highside
    //LL_TIM_SetUpdateSource(H_Bridge_1_PWM.TIMx, LL_TIM_UPDATESOURCE_COUNTER);
    //LL_TIM_EnableUpdateEvent(H_Bridge_1_PWM.TIMx);
    LL_TIM_DisableIT_CC1(H_Bridge_1_PWM.TIMx);

    // H bridge lowside
    PWM_Init(&H_Bridge_2_PWM, H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_HIGH);
    PWM_Disable(&H_Bridge_2_PWM);
    LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);

    // Enable Counter interupt for H bridge lowside
    //LL_TIM_SetUpdateSource(H_Bridge_2_PWM.TIMx, LL_TIM_UPDATESOURCE_COUNTER);
    //LL_TIM_EnableUpdateEvent(H_Bridge_2_PWM.TIMx);
    //LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);
    LL_TIM_DisableIT_CC2(H_Bridge_1_PWM.TIMx);

    LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
    LL_GPIO_SetOutputPin(V_SWITCH_HIN1_PORT, V_SWITCH_HIN1_PIN);
    PWM_Init(&V_Switch_1_PWM, V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_HIGH);
    PWM_Set_Freq(&V_Switch_1_PWM, 10000);
    PWM_Set_Duty(&V_Switch_1_PWM, 50);
    PWM_Enable(&V_Switch_1_PWM);
}

/* :::::::::: H Bridge Task ::::::::::::: */
void H_Bridge_Task(void*)
{
    switch (H_Bridge_State)
    {
    case H_BRIDGE_STOP_STATE:
        // Disable Update Interupt
        LL_TIM_DisableIT_CC1(H_Bridge_1_PWM.TIMx);
        LL_TIM_DisableIT_CC2(H_Bridge_2_PWM.TIMx);

        // TURN OFF LOW AND HIGH FIRST
        PWM_Set_Duty(&H_Bridge_1_PWM, 0);
        PWM_Set_Duty(&H_Bridge_2_PWM, 0);

        // LOW AND HIGH SIDE GND ON
        LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
        LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);

        // DISABLE PWM
        PWM_Disable(&H_Bridge_1_PWM);
        PWM_Disable(&H_Bridge_2_PWM);

        high_side_pulse_count = 0;
        is_h_bridge_set_up = false;

        if(is_h_bridge_enable == true)
        {
            H_Bridge_State = H_BRIDGE_1_STATE;
        }
        break;
    case H_BRIDGE_1_STATE:
        if(is_h_bridge_set_up == false)
        {
            // TURN OFF LOW AND HIGH FIRST
            //PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            //PWM_Set_Duty(&H_Bridge_2_PWM, 0);

            // HB2 GND ON
            LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);

            // HB1 300 ON
            LL_GPIO_SetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);

            // Turn HB2 on, no switching
            PWM_Set_Duty(&H_Bridge_2_PWM, 100);

            // Turn HB1 on, switching @ 50% duty
            //PWM_Set_Duty(&H_Bridge_1_PWM, 50);
            PWM_Set_Duty(&H_Bridge_1_PWM, hs_duty);

            // Enable Update Interupt
            LL_TIM_EnableIT_CC1(H_Bridge_1_PWM.TIMx);

            // Enable Low and High side on
            PWM_Enable(&H_Bridge_2_PWM);
            PWM_Enable(&H_Bridge_1_PWM);

            // Enable the flag
            is_h_bridge_set_up = true;
        }
        //else if(high_side_pulse_count == high_side_set_pulse_count)
        //{
            // Disable Update Interupt
            //LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            //PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            //PWM_Set_Duty(&H_Bridge_2_PWM, 0);

            // LOW AND HIGH SIDE GND ON
            //LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
            //LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);

            // DISABLE PWM
            //PWM_Disable(&H_Bridge_1_PWM);
            //PWM_Disable(&H_Bridge_2_PWM);

            //high_side_pulse_count = 0;
            //is_h_bridge_set_up = false;
            //H_Bridge_State = H_BRDIGE_2_STATE;
        //}
        else if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        break;
    case H_BRDIGE_2_STATE:
        if (is_h_bridge_set_up == false)
        {
            // TURN OFF LOW AND HIGH FIRST
            //PWM_Set_Duty(&H_Bridge_2_PWM, 0);
            //PWM_Set_Duty(&H_Bridge_1_PWM, 0);

            // HIGH SIDE GND ON
            LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);

            // LOW SIDE 300 ON
            LL_GPIO_SetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);

            // Turn High side on, no switching
            PWM_Set_Duty(&H_Bridge_1_PWM, 100);

            // Turn Low side on, switching @ 50% duty
            //PWM_Set_Duty(&H_Bridge_2_PWM, 50);
            PWM_Set_Duty(&H_Bridge_2_PWM, ls_duty);

            // Enable Update Interupt
            LL_TIM_EnableIT_CC2(H_Bridge_2_PWM.TIMx);

            // Enable Low and High side on
            PWM_Enable(&H_Bridge_1_PWM);
            PWM_Enable(&H_Bridge_2_PWM);

            // Enable the flag
            is_h_bridge_set_up = true;
        }
        //else if(low_side_pulse_count == low_side_set_pulse_count)
        //{
            // Disable Update Interupt
            //LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            //PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            //PWM_Set_Duty(&H_Bridge_2_PWM, 0);

            // LOW AND HIGH SIDE GND ON
            //LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
            //LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);

            // DISABLE PWM
            //PWM_Disable(&H_Bridge_1_PWM);
            //PWM_Disable(&H_Bridge_2_PWM);

            //low_side_pulse_count = 0;
            //is_h_bridge_set_up = false;
            //H_Bridge_State = H_BRIDGE_1_STATE;

            //SchedulerTaskDisable(2);
        //}
        else if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        break;

    default:
        break;
    }
}

/* ::::H_Bridge 1 Interupt Handle:::: */
void H_Bridge_1_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_CC1(H_Bridge_1_PWM.TIMx) == true)
    {
        high_side_pulse_count++;
        LL_TIM_ClearFlag_CC1(H_Bridge_1_PWM.TIMx);

        if (high_side_pulse_count == high_side_set_pulse_count)
        {
            LL_TIM_DisableIT_CC1(H_Bridge_1_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            PWM_Set_Duty(&H_Bridge_2_PWM, 0);

            // DISABLE PWM
            PWM_Disable(&H_Bridge_1_PWM);
            PWM_Disable(&H_Bridge_2_PWM);

            high_side_pulse_count = 0;
            is_h_bridge_set_up = false;
            H_Bridge_State = H_BRDIGE_2_STATE;
        }
        
    }
}

/* ::::H_Bridge 2 Interupt Handle:::: */
void H_Bridge_2_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_CC2(H_Bridge_2_PWM.TIMx) == true)
    {
        low_side_pulse_count++;
        LL_TIM_ClearFlag_CC2(H_Bridge_2_PWM.TIMx);

        if (low_side_pulse_count == low_side_set_pulse_count)
        {
            LL_TIM_DisableIT_CC2(H_Bridge_2_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            PWM_Set_Duty(&H_Bridge_2_PWM, 0);
            PWM_Set_Duty(&H_Bridge_2_PWM, 0);

            // DISABLE PWM
            PWM_Disable(&H_Bridge_1_PWM);
            PWM_Disable(&H_Bridge_2_PWM);

            low_side_pulse_count = 0;
            is_h_bridge_set_up = false;
            H_Bridge_State = H_BRIDGE_1_STATE;
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
