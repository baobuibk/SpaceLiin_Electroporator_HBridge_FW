/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "app.h"

#include "h_bridge_driver.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
__STATIC_INLINE void HB_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty, bool apply_now);
__STATIC_INLINE void HB_Set_OC(PWM_TypeDef *PWMx, uint32_t _OC, bool apply_now);
__STATIC_INLINE void HB_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq, bool apply_now);
__STATIC_INLINE void HB_Set_ARR(PWM_TypeDef *PWMx, uint32_t _ARR, bool apply_now);

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
PWM_TypeDef H_Bridge_1_PWM =
{
    .TIMx       =   H_BRIDGE_SD1_HANDLE,
    .Channel    =   H_BRIDGE_SD1_CHANNEL,
    .Prescaler  =   600,
    .Mode       =   LL_TIM_OCMODE_FORCED_ACTIVE,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   6, //100us
    .Freq       =   0,
};
PWM_TypeDef H_Bridge_2_PWM =
{
    .TIMx       =   H_BRIDGE_SD2_HANDLE,
    .Channel    =   H_BRIDGE_SD2_CHANNEL,
    .Prescaler  =   600,
    .Mode       =   LL_TIM_OCMODE_FORCED_ACTIVE,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   6, //100us
    .Freq       =   0,
};

H_Bridge_typdef H_Bridge_1 =
{
    .Port               = H_BRIDGE_HIN1_PORT,
    .Pin                = H_BRIDGE_HIN1_PIN,
    .Pin_State          = 0,
    .PWM                = &H_Bridge_1_PWM,
    .Mode               = H_BRIDGE_MODE_LS_ON,
    .delay_time_ms      = 0,
    .on_time_ms         = 0,
    .off_time_ms        = 0,
    .set_pulse_count    = 0,
    .pulse_count        = 0,
    .is_setted          = false,
};

H_Bridge_typdef H_Bridge_2 =
{
    .Port               = H_BRIDGE_HIN2_PORT,
    .Pin                = H_BRIDGE_HIN2_PIN,
    .Pin_State          = 0,
    .PWM                = &H_Bridge_2_PWM,
    .Mode               = H_BRIDGE_MODE_LS_ON,
    .delay_time_ms      = 0,
    .on_time_ms         = 0,
    .off_time_ms        = 0,
    .set_pulse_count    = 0,
    .pulse_count        = 0,
    .is_setted          = false,
};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: H Bridge Driver Init :::::::: */
void H_Bridge_Driver_Init(void)
{   // H bridge 1 init
    PWM_Init(H_Bridge_1.PWM);
    PWM_Enable(H_Bridge_1.PWM);
    LL_GPIO_ResetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);
    LL_TIM_DisableIT_UPDATE(H_Bridge_1.PWM->TIMx);

    // H bridge 2 init
    PWM_Init(H_Bridge_2.PWM);
    PWM_Enable(H_Bridge_2.PWM);
    LL_GPIO_ResetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);
    LL_TIM_DisableIT_UPDATE(H_Bridge_2.PWM->TIMx);
}

void H_Bridge_Set_Mode(H_Bridge_typdef* H_Bridge_x, H_Bridge_mode SetMode)
{
    LL_TIM_DisableIT_UPDATE(H_Bridge_x->PWM->TIMx);
    LL_TIM_DisableCounter(H_Bridge_x->PWM->TIMx);
    LL_TIM_ClearFlag_UPDATE(H_Bridge_x->PWM->TIMx);
    H_Bridge_x->Mode = SetMode;

    switch (SetMode)
    {
    case H_BRIDGE_MODE_PULSE:
        HB_Set_Freq(H_Bridge_x->PWM, (1000 / H_Bridge_x->delay_time_ms), 1);
        HB_Set_OC(H_Bridge_x->PWM, 0, 1);
        LL_TIM_OC_SetMode(H_Bridge_x->PWM->TIMx, H_Bridge_x->PWM->Channel, LL_TIM_OCMODE_PWM2);
        HB_Set_Freq(H_Bridge_x->PWM, (1000 / H_Bridge_x->on_time_ms), 0); //Period = 1ms
        HB_Set_OC(H_Bridge_x->PWM, 6, 0); //Duty = 50us
        
        H_Bridge_x->Pin_State = 1;

        break;
    case H_BRIDGE_MODE_HS_ON:
    case H_BRIDGE_MODE_LS_ON:
        LL_TIM_OC_SetMode(H_Bridge_x->PWM->TIMx, H_Bridge_x->PWM->Channel, LL_TIM_OCMODE_PWM2);
        HB_Set_ARR(H_Bridge_x->PWM, 120, 1); //Period = 1ms
        HB_Set_OC(H_Bridge_x->PWM, 6, 1); //Duty = 50us
        H_Bridge_x->pulse_count = 0;
        H_Bridge_x->delay_time_ms = 0;

        break;
    case H_BRIDGE_MODE_FLOAT:
        LL_TIM_OC_SetMode(H_Bridge_x->PWM->TIMx, H_Bridge_x->PWM->Channel, LL_TIM_OCMODE_FORCED_INACTIVE);
        HB_Set_ARR(H_Bridge_x->PWM, 0, 1); //Period = 1ms
        HB_Set_OC(H_Bridge_x->PWM, 0, 1); //Duty = 50us
        H_Bridge_x->pulse_count = 0;
        H_Bridge_x->delay_time_ms = 0;

        break;
    
    default:
        break;
    }

    //if (H_Bridge_x->delay_time_ms == 0)
    //{
        //LL_TIM_GenerateEvent_UPDATE(H_Bridge_x->PWM->TIMx);
    //}

    LL_TIM_ClearFlag_UPDATE(H_Bridge_x->PWM->TIMx);
    LL_TIM_EnableIT_UPDATE(H_Bridge_x->PWM->TIMx);
    LL_TIM_EnableCounter(H_Bridge_x->PWM->TIMx);
}

