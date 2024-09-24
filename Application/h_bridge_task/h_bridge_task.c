/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdbool.h>

#include "stm32f0xx_ll_rcc.h"
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
static PWM_TypeDef V_Switch_1_PWM =
{
    .TIMx       =   V_SWITCH_LIN1_HANDLE,
    .Channel    =   V_SWITCH_LIN1_CHANNEL,
    .Prescaler  =   0,
    .Mode       =   LL_TIM_OCMODE_PWM1,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   0,
    .Freq       =   0,
};
static PWM_TypeDef V_Switch_2_PWM =
{
    .TIMx       =   V_SWITCH_LIN2_HANDLE,
    .Channel    =   V_SWITCH_LIN2_CHANNEL,
    .Prescaler  =   0,
    .Mode       =   LL_TIM_OCMODE_PWM1,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   0,
    .Freq       =   0,
};

typedef enum
{
    PWM_LOGIC_HIGH_STATE,
    PWM_LOGIC_LOW_STATE,
} PWM_state_typdef;

static PWM_state_typdef     PWM_State = PWM_LOGIC_HIGH_STATE;
static bool                 is_h_bridge_set_up      = false;

static volatile uint16_t    high_side_pulse_count   = 0;
static volatile uint16_t    low_side_pulse_count    = 0;

static volatile bool        is_5s_set_up            = false;
static volatile bool        is_5s_finished          = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static inline void SD_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
PWM_TypeDef H_Bridge_1_PWM =
{
    .TIMx       =   H_BRIDGE_SD1_HANDLE,
    .Channel    =   H_BRIDGE_SD1_CHANNEL,
    .Prescaler  =   10,
    .Mode       =   LL_TIM_OCMODE_PWM2,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   1636, //500us
    .Freq       =   0,
};
PWM_TypeDef H_Bridge_2_PWM =
{
    .TIMx       =   H_BRIDGE_SD2_HANDLE,
    .Channel    =   H_BRIDGE_SD2_CHANNEL,
    .Prescaler  =   10,
    .Mode       =   LL_TIM_OCMODE_PWM2,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   1636, //500us
    .Freq       =   0,
};

H_Bridge_State_typedef H_Bridge_State = H_BRIDGE_STOP_STATE;

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
    // H bridge 1 init
    PWM_Init(&H_Bridge_1_PWM);
    PWM_Disable(&H_Bridge_1_PWM);
    LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
    PWM_Enable(&H_Bridge_1_PWM);
    LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);

    // H bridge lowside
    PWM_Init(&H_Bridge_2_PWM);
    LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
    PWM_Enable(&H_Bridge_2_PWM);
    LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
    LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

    LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
    LL_GPIO_SetOutputPin(V_SWITCH_HIN1_PORT, V_SWITCH_HIN1_PIN);
    PWM_Init(&V_Switch_1_PWM);
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
        LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);
        LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

        // TURN OFF LOW AND HIGH FIRST
        //PWM_Set_Duty(&H_Bridge_1_PWM, 0);
        //PWM_Set_Duty(&H_Bridge_2_PWM, 0);

        // LOW AND HIGH SIDE GND ON
        //LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
        //LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);

        // DISABLE PWM
        LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
        LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
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
            //PWM_Set_Freq(&H_Bridge_1_PWM, 1000 / hs_on_time_ms);
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_PWM2);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD1_HANDLE);
            LL_TIM_DisableUpdateEvent(H_Bridge_1_PWM.TIMx);
            LL_TIM_EnableUpdateEvent(H_Bridge_1_PWM.TIMx);
            LL_TIM_GenerateEvent_UPDATE(H_BRIDGE_SD1_HANDLE);
            is_h_bridge_set_up = true;
        }
        else if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        else if(high_side_pulse_count == (high_side_set_pulse_count * 2))
        {
            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            high_side_pulse_count = 0;
            is_h_bridge_set_up = false;
            PWM_State = PWM_LOGIC_HIGH_STATE;
            H_Bridge_State = H_BRDIGE_2_STATE;
        }
        break;
    case H_BRDIGE_2_STATE:
        if (is_h_bridge_set_up == false)
        {
            PWM_Set_Freq(&H_Bridge_2_PWM, 1000 / ls_on_time_ms);
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_PWM2);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD2_HANDLE);
            LL_TIM_DisableUpdateEvent(H_Bridge_1_PWM.TIMx);
            LL_TIM_EnableUpdateEvent(H_Bridge_1_PWM.TIMx);
            LL_TIM_GenerateEvent_UPDATE(H_BRIDGE_SD2_HANDLE);
            is_h_bridge_set_up = true;
        }
        else if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
        }
        else if(low_side_pulse_count == (low_side_set_pulse_count * 2))
        {
            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            low_side_pulse_count = 0;
            is_h_bridge_set_up = false;
            PWM_State = PWM_LOGIC_HIGH_STATE;
            H_Bridge_State = H_BRIDGE_1_STATE;
        }
        break;

    default:
        break;
    }
}

/* ::::H_Bridge 1 Interupt Handle:::: */
void H_Bridge_1_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(H_Bridge_1_PWM.TIMx) == true)
    {
        high_side_pulse_count++;
        LL_TIM_ClearFlag_UPDATE(H_Bridge_1_PWM.TIMx);
        
        switch (PWM_State)
        {
        case PWM_LOGIC_HIGH_STATE:
            LL_GPIO_SetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / hs_off_time_ms);
            PWM_State = PWM_LOGIC_LOW_STATE;
            break;
        
        case PWM_LOGIC_LOW_STATE:
            LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / hs_on_time_ms);
            PWM_State = PWM_LOGIC_HIGH_STATE;
            break;

        default:
            break;
        }
    }
}

/* ::::H_Bridge 2 Interupt Handle:::: */
void H_Bridge_2_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_CC2(H_Bridge_2_PWM.TIMx) == true)
    {
        low_side_pulse_count++;
        LL_TIM_ClearFlag_UPDATE(H_Bridge_2_PWM.TIMx);
        
        switch (PWM_State)
        {
        case PWM_LOGIC_HIGH_STATE:
            LL_GPIO_SetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
            SD_Set_Freq(&H_Bridge_2_PWM, 1000 / ls_off_time_ms);
            PWM_State = PWM_LOGIC_LOW_STATE;
            break;
        
        case PWM_LOGIC_LOW_STATE:
            LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
            SD_Set_Freq(&H_Bridge_2_PWM, 1000 / ls_on_time_ms);
            PWM_State = PWM_LOGIC_HIGH_STATE;
            break;

        default:
            break;
        }
    }
}

static inline void SD_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq)
{
    uint16_t SD_ARR;
    SD_ARR = __LL_TIM_CALC_ARR(APB1_TIMER_CLK, LL_TIM_GetPrescaler(PWMx->TIMx), _Freq);
    LL_TIM_SetAutoReload(PWMx->TIMx, SD_ARR);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
