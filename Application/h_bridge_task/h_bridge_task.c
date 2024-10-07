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
typedef enum
{
    H_BRIDGE_STOP_STATE,
    H_BRIDGE_HV_1_STATE,
    H_BRDIGE_HV_2_STATE,
    H_BRIDGE_LV_1_STATE,
    H_BRIDGE_LV_2_STATE,
} H_Bridge_State_typedef;

typedef enum
{
    PWM_LOGIC_HIGH_STATE,
    PWM_LOGIC_LOW_STATE,
} PWM_state_typdef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static PWM_state_typdef     PWM_State = PWM_LOGIC_HIGH_STATE;
static bool                 is_h_bridge_off      = false;

static volatile uint16_t    pulse_set_count     = 0;
static volatile uint16_t    pulse_count         = 0;
static volatile uint8_t     pulse_on_time_ms    = 0;
static volatile uint8_t     pulse_off_time_ms   = 0;

static volatile bool        is_5s_set_up            = false;
static volatile bool        is_5s_finished          = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static inline void SD_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq);
static inline void SD_Set_OC(PWM_TypeDef *PWMx, uint32_t OC);

static inline void V_Switch_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq);
static inline void V_Switch_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern PWM_TypeDef V_Switch_1_PWM;
extern PWM_TypeDef V_Switch_2_PWM;
PWM_TypeDef H_Bridge_1_PWM =
{
    .TIMx       =   H_BRIDGE_SD1_HANDLE,
    .Channel    =   H_BRIDGE_SD1_CHANNEL,
    .Prescaler  =   300,
    .Mode       =   LL_TIM_OCMODE_PWM2,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   6, //50us
    .Freq       =   0,
};
PWM_TypeDef H_Bridge_2_PWM =
{
    .TIMx       =   H_BRIDGE_SD2_HANDLE,
    .Channel    =   H_BRIDGE_SD2_CHANNEL,
    .Prescaler  =   300,
    .Mode       =   LL_TIM_OCMODE_PWM2,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   6, //50us
    .Freq       =   0,
};

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
{     // H bridge 1 init
    PWM_Init(&H_Bridge_1_PWM);
    //PWM_Disable(&H_Bridge_1_PWM);
    LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
    PWM_Enable(&H_Bridge_1_PWM);
    LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
    LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);

    // H bridge lowside
    PWM_Init(&H_Bridge_2_PWM);
    LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
    PWM_Enable(&H_Bridge_2_PWM);
    LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
    LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);
}