void H_Bridge_Set_Pulse_Timing(H_Bridge_typdef* H_Bridge_x, uint16_t Set_delay_time_ms, uint16_t Set_on_time_ms, uint16_t Set_off_time_ms, uint16_t Set_pulse_count)
{
    LL_TIM_DisableIT_UPDATE(H_Bridge_x->PWM->TIMx);
    LL_TIM_DisableCounter(H_Bridge_x->PWM->TIMx);
    LL_TIM_ClearFlag_UPDATE(H_Bridge_x->PWM->TIMx);

    H_Bridge_x->delay_time_ms   = Set_delay_time_ms;

    H_Bridge_x->on_time_ms      = Set_on_time_ms;
    H_Bridge_x->off_time_ms     = Set_off_time_ms;

    H_Bridge_x->set_pulse_count = Set_pulse_count;
    H_Bridge_x->pulse_count     = 0;

    LL_TIM_EnableIT_UPDATE(H_Bridge_x->PWM->TIMx);
    LL_TIM_EnableCounter(H_Bridge_x->PWM->TIMx);
}

void H_Bridge_Kill(void)
{
    H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_FLOAT);
    H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_FLOAT);
}

/* ::::H_Bridge 1 Interupt Handle:::: */
void H_Bridge_1_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(H_Bridge_1.PWM->TIMx) == true)
    {
        LL_TIM_ClearFlag_UPDATE(H_Bridge_1.PWM->TIMx);

        switch (H_Bridge_1.Mode)
        {
        case H_BRIDGE_MODE_PULSE:
            H_Bridge_1.pulse_count++;

            if (H_Bridge_1.Pin_State == 1)
            {
                LL_GPIO_SetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);
                HB_Set_Freq(H_Bridge_1.PWM, 1000 / H_Bridge_1.off_time_ms, 0);
                H_Bridge_1.Pin_State = 0;
            }
            else
            {
                LL_GPIO_ResetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);
                HB_Set_Freq(H_Bridge_1.PWM, 1000 / H_Bridge_1.on_time_ms, 0);
                H_Bridge_1.Pin_State = 1;
            }

            break;
        case H_BRIDGE_MODE_HS_ON:
            LL_GPIO_SetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);

            LL_TIM_OC_SetMode(H_Bridge_1.PWM->TIMx, H_Bridge_1.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_1.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_1.PWM->TIMx);
            LL_TIM_DisableIT_UPDATE(H_Bridge_1.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_LS_ON:
            LL_GPIO_ResetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);

            LL_TIM_OC_SetMode(H_Bridge_1.PWM->TIMx, H_Bridge_1.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_1.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_1.PWM->TIMx);
            LL_TIM_DisableIT_UPDATE(H_Bridge_1.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_FLOAT:
            LL_TIM_DisableIT_UPDATE(H_Bridge_1.PWM->TIMx);
            break;
        
        default:
            break;
        }

        H_Bridge_1.is_setted = true;
    }
}

/* ::::H_Bridge 2 Interupt Handle:::: */
void H_Bridge_2_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(H_Bridge_2.PWM->TIMx) == true)
    {
        LL_TIM_ClearFlag_UPDATE(H_Bridge_2.PWM->TIMx);

        switch (H_Bridge_2.Mode)
        {
        case H_BRIDGE_MODE_PULSE:
            H_Bridge_2.pulse_count++;

            if (H_Bridge_2.Pin_State == 1)
            {
                LL_GPIO_SetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);
                HB_Set_Freq(H_Bridge_2.PWM, 1000 / H_Bridge_2.off_time_ms, 0);
                H_Bridge_2.Pin_State = 0;
            }
            else
            {
                LL_GPIO_ResetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);
                HB_Set_Freq(H_Bridge_2.PWM, 1000 / H_Bridge_2.on_time_ms, 0);
                H_Bridge_2.Pin_State = 1;
            }

            break;
        case H_BRIDGE_MODE_HS_ON:
            LL_GPIO_SetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);

            LL_TIM_OC_SetMode(H_Bridge_2.PWM->TIMx, H_Bridge_2.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_2.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_2.PWM->TIMx);
            LL_TIM_DisableIT_UPDATE(H_Bridge_2.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_LS_ON:
            LL_GPIO_ResetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);

            LL_TIM_OC_SetMode(H_Bridge_2.PWM->TIMx, H_Bridge_2.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_2.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_2.PWM->TIMx);
            LL_TIM_DisableIT_UPDATE(H_Bridge_2.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_FLOAT:
            LL_TIM_DisableIT_UPDATE(H_Bridge_2.PWM->TIMx);
            break;
        
        default:
            break;
        }

        H_Bridge_2.is_setted = true;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
