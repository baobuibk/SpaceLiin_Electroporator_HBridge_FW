/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "stm32f0xx_ll_rcc.h"

#include "adc_task.h"
#include "pwm.h"
#include "crc.h"

#include "app.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct _current_sense_typedef {
	uint16_t    buffer_size;
	char        *p_buffer;

	uint16_t    write_index;
	uint16_t    read_value;
};
typedef struct _current_sense_typedef current_sense_typedef;

typedef enum
{
    IMPEDANCE_STOP_STATE,
    IMPEDANCE_MEASURE_STATE,
} Impedance_State_typedef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static bool is_ADC_read_completed       = false;
static Impedance_State_typedef Impedance_State = IMPEDANCE_STOP_STATE;

static uint16_t Impedance_Measure_Count = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static inline void V_Switch_Set_Freq(PWM_TypeDef *PWMx, uint32_t _Freq);
static inline void V_Switch_Set_Duty(PWM_TypeDef *PWMx, uint32_t _Duty);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef GPC_UART;

extern PWM_TypeDef V_Switch_1_PWM;
extern PWM_TypeDef H_Bridge_1_PWM;
extern PWM_TypeDef H_Bridge_2_PWM;

//uint16_t g_Feedback_Current_buffer[3072];

//current_sense_typedef Current_Sense;

bool        is_impedance_task_enable = false;
uint16_t    Impedance_Measure_Period = 1000;
uint32_t    Impedance_Current_Average    = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: ADC Task Init :::::::: */
void ADC_Task_Init(uint32_t Sampling_Time)
{
    //Current_Sense.p_buffer      = g_Feedback_Current_buffer;
	//Current_Sense.buffer_size   = 3072;
	//Current_Sense.write_index   = 0;
    //Current_Sense.read_value    = 0;

	//if (Current_Sense.buffer_size != 0) {
		//memset((void*) Current_Sense.p_buffer, 0, sizeof(Current_Sense.p_buffer));
	//}

    ADC_Init(ADC_I_SENSE_HANDLE, Sampling_Time);

    LL_ADC_REG_SetSequencerChannels(ADC_I_SENSE_HANDLE, ADC_I_SENSE_CHANNEL);
    LL_ADC_REG_SetSequencerDiscont(ADC_I_SENSE_HANDLE, LL_ADC_REG_SEQ_DISCONT_1RANK);

    LL_ADC_EnableIT_EOC(ADC_I_SENSE_HANDLE);
    //LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
}

/* :::::::::: ADC Task ::::::::::::: */
//TODO: Áp dụng ring buffer và mạch lọc cho ADC.
void ADC_Task(void*)
{
    //uint16_t ADC_Value = 0;

    if (is_ADC_read_completed == true)
    {
    	is_ADC_read_completed = false;
        //ADC_Value = LL_ADC_REG_ReadConversionData12(ADC_I_SENSE_HANDLE);
        //Current_Sense.read_value = __LL_ADC_CALC_DATA_TO_VOLTAGE(11000, ADC_Value, LL_ADC_RESOLUTION_12B);
        //Current_Sense.p_buffer[Current_Sense.write_index] = Current_Sense.read_value;
        //Current_Sense.write_index++;
        LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
    }

}

