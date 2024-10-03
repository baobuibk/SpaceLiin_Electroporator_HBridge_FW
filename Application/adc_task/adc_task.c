/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "adc_task.h"

#include "app.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static bool is_ADC_read_completed       = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
uint16_t g_Feedback_Current = 0;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: ADC Task Init :::::::: */
void ADC_Task_Init(uint32_t Sampling_Time)
{

    ADC_Init(ADC_I_SENSE_HANDLE, Sampling_Time);

    LL_ADC_REG_SetSequencerChannels(ADC_I_SENSE_HANDLE, ADC_I_SENSE_CHANNEL);
    LL_ADC_REG_SetSequencerDiscont(ADC_I_SENSE_HANDLE, LL_ADC_REG_SEQ_DISCONT_1RANK);

    LL_ADC_EnableIT_EOC(ADC_I_SENSE_HANDLE);
//    LL_ADC_EnableIT_EOS(ADC_I_SENSE_HANDLE);
    LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);

}

/* :::::::::: ADC Task ::::::::::::: */
//TODO: Áp dụng ring buffer và mạch lọc cho ADC.
void ADC_Task(void*)
{
    uint16_t ADC_Value = 0;

    if (is_ADC_read_completed == true)
    {
    	is_ADC_read_completed = false;
        ADC_Value = LL_ADC_REG_ReadConversionData12(ADC_I_SENSE_HANDLE);
        g_Feedback_Current = __LL_ADC_CALC_DATA_TO_VOLTAGE(11000, ADC_Value, LL_ADC_RESOLUTION_12B);
        LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
    }

}

/* :::::::::: ADC Interupt Handler ::::::::::::: */
void ADC_Task_IRQHandler(void)
{
    if(LL_ADC_IsActiveFlag_EOC(ADC_I_SENSE_HANDLE) == true)
    {
        is_ADC_read_completed = true;
        LL_ADC_ClearFlag_EOC(ADC_I_SENSE_HANDLE);
    }

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
