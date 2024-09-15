/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "h_bridge_task.h"

#include "scheduler.h"
#include "pwm.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
typedef enum
{
    H_BRIDGE_POS_STATE,
    H_BRDIGE_MINUS_STATE,
    H_BRIDGE_WAIT,
} H_Bridge_State_typedef;
*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
PWM_TypeDef HighSide_PWM;
PWM_TypeDef LowSide_PWM;

H_Bridge_State_typedef H_Bridge_State = H_BRIDGE_POS_STATE;

static          bool        is_h_bridge_enable      = false;
static volatile uint16_t    high_side_pulse_count   = 0;
static volatile uint16_t    low_side_pulse_count    = 0;
static volatile bool        is_5s_set_up            = false;
static volatile bool        is_5s_finished          = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: H Bridge Task Init :::::::: */
void H_Bridge_Task_Init(void)
{
    // H bridge lowside
    PWM_Init(&LowSide_PWM, TIM16, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_HIGH);
    //PWM_Set_Freq(&LowSide_PWM, 1000);
    PWM_Set_Freq(&LowSide_PWM, 100);
    PWM_Set_Duty(&LowSide_PWM, 0);
    PWM_Disable(&LowSide_PWM);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

    // Enable Counter interupt for H bridge lowside
    LL_TIM_SetUpdateSource(LowSide_PWM.TIMx, LL_TIM_UPDATESOURCE_COUNTER);
    LL_TIM_EnableUpdateEvent(LowSide_PWM.TIMx);
    LL_TIM_DisableIT_UPDATE(LowSide_PWM.TIMx);

    // H bridge highside
    PWM_Init(&HighSide_PWM, TIM17, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1, LL_TIM_OCPOLARITY_HIGH);
    //PWM_Set_Freq(&HighSide_PWM, 1000);
    PWM_Set_Freq(&HighSide_PWM, 100);
    PWM_Set_Duty(&HighSide_PWM, 0);
    PWM_Disable(&HighSide_PWM);
    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8);

    // Enable Counter interupt for H bridge highside
    LL_TIM_SetUpdateSource(HighSide_PWM.TIMx, LL_TIM_UPDATESOURCE_COUNTER);
    LL_TIM_EnableUpdateEvent(HighSide_PWM.TIMx);
    LL_TIM_DisableIT_UPDATE(HighSide_PWM.TIMx);

}

/* :::::::::: H Bridge Task ::::::::::::: */
void H_Bridge_Task(void)
{
    switch (H_Bridge_State)
    {
    case H_BRIDGE_POS_STATE:
        if(is_h_bridge_enable == false)
        {
            // TURN OFF LOW AND HIGH FIRST
            PWM_Set_Duty(&HighSide_PWM, 0);
            PWM_Set_Duty(&LowSide_PWM, 0);

            // LOW SIDE GND ON
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

            // HIGH SIDE 300 ON
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_8);

            // Turn Low side on, no switching
            PWM_Set_Duty(&LowSide_PWM, 100);

            // Turn High side on, switching @ 50% duty
            //PWM_Set_Duty(&HighSide_PWM, 50);
            PWM_Set_Duty(&HighSide_PWM, 50);

            // Enable Update Interupt
            LL_TIM_EnableIT_UPDATE(HighSide_PWM.TIMx);

            // Enable Low and High side on
            PWM_Enable(&LowSide_PWM);
            PWM_Enable(&HighSide_PWM);

            // Enable the flag
            is_h_bridge_enable = true;
        }
        else if(high_side_pulse_count >= 10)
        {
            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(HighSide_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            PWM_Set_Duty(&HighSide_PWM, 0);
            PWM_Set_Duty(&LowSide_PWM, 0);

            // LOW AND HIGH SIDE GND ON
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8);

            // DISABLE PWM
            PWM_Disable(&HighSide_PWM);
            PWM_Disable(&LowSide_PWM);

            high_side_pulse_count = 0;
            is_h_bridge_enable = false;
            H_Bridge_State = H_BRDIGE_MINUS_STATE;
        }
        break;
    case H_BRDIGE_MINUS_STATE:
        if (is_h_bridge_enable == false)
        {
            // TURN OFF LOW AND HIGH FIRST
            PWM_Set_Duty(&LowSide_PWM, 0);
            PWM_Set_Duty(&HighSide_PWM, 0);

            // HIGH SIDE GND ON
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8);

            // LOW SIDE 300 ON
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);

            // Turn High side on, no switching
            PWM_Set_Duty(&HighSide_PWM, 100);

            // Turn Low side on, switching @ 50% duty
            //PWM_Set_Duty(&LowSide_PWM, 50);
            PWM_Set_Duty(&LowSide_PWM, 50);

            // Enable Update Interupt
            LL_TIM_EnableIT_UPDATE(LowSide_PWM.TIMx);

            // Enable Low and High side on
            PWM_Enable(&HighSide_PWM);
            PWM_Enable(&LowSide_PWM);

            // Enable the flag
            is_h_bridge_enable = true;
        }
        else if(low_side_pulse_count >= 10)
        {
            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(LowSide_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            PWM_Set_Duty(&HighSide_PWM, 0);
            PWM_Set_Duty(&LowSide_PWM, 0);

            // LOW AND HIGH SIDE GND ON
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8);

            // DISABLE PWM
            PWM_Disable(&HighSide_PWM);
            PWM_Disable(&LowSide_PWM);

            low_side_pulse_count = 0;
            is_h_bridge_enable = false;
            H_Bridge_State = H_BRIDGE_WAIT;

            SchedulerTaskDisable(2);
        }
        break;

    default:
        break;
    }
}

void H_Bridge_5_Secconds_Task()
{
    switch (H_Bridge_State)
    {
        case H_BRIDGE_WAIT:
        {
            if(is_5s_set_up == false)
            {
                LL_TIM_EnableIT_UPDATE(TIM7);
                LL_TIM_EnableCounter(TIM7);
                is_5s_set_up = true;
            }
            else if(is_5s_finished == true)
            {
                LL_TIM_DisableIT_UPDATE(TIM7);
                LL_TIM_DisableCounter(TIM7);
                is_5s_set_up = false;
                is_5s_finished = false;
                H_Bridge_State = H_BRIDGE_POS_STATE;
            }
        }
        break;

    default:
        break;
    }
}

/* ::::H_Bridge High Side Interupt Handle:::: */
void H_Bridge_High_Side_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(HighSide_PWM.TIMx) == true)
    {
        high_side_pulse_count++;
        LL_TIM_ClearFlag_UPDATE(HighSide_PWM.TIMx);
    }
}

/* ::::H_Bridge Low Side Interupt Handle:::: */
void H_Bridge_Low_Side_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(LowSide_PWM.TIMx) == true)
    {
        low_side_pulse_count++;
        LL_TIM_ClearFlag_UPDATE(LowSide_PWM.TIMx);
    }
}

/* ::::H_Bridge 5 secconds Interupt Handle:::: */
void H_Bridge_5_Secconds_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(TIM7) == true)
    {
        is_5s_finished = true;
        LL_TIM_ClearFlag_UPDATE(TIM7);
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