void Impedance_Task(void*)
{
    switch (Impedance_State)
    {
    case IMPEDANCE_STOP_STATE:
    if (is_impedance_task_enable == true)
    {
        LL_GPIO_ResetOutputPin(V_SWITCH_HIN2_PORT, V_SWITCH_HIN2_PIN);
        LL_TIM_OC_SetMode(V_SWITCH_LIN2_HANDLE, V_SWITCH_LIN2_CHANNEL, LL_TIM_OCMODE_PWM1);

        // STOP THE CNT AND RESET IT TO 0.
        LL_TIM_DisableCounter(V_SWITCH_LIN1_HANDLE);
        PWM_Set_Freq(&V_Switch_1_PWM, 1000 / 100);
        PWM_Set_Duty(&V_Switch_1_PWM, 0);
        LL_TIM_OC_SetMode(V_SWITCH_LIN1_HANDLE, V_SWITCH_LIN1_CHANNEL, LL_TIM_OCMODE_PWM1);
        V_Switch_Set_Freq(&V_Switch_1_PWM, 10000);
        V_Switch_Set_Duty(&V_Switch_1_PWM, 50);

        LL_TIM_ClearFlag_UPDATE(V_SWITCH_LIN1_HANDLE);
        LL_TIM_EnableIT_UPDATE(V_SWITCH_LIN1_HANDLE);
        LL_TIM_EnableCounter(V_SWITCH_LIN1_HANDLE);

        //LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_INACTIVE);
        LL_GPIO_SetOutputPin(H_BRIDGE_HIN1_PORT, H_BRIDGE_HIN1_PIN);
        //LL_TIM_OC_SetMode(H_BRIDGE_SD1_HANDLE, H_BRIDGE_SD1_CHANNEL, LL_TIM_OCMODE_FORCED_ACTIVE);

        Impedance_Current_Average   = 0;
        Impedance_Measure_Count     = 0;
        Impedance_State = IMPEDANCE_MEASURE_STATE;
        LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
    }
    
        break;
    case IMPEDANCE_MEASURE_STATE:
        uint16_t ADC_Value = 0;
        
        if (is_ADC_read_completed == true)
        {
            Impedance_Measure_Count++;
            is_ADC_read_completed = false;
            ADC_Value = LL_ADC_REG_ReadConversionData12(ADC_I_SENSE_HANDLE);
            Impedance_Current_Average += __LL_ADC_CALC_DATA_TO_VOLTAGE(11000, ADC_Value, LL_ADC_RESOLUTION_12B);
            if (Impedance_Measure_Count < Impedance_Measure_Period)
                LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
        }
        
        if (Impedance_Measure_Count >= Impedance_Measure_Period)
        //if (0)
        {
            Impedance_Current_Average /= Impedance_Measure_Period;
            pu_GPP_FSP_Payload->avr_current.Cmd 	        = FSP_CMD_AVR_CURRENT;
			pu_GPP_FSP_Payload->avr_current.Value_low       = Impedance_Current_Average;
            pu_GPP_FSP_Payload->avr_current.Value_high      = (Impedance_Current_Average >> 8);
			s_GPP_FSP_Packet.sod 		= FSP_PKT_SOD;
			s_GPP_FSP_Packet.src_adr 	= fsp_my_adr;
			s_GPP_FSP_Packet.dst_adr 	= FSP_ADR_GPC;
			s_GPP_FSP_Packet.length 	= 3;
			s_GPP_FSP_Packet.type 		= FSP_PKT_TYPE_CMD_W_DATA;
			s_GPP_FSP_Packet.eof 		= FSP_PKT_EOF;
			s_GPP_FSP_Packet.crc16 		= crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &s_GPP_FSP_Packet.src_adr, s_GPP_FSP_Packet.length + 4);

			uint8_t encoded_frame[20] = { 0 };
			uint8_t frame_len;
			fsp_encode(&s_GPP_FSP_Packet, encoded_frame, &frame_len);

			UART_FSP(&GPC_UART, (char*)encoded_frame, frame_len);

            Impedance_Current_Average   = 0;
            Impedance_Measure_Count     = 0;

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

            is_impedance_task_enable = false;
            Impedance_State = IMPEDANCE_STOP_STATE;

            SchedulerTaskDisable(3);
        }
        break;

    default:
        Impedance_State = IMPEDANCE_STOP_STATE;
        break;
    }
}

/* :::::::::: ADC Interupt Handler ::::::::::::: */
void I_SENSE_ADC_IRQHandler(void)
{
    if(LL_ADC_IsActiveFlag_EOC(ADC_I_SENSE_HANDLE) == true)
    {
        is_ADC_read_completed = true;
        LL_ADC_ClearFlag_EOC(ADC_I_SENSE_HANDLE);
    }

}

void I_SENSE_TIMER_IRQHandler(void)
{
    ;
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
