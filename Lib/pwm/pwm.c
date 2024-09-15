#include "pwm.h"

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_tim.h"

#define APB1_TIMER_CLK \
__LL_RCC_CALC_PCLK1_FREQ(__LL_RCC_CALC_HCLK_FREQ(SystemCoreClock, LL_RCC_GetAHBPrescaler()), LL_RCC_GetAPB1Prescaler())
/**
  * @brief  Define the behavior of the PWM.
  * @note   The Pin is set to high so that the FET is turned off.
  * @param  PWMx    PWM instance
  * @param  TIMx    TIM instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_TIM_CHANNEL_CH1
  *         @arg @ref LL_TIM_CHANNEL_CH2
  *         @arg @ref LL_TIM_CHANNEL_CH3
  *         @arg @ref LL_TIM_CHANNEL_CH4
  * @param  Mode This parameter can be one of the following values:
  *         @arg @ref LL_TIM_OCMODE_FROZEN
  *         @arg @ref LL_TIM_OCMODE_ACTIVE
  *         @arg @ref LL_TIM_OCMODE_INACTIVE
  *         @arg @ref LL_TIM_OCMODE_TOGGLE
  *         @arg @ref LL_TIM_OCMODE_FORCED_INACTIVE
  *         @arg @ref LL_TIM_OCMODE_FORCED_ACTIVE
  *         @arg @ref LL_TIM_OCMODE_PWM1
  *         @arg @ref LL_TIM_OCMODE_PWM2
  * @param  Polarity This parameter can be one of the following values:
  *         @arg @ref LL_TIM_OCPOLARITY_HIGH
  *         @arg @ref LL_TIM_OCPOLARITY_LOW
  * @retval None
  */
void PWM_Init(PWM_TypeDef *PWMx, TIM_TypeDef *TIMx, uint32_t Channel, uint32_t Mode, uint32_t Polarity)
{
    // Set PWM MODE
    PWMx->TIMx    = TIMx;
    PWMx->Channel = Channel;
    PWMx->Mode    = Mode;
    LL_TIM_OC_SetMode(PWMx->TIMx, PWMx->Channel, PWMx->Mode);

    PWMx->Polarity = Polarity;
    LL_TIM_OC_SetPolarity(PWMx->TIMx, PWMx->Channel, PWMx->Polarity);

    // Set PWM FREQ
    PWMx->Freq = __LL_TIM_CALC_ARR(APB1_TIMER_CLK, LL_TIM_GetPrescaler(PWMx->TIMx), 100000);
    LL_TIM_SetAutoReload(PWMx->TIMx, PWMx->Freq);
    
    // Set PWM DUTY for channel 1
    PWMx->Duty = 0;
    LL_TIM_OC_SetCompareCH1(PWMx->TIMx, PWMx->Duty);

    // Enable the PWM FREQ and PWM DUTY
    LL_TIM_EnableARRPreload(PWMx->TIMx);
    LL_TIM_OC_EnablePreload(PWMx->TIMx, PWMx->Channel);

    // After change and enable make an event update
    LL_TIM_GenerateEvent_UPDATE(PWMx->TIMx);
}

/**
  * @brief  Enable the PWM.
  * @param  PWMx->TIMx Timer instance
  * @param  PWMx->Channel This parameter can be one of the following values:
  *         @arg @ref LL_TIM_CHANNEL_CH1
  *         @arg @ref LL_TIM_CHANNEL_CH2
  *         @arg @ref LL_TIM_CHANNEL_CH3
  *         @arg @ref LL_TIM_CHANNEL_CH4
  * @retval None
  */
void PWM_Enable(PWM_TypeDef *PWMx)
{
    if((PWMx->TIMx == TIM16) || (PWMx->TIMx == TIM17))
    {
        // Sellect off state when run or disable
        LL_TIM_SetOffStates(PWMx->TIMx, LL_TIM_OSSI_ENABLE, LL_TIM_OSSR_ENABLE);

        // Enable the PWM to output
        LL_TIM_CC_EnableChannel(PWMx->TIMx, PWMx->Channel);

        // Disable Automatic output enable
        // so that MOE (Main output enable) can be
        // set only by software
        LL_TIM_DisableAutomaticOutput(PWMx->TIMx);

        // Enable MOE (Main output enable)
        LL_TIM_EnableAllOutputs(PWMx->TIMx);

        // Enable the PWM to output
        LL_TIM_CC_EnableChannel(PWMx->TIMx, PWMx->Channel);
        LL_TIM_EnableCounter(PWMx->TIMx);
    }
    else
    {
        // Enable the PWM to output
        LL_TIM_CC_EnableChannel(PWMx->TIMx, PWMx->Channel);
        LL_TIM_EnableCounter(PWMx->TIMx);
    }
}

/**
  * @brief  Disable the PWM.
  * @param  PWMx->TIMx Timer instance
  * @param  PWMx->Channel This parameter can be one of the following values:
  *         @arg @ref LL_TIM_CHANNEL_CH1
  *         @arg @ref LL_TIM_CHANNEL_CH2
  *         @arg @ref LL_TIM_CHANNEL_CH3
  *         @arg @ref LL_TIM_CHANNEL_CH4
  * @retval None
  */
void PWM_Disable(PWM_TypeDef *PWMx)
{
    if((PWMx->TIMx == TIM16) || (PWMx->TIMx == TIM17))
    {
        // Enable MOE (Main output enable)
        LL_TIM_DisableAllOutputs(PWMx->TIMx);

        // Disable the PWM to output
        LL_TIM_DisableCounter(PWMx->TIMx);
        LL_TIM_CC_DisableChannel(PWMx->TIMx, PWMx->Channel);
    }
    else
    {
        // Disable the PWM to output
        LL_TIM_DisableCounter(PWMx->TIMx);
        LL_TIM_CC_DisableChannel(PWMx->TIMx, PWMx->Channel);
    }
}

void PWM_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty)
{
    // Limit the duty to 100
    if (_Duty > 100)
      return;

    // Set PWM DUTY for channel 1
    PWMx->Duty = (PWMx->Freq * (_Duty / 100.0)) + 1;
    LL_TIM_OC_SetCompareCH1(PWMx->TIMx, PWMx->Duty);
}

void PWM_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq)
{
    // Set PWM FREQ
    PWMx->Freq = __LL_TIM_CALC_ARR(APB1_TIMER_CLK, LL_TIM_GetPrescaler(PWMx->TIMx), _Freq);
    LL_TIM_SetAutoReload(PWMx->TIMx, PWMx->Freq);
}
