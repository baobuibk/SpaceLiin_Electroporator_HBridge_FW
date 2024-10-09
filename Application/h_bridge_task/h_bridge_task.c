/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdbool.h>

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_gpio.h"

#include "app.h"

#include "h_bridge_driver.h"

#include "scheduler.h"
#include "pwm.h"
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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static inline void V_Switch_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq);
static inline void V_Switch_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern PWM_TypeDef V_Switch_1_PWM;
extern PWM_TypeDef V_Switch_2_PWM;

H_Bridge_State_typedef H_Bridge_State = H_BRIDGE_STOP_STATE;

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

/* :::::::::: H Bridge Task ::::::::::::: */
void H_Bridge_Task(void*)
{
    switch (H_Bridge_State)
    {
    case H_BRIDGE_STOP_STATE:
        if(is_h_bridge_enable == false)
        {
            LL_GPIO_ResetOutputPin(V_SWITCH_HIN1_PORT, V_SWITCH_HIN1_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);

            LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);

            SchedulerTaskDisable(0);
        }
        else if(is_h_bridge_enable == true)
        {
            LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(V_SWITCH_LIN1_HANDLE);
            PWM_Set_Freq(&V_Switch_1_PWM, 1000 / (pulse_delay_ms / 2), 1);
            PWM_Set_Duty(&V_Switch_1_PWM, 0, 1);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);
            V_Switch_Set_Freq(&V_Switch_1_PWM, 10000);
            V_Switch_Set_Duty(&V_Switch_1_PWM, 50);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN1_HANDLE);
            LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN1_HANDLE);

            LL_TIM_EnableCounter(V_SWITCH_LIN1_HANDLE);

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
            LL_GPIO_ResetOutputPin(V_SWITCH_HIN1_PORT, V_SWITCH_HIN1_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(V_SWITCH_LIN2_HANDLE);
            PWM_Set_Freq(&V_Switch_2_PWM, 1000 / (pulse_delay_ms / 2), 1);
            PWM_Set_Duty(&V_Switch_2_PWM, 0, 1);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);
            V_Switch_Set_Freq(&V_Switch_2_PWM, 10000);
            V_Switch_Set_Duty(&V_Switch_2_PWM, 50);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN2_HANDLE);
            LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN2_HANDLE);

            LL_TIM_EnableCounter(V_SWITCH_LIN2_HANDLE);

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
            LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(V_SWITCH_LIN1_HANDLE);
            PWM_Set_Freq(&V_Switch_1_PWM, 1000 / (pulse_delay_ms / 2));
            PWM_Set_Duty(&V_Switch_1_PWM, 0);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);
            V_Switch_Set_Freq(&V_Switch_1_PWM, 10000);
            V_Switch_Set_Duty(&V_Switch_1_PWM, 50);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN1_HANDLE);
            LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN1_HANDLE);

            LL_TIM_EnableCounter(V_SWITCH_LIN1_HANDLE);

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
static inline void V_Switch_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq)
{
    uint16_t SD_ARR;
    SD_ARR = __LL_TIM_CALC_ARR(APB1_TIMER_CLK, LL_TIM_GetPrescaler(PWMx->TIMx), _Freq);
    LL_TIM_SetAutoReload(PWMx->TIMx, SD_ARR);
}

static inline void V_Switch_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty)
{
    // Limit the duty to 100
    if (_Duty > 100)
      return;

    // Set PWM DUTY for channel 1
    PWMx->Duty = (PWMx->Freq * (_Duty / 100.0));

    switch (PWMx->Channel)
    {
    case LL_TIM_CHANNEL_CH1:
        LL_TIM_OC_SetCompareCH1(PWMx->TIMx, _Duty);
        break;
    case LL_TIM_CHANNEL_CH2:
        LL_TIM_OC_SetCompareCH2(PWMx->TIMx, _Duty);
        break;
    case LL_TIM_CHANNEL_CH3:
        LL_TIM_OC_SetCompareCH3(PWMx->TIMx, _Duty);
        break;
    case LL_TIM_CHANNEL_CH4:
        LL_TIM_OC_SetCompareCH4(PWMx->TIMx, _Duty);
        break;

    default:
        break;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
