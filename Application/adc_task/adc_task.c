/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "app.h"

#include "h_bridge_driver.h"
#include "v_switch_driver.h"

#include "adc_task.h"
#include "crc.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
typedef struct _current_sense_typedef {
	uint16_t    buffer_size;
	char        *p_buffer;

	uint16_t    write_index;
	uint16_t    read_value;
}current_sense_typedef;

typedef enum
{
    IMPEDANCE_STOP_STATE,
    IMPEDANCE_MEASURE_STATE,
} Impedance_State_typedef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static bool is_ADC_read_completed               = false;
static Impedance_State_typedef Impedance_State  = IMPEDANCE_STOP_STATE;

static uint16_t Impedance_Measure_Count = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef GPC_UART;

bool        is_impedance_task_enable    = false;
uint16_t    Impedance_Measure_Period    = 1000;
uint32_t    Impedance_Current_Average   = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: ADC Task Init :::::::: */
void ADC_Task_Init(uint32_t Sampling_Time)
{
    ADC_Init(ADC_I_SENSE_HANDLE, Sampling_Time);

    LL_ADC_REG_SetSequencerChannels(ADC_I_SENSE_HANDLE, ADC_I_SENSE_CHANNEL);
    LL_ADC_REG_SetSequencerDiscont(ADC_I_SENSE_HANDLE, LL_ADC_REG_SEQ_DISCONT_1RANK);

    LL_ADC_EnableIT_EOC(ADC_I_SENSE_HANDLE);
}

/* :::::::::: ADC Task ::::::::::::: */
//TODO: Áp dụng ring buffer và mạch lọc cho ADC.
void ADC_Task(void*)
{
    if (is_ADC_read_completed == true)
    {
    	is_ADC_read_completed = false;
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
        V_Switch_Set_Mode(V_SWITCH_MODE_LV_ON);
        H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
        H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_HS_ON);

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

            V_Switch_Set_Mode(V_SWITCH_MODE_ALL_OFF);
            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);

            is_impedance_task_enable    = false;
            Impedance_State             = IMPEDANCE_STOP_STATE;

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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