__STATIC_INLINE void HB_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty, bool apply_now)
{
    LL_TIM_DisableUpdateEvent(PWMx->TIMx);

    // Limit the duty to 100
    if (_Duty > 100)
      return;

    // Set PWM DUTY for channel 1
    PWMx->Duty = (PWMx->Freq * (_Duty / 100.0));
    switch (PWMx->Channel)
    {
    case LL_TIM_CHANNEL_CH1:
        LL_TIM_OC_SetCompareCH1(PWMx->TIMx, PWMx->Duty);
        break;
    case LL_TIM_CHANNEL_CH2:
        LL_TIM_OC_SetCompareCH2(PWMx->TIMx, PWMx->Duty);
        break;
    case LL_TIM_CHANNEL_CH3:
        LL_TIM_OC_SetCompareCH3(PWMx->TIMx, PWMx->Duty);
        break;
    case LL_TIM_CHANNEL_CH4:
        LL_TIM_OC_SetCompareCH4(PWMx->TIMx, PWMx->Duty);
        break;

    default:
        break;
    }

    LL_TIM_EnableUpdateEvent(PWMx->TIMx);

    if(apply_now == 1)
    {
        if (LL_TIM_IsEnabledIT_UPDATE(PWMx->TIMx))
        {
            LL_TIM_DisableIT_UPDATE(PWMx->TIMx);
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
            LL_TIM_EnableIT_UPDATE(PWMx->TIMx);
        }
        else
        {
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
        }
    }
}

__STATIC_INLINE void HB_Set_OC(PWM_TypeDef *PWMx, uint32_t _OC, bool apply_now)
{
    LL_TIM_DisableUpdateEvent(PWMx->TIMx);

    // Set PWM DUTY for channel 1
    PWMx->Duty = _OC;
    switch (PWMx->Channel)
    {
    case LL_TIM_CHANNEL_CH1:
        LL_TIM_OC_SetCompareCH1(PWMx->TIMx, PWMx->Duty);
        break;
    case LL_TIM_CHANNEL_CH2:
        LL_TIM_OC_SetCompareCH2(PWMx->TIMx, PWMx->Duty);
        break;
    case LL_TIM_CHANNEL_CH3:
        LL_TIM_OC_SetCompareCH3(PWMx->TIMx, PWMx->Duty);
        break;
    case LL_TIM_CHANNEL_CH4:
        LL_TIM_OC_SetCompareCH4(PWMx->TIMx, PWMx->Duty);
        break;

    default:
        break;
    }

    LL_TIM_EnableUpdateEvent(PWMx->TIMx);

    if(apply_now == 1)
    {
        if (LL_TIM_IsEnabledIT_UPDATE(PWMx->TIMx))
        {
            LL_TIM_DisableIT_UPDATE(PWMx->TIMx);
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
            LL_TIM_EnableIT_UPDATE(PWMx->TIMx);
        }
        else
        {
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
        }
    }
}

__STATIC_INLINE void HB_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq, bool apply_now)
{
    LL_TIM_DisableUpdateEvent(PWMx->TIMx);

    // Set PWM FREQ
    PWMx->Freq = __LL_TIM_CALC_ARR(APB1_TIMER_CLK, LL_TIM_GetPrescaler(PWMx->TIMx), _Freq);
    LL_TIM_SetAutoReload(PWMx->TIMx, PWMx->Freq);

    LL_TIM_EnableUpdateEvent(PWMx->TIMx);

    if(apply_now == 1)
    {
        if (LL_TIM_IsEnabledIT_UPDATE(PWMx->TIMx))
        {
            LL_TIM_DisableIT_UPDATE(PWMx->TIMx);
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
            LL_TIM_EnableIT_UPDATE(PWMx->TIMx);
        }
        else
        {
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
        }
    }
}

__STATIC_INLINE void HB_Set_ARR(PWM_TypeDef *PWMx, uint32_t _ARR, bool apply_now)
{
    LL_TIM_DisableUpdateEvent(PWMx->TIMx);

    // Set PWM FREQ
    PWMx->Freq = _ARR;
    LL_TIM_SetAutoReload(PWMx->TIMx, PWMx->Freq);

    LL_TIM_EnableUpdateEvent(PWMx->TIMx);

    if(apply_now == 1)
    {
        if (LL_TIM_IsEnabledIT_UPDATE(PWMx->TIMx))
        {
            LL_TIM_DisableIT_UPDATE(PWMx->TIMx);
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
            LL_TIM_EnableIT_UPDATE(PWMx->TIMx);
        }
        else
        {
            LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
