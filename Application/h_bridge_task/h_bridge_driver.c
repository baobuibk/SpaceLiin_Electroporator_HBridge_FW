/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "stm32f0xx_ll_rcc.h"

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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern PWM_TypeDef V_Switch_1_PWM;
extern PWM_TypeDef V_Switch_2_PWM;
PWM_TypeDef H_Bridge_1_PWM =
{
    .TIMx       =   H_BRIDGE_SD1_HANDLE,
    .Channel    =   H_BRIDGE_SD1_CHANNEL,
    .Prescaler  =   300,
    .Mode       =   LL_TIM_OCMODE_FORCED_ACTIVE,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   6, //50us
    .Freq       =   0,
};
PWM_TypeDef H_Bridge_2_PWM =
{
    .TIMx       =   H_BRIDGE_SD2_HANDLE,
    .Channel    =   H_BRIDGE_SD2_CHANNEL,
    .Prescaler  =   300,
    .Mode       =   LL_TIM_OCMODE_FORCED_ACTIVE,
    .Polarity   =   LL_TIM_OCPOLARITY_HIGH,
    .Duty       =   6, //50us
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
    case H_BRIDGE_MODE_HS_ON:
    case H_BRIDGE_MODE_LS_ON:
        LL_TIM_OC_SetMode(H_Bridge_x->PWM->TIMx, H_Bridge_x->PWM->Channel, LL_TIM_OCMODE_PWM2);
        PWM_Set_ARR(H_Bridge_x->PWM, 120, 0); //Period = 1ms
        PWM_Set_OC(H_Bridge_x->PWM, 6, 0); //Duty = 50us

        LL_TIM_GenerateEvent_UPDATE(H_Bridge_x->PWM->TIMx);

        break;
    case H_BRIDGE_MODE_FLOAT:
        LL_TIM_OC_SetMode(H_Bridge_x->PWM->TIMx, H_Bridge_x->PWM->Channel, LL_TIM_OCMODE_FORCED_INACTIVE);
        PWM_Set_ARR(H_Bridge_x->PWM, 0, 0); //Period = 1ms
        PWM_Set_OC(H_Bridge_x->PWM, 0, 0); //Duty = 50us

        LL_TIM_GenerateEvent_UPDATE(H_Bridge_x->PWM->TIMx);

        break;
    case H_BRIDGE_MODE_PULSE:
        PWM_Set_Freq(H_Bridge_x->PWM, (1000 / H_Bridge_x->delay_time_ms), 1);
        PWM_Set_OC(H_Bridge_x->PWM, 0, 1);
        LL_TIM_OC_SetMode(H_Bridge_x->PWM->TIMx, H_Bridge_x->PWM->Channel, LL_TIM_OCMODE_PWM2);
        PWM_Set_Freq(H_Bridge_x->PWM, (1000 / H_Bridge_x->on_time_ms), 0); //Period = 1ms
        PWM_Set_OC(H_Bridge_x->PWM, 6, 0); //Duty = 50us

        if (H_Bridge_x->delay_time_ms == 0)
        {
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_x->PWM->TIMx);
        }
        
        H_Bridge_x->Pin_State = 1;

        break;
    
    default:
        break;
    }

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
    H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);
    H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
}

/* ::::H_Bridge 1 Interupt Handle:::: */
void H_Bridge_1_Interupt_Handle()
{
    if(LL_TIM_IsActiveFlag_UPDATE(H_Bridge_1.PWM->TIMx) == true)
    {
        LL_TIM_ClearFlag_UPDATE(H_Bridge_1.PWM->TIMx);

        switch (H_Bridge_1.Mode)
        {
        case H_BRIDGE_MODE_HS_ON:
            LL_GPIO_SetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);

            LL_TIM_OC_SetMode(H_Bridge_1.PWM->TIMx, H_Bridge_1.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_1.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_1.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_LS_ON:
            LL_GPIO_ResetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);

            LL_TIM_OC_SetMode(H_Bridge_1.PWM->TIMx, H_Bridge_1.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_1.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_1.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_FLOAT:
            LL_TIM_DisableIT_UPDATE(H_Bridge_1.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_PULSE:
            H_Bridge_1.pulse_count++;

            if (H_Bridge_1.Pin_State == 1)
            {
                LL_GPIO_SetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);
                PWM_Set_Freq(H_Bridge_1.PWM, 1000 / H_Bridge_1.off_time_ms, 0);
                H_Bridge_1.Pin_State = 0;
            }
            else
            {
                LL_GPIO_ResetOutputPin(H_Bridge_1.Port, H_Bridge_1.Pin);
                PWM_Set_Freq(H_Bridge_1.PWM, 1000 / H_Bridge_1.on_time_ms, 0);
                H_Bridge_1.Pin_State = 1;
            }

            break;
        
        default:
            break;
        }
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
        case H_BRIDGE_MODE_HS_ON:
            LL_GPIO_SetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);

            LL_TIM_OC_SetMode(H_Bridge_2.PWM->TIMx, H_Bridge_2.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_2.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_2.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_LS_ON:
            LL_GPIO_ResetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);

            LL_TIM_OC_SetMode(H_Bridge_2.PWM->TIMx, H_Bridge_2.PWM->Channel, LL_TIM_OCMODE_FORCED_ACTIVE);
            LL_TIM_GenerateEvent_UPDATE(H_Bridge_2.PWM->TIMx);
            LL_TIM_ClearFlag_UPDATE(H_Bridge_2.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_FLOAT:
            LL_TIM_DisableIT_UPDATE(H_Bridge_2.PWM->TIMx);
            break;
        case H_BRIDGE_MODE_PULSE:
            H_Bridge_2.pulse_count++;

            if (H_Bridge_2.Pin_State == 1)
            {
                LL_GPIO_SetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);
                PWM_Set_Freq(H_Bridge_2.PWM, 1000 / H_Bridge_2.off_time_ms, 0);
                H_Bridge_2.Pin_State = 0;
            }
            else
            {
                LL_GPIO_ResetOutputPin(H_Bridge_2.Port, H_Bridge_2.Pin);
                PWM_Set_Freq(H_Bridge_2.PWM, 1000 / H_Bridge_2.on_time_ms, 0);
                H_Bridge_2.Pin_State = 1;
            }

            break;
        
        default:
            break;
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