/* :::::::::: H Bridge Task ::::::::::::: */
void H_Bridge_Task(void*)
{
    switch (H_Bridge_State)
    {
    case H_BRIDGE_STOP_STATE:
        if(is_h_bridge_off == false)
        {
            LL_GPIO_ResetOutputPin(V_SWITCH_HIN1_PORT, V_SWITCH_HIN1_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);

            LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);
            LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

            // DISABLE PWM
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            is_h_bridge_off = true;
        }
        else if(is_h_bridge_enable == true)
        {
            pulse_set_count     = hv_pulse_count;
            pulse_count         = 0;
            pulse_on_time_ms    = hv_on_time_ms;
            pulse_off_time_ms   = hv_off_time_ms;
            H_Bridge_State      = H_BRIDGE_HV_1_STATE;

            LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(V_SWITCH_LIN1_HANDLE);
            PWM_Set_Freq(&V_Switch_1_PWM, 1000 / (pulse_delay_ms / 2));
            PWM_Set_Duty(&V_Switch_1_PWM, 0);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);
            V_Switch_Set_Freq(&V_Switch_1_PWM, 10000);
            V_Switch_Set_Duty(&V_Switch_1_PWM, 50);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(H_BRIDGE_SD1_HANDLE);
            PWM_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_delay_ms);
            PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_PWM2);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_on_time_ms);
            SD_Set_OC(&H_Bridge_1_PWM, 6);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN1_HANDLE);
            LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN1_HANDLE);
            LL_TIM_ClearFlag_UPDATE(H_BRIDGE_SD1_HANDLE);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD1_HANDLE);

            LL_TIM_EnableCounter(V_SWITCH_LIN1_HANDLE);
            LL_TIM_EnableCounter(H_BRIDGE_SD1_HANDLE);
        }
        break;
    case H_BRIDGE_HV_1_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State  = H_BRIDGE_STOP_STATE;
            is_h_bridge_off = false;
        }
        else if(pulse_count >= (pulse_set_count * 2))
        {
            pulse_count     = 0;
            PWM_State       = PWM_LOGIC_HIGH_STATE;
            H_Bridge_State  = H_BRDIGE_HV_2_STATE;

            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(H_BRIDGE_SD2_HANDLE);

            // CHANGE THE ARR AND SET THE PWM MODE
            // WHEN CHANGE THE ARR, IT ALSO FIRE THE UG BIT
            // WHICH RESET THE CNT AND PRESCALE CNT
            PWM_Set_Freq(&H_Bridge_2_PWM, 20000);
            PWM_Set_Duty(&H_Bridge_2_PWM, 0);
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_PWM2);
            SD_Set_Freq(&H_Bridge_2_PWM, 1000 / pulse_on_time_ms);
            SD_Set_OC(&H_Bridge_2_PWM, 6);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(H_BRIDGE_SD2_HANDLE);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD2_HANDLE);
            LL_TIM_EnableCounter(H_BRIDGE_SD2_HANDLE);
        }
        break;
    case H_BRDIGE_HV_2_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
            is_h_bridge_off = false;
        }
        else if(pulse_count >= (pulse_set_count * 2))
        {
            pulse_set_count     = lv_pulse_count;
            pulse_count         = 0;
            pulse_on_time_ms    = lv_on_time_ms;
            pulse_off_time_ms   = lv_off_time_ms;
            PWM_State           = PWM_LOGIC_HIGH_STATE;
            H_Bridge_State      = H_BRIDGE_LV_1_STATE;

            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(H_BRIDGE_SD1_HANDLE);

            LL_GPIO_ResetOutputPin(V_SWITCH_HIN1_PORT, V_SWITCH_HIN1_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(V_SWITCH_LIN2_HANDLE);
            PWM_Set_Freq(&V_Switch_2_PWM, 1000 / (pulse_delay_ms / 2));
            PWM_Set_Duty(&V_Switch_2_PWM, 0);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);
            V_Switch_Set_Freq(&V_Switch_2_PWM, 10000);
            V_Switch_Set_Duty(&V_Switch_2_PWM, 50);

            // CHANGE THE ARR AND SET THE PWM MODE
            // WHEN CHANGE THE ARR, IT ALSO FIRE THE UG BIT
            // WHICH RESET THE CNT AND PRESCALE CNT
            PWM_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_delay_ms);
            PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_PWM2);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_on_time_ms);
            SD_Set_OC(&H_Bridge_1_PWM, 6);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN2_HANDLE);
            LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN2_HANDLE);
            LL_TIM_ClearFlag_UPDATE(H_BRIDGE_SD1_HANDLE);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD1_HANDLE);

            LL_TIM_EnableCounter(V_SWITCH_LIN2_HANDLE);
            LL_TIM_EnableCounter(H_BRIDGE_SD1_HANDLE);
        }
        break;
    case H_BRIDGE_LV_1_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
            is_h_bridge_off = false;
        }
        else if(pulse_count >= (pulse_set_count * 2))
        {
            pulse_count     = 0;
            PWM_State       = PWM_LOGIC_HIGH_STATE;
            H_Bridge_State  = H_BRIDGE_LV_2_STATE;

            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_1_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(H_BRIDGE_SD2_HANDLE);

            // CHANGE THE ARR AND SET THE PWM MODE
            // WHEN CHANGE THE ARR, IT ALSO FIRE THE UG BIT
            // WHICH RESET THE CNT AND PRESCALE CNT
            PWM_Set_Freq(&H_Bridge_2_PWM, 20000);
            PWM_Set_Duty(&H_Bridge_2_PWM, 0);
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_PWM2);
            SD_Set_Freq(&H_Bridge_2_PWM, 1000 / pulse_on_time_ms);
            SD_Set_OC(&H_Bridge_2_PWM, 6);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(H_BRIDGE_SD2_HANDLE);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD2_HANDLE);
            LL_TIM_EnableCounter(H_BRIDGE_SD2_HANDLE);
        }
        break;
    case H_BRIDGE_LV_2_STATE:
        if(is_h_bridge_enable == false)
        {
            H_Bridge_State = H_BRIDGE_STOP_STATE;
            is_h_bridge_off = false;
        }
        else if(pulse_count >= (pulse_set_count * 2))
        {
            /*
            pulse_set_count     = hv_pulse_count;
            pulse_count         = 0;
            pulse_on_time_ms    = hv_on_time_ms;
            pulse_off_time_ms   = hv_off_time_ms;
            PWM_State           = PWM_LOGIC_HIGH_STATE;
            H_Bridge_State      = H_BRIDGE_HV_1_STATE;

            // Disable Update Interupt
            LL_TIM_DisableIT_UPDATE(H_Bridge_2_PWM.TIMx);

            // TURN OFF LOW AND HIGH FIRST
            LL_TIM_OC_SetMode(H_BRIDGE_SD2_HANDLE, H_BRIDGE_SD2_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(H_BRIDGE_SD1_HANDLE);

            LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
            LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

            // STOP THE CNT AND RESET IT TO 0.
            LL_TIM_DisableCounter(V_SWITCH_LIN1_HANDLE);
            PWM_Set_Freq(&V_Switch_1_PWM, 1000 / (pulse_delay_ms / 2));
            PWM_Set_Duty(&V_Switch_1_PWM, 0);
            LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);
            V_Switch_Set_Freq(&V_Switch_1_PWM, 10000);
            V_Switch_Set_Duty(&V_Switch_1_PWM, 50);

            // CHANGE THE ARR AND SET THE PWM MODE
            // WHEN CHANGE THE ARR, IT ALSO FIRE THE UG BIT
            // WHICH RESET THE CNT AND PRESCALE CNT
            PWM_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_delay_ms);
            PWM_Set_Duty(&H_Bridge_1_PWM, 0);
            LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_PWM2);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_on_time_ms);
            SD_Set_OC(&H_Bridge_1_PWM, 163);

            //ENABLE IT UPDATE, ENABLE CNT AND GENERATE EVENT
            LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN1_HANDLE);
            LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN1_HANDLE);
            LL_TIM_ClearFlag_UPDATE(H_BRIDGE_SD1_HANDLE);
            LL_TIM_EnableIT_UPDATE(H_BRIDGE_SD1_HANDLE);

            LL_TIM_EnableCounter(V_SWITCH_LIN1_HANDLE);
            LL_TIM_EnableCounter(H_BRIDGE_SD1_HANDLE);
            */
        
            
            is_h_bridge_enable = false;
            H_Bridge_State = H_BRIDGE_STOP_STATE;
            is_h_bridge_off = false;
            
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
        pulse_count++;
        LL_TIM_ClearFlag_UPDATE(H_Bridge_1_PWM.TIMx);
        
        switch (PWM_State)
        {
        case PWM_LOGIC_HIGH_STATE:
            LL_GPIO_SetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_off_time_ms);
            PWM_State = PWM_LOGIC_LOW_STATE;
            break;
        
        case PWM_LOGIC_LOW_STATE:
            LL_GPIO_ResetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
            SD_Set_Freq(&H_Bridge_1_PWM, 1000 / pulse_on_time_ms);
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
    if(LL_TIM_IsActiveFlag_UPDATE(H_Bridge_2_PWM.TIMx) == true)
    {
        pulse_count++;
        LL_TIM_ClearFlag_UPDATE(H_Bridge_2_PWM.TIMx);
        
        switch (PWM_State)
        {
        case PWM_LOGIC_HIGH_STATE:
            LL_GPIO_SetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
            SD_Set_Freq(&H_Bridge_2_PWM, 1000 / pulse_off_time_ms);
            PWM_State = PWM_LOGIC_LOW_STATE;
            break;
        
        case PWM_LOGIC_LOW_STATE:
            LL_GPIO_ResetOutputPin(H_BRIDGE_HIN2_PORT, H_BRIDGE_HIN2_PIN);
            SD_Set_Freq(&H_Bridge_2_PWM, 1000 / pulse_on_time_ms);
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

static inline void SD_Set_OC(PWM_TypeDef *PWMx, uint32_t OC)
{
    switch (PWMx->Channel)
    {
    case LL_TIM_CHANNEL_CH1:
        LL_TIM_OC_SetCompareCH1(PWMx->TIMx, OC);
        break;
    case LL_TIM_CHANNEL_CH2:
        LL_TIM_OC_SetCompareCH2(PWMx->TIMx, OC);
        break;
    case LL_TIM_CHANNEL_CH3:
        LL_TIM_OC_SetCompareCH3(PWMx->TIMx, OC);
        break;
    case LL_TIM_CHANNEL_CH4:
        LL_TIM_OC_SetCompareCH4(PWMx->TIMx, OC);
        break;

    default:
        break;
    }
}

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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
