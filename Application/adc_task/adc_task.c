/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "app.h"

#include "adc_task.h"
#include "crc.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define CURRENT_SENSE_VALUE \
__LL_ADC_CALC_DATA_TO_VOLTAGE(11000, LL_ADC_REG_ReadConversionData12(ADC_I_SENSE_HANDLE), LL_ADC_RESOLUTION_12B)

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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static bool is_ADC_read_completed   = false;

static uint16_t Current_Sense_Count = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static void fsp_print(uint8_t packet_length);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef GPC_UART;

bool        is_Measure_Impedance    = false;

uint16_t    Current_Sense_Period    = 1000;
uint32_t    Current_Sense_Sum       = 0;
uint16_t    Current_Sense_Average   = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: Current Sense Task Init :::::::: */
void Current_Sense_Task_Init(uint32_t Sampling_Time)
{
    ADC_Init(ADC_I_SENSE_HANDLE, Sampling_Time);

    LL_ADC_REG_SetSequencerChannels(ADC_I_SENSE_HANDLE, ADC_I_SENSE_CHANNEL);
    LL_ADC_REG_SetSequencerDiscont(ADC_I_SENSE_HANDLE, LL_ADC_REG_SEQ_DISCONT_1RANK);

    LL_ADC_EnableIT_EOC(ADC_I_SENSE_HANDLE);
}

/* :::::::::: Current Sense Task ::::::::::::: */
void Current_Sense_Task(void*)
{
    if ((is_ADC_read_completed == true) && (Current_Sense_Count < Current_Sense_Period))
    {
        is_ADC_read_completed = false;
        Current_Sense_Count++;

        Current_Sense_Sum += CURRENT_SENSE_VALUE;
        LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
    }
    
    if (Current_Sense_Count >= Current_Sense_Period)
    {
        Current_Sense_Average = (Current_Sense_Sum / Current_Sense_Period) * 1.132139958;

        if (is_Measure_Impedance == true)
        {
            V_Switch_Set_Mode(V_SWITCH_MODE_ALL_OFF);
            H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
            H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_LS_ON);
            is_Measure_Impedance = false;

            pu_GPP_FSP_Payload->get_impedance.Cmd 	     = FSP_CMD_SENT_IMPEDANCE;
            pu_GPP_FSP_Payload->get_impedance.Value_low  = Current_Sense_Average;
            pu_GPP_FSP_Payload->get_impedance.Value_high = (Current_Sense_Average >> 8);
        }
        else
        {
            pu_GPP_FSP_Payload->get_current.Cmd 	     = FSP_CMD_SENT_CURRENT;
            pu_GPP_FSP_Payload->get_current.Value_low    = Current_Sense_Average;
            pu_GPP_FSP_Payload->get_current.Value_high   = (Current_Sense_Average >> 8);
        }

        fsp_print(3);

        Current_Sense_Sum       = 0;
        Current_Sense_Count     = 0;
        Current_Sense_Period    = 1000;
        
        SchedulerTaskDisable(3);
    }
}

/* :::::::::: ADC Interupt Handler ::::::::::::: */
void Current_Sense_ADC_IRQHandler(void)
{
    if(LL_ADC_IsActiveFlag_EOC(ADC_I_SENSE_HANDLE) == true)
    {
        is_ADC_read_completed = true;
        LL_ADC_ClearFlag_EOC(ADC_I_SENSE_HANDLE);
    }

}

void Current_Sense_TIMER_IRQHandler(void)
{
    ;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static void fsp_print(uint8_t packet_length)
{
    s_GPP_FSP_Packet.sod 		= FSP_PKT_SOD;
    s_GPP_FSP_Packet.src_adr 	= fsp_my_adr;
    s_GPP_FSP_Packet.dst_adr 	= FSP_ADR_GPC;
    s_GPP_FSP_Packet.length 	= packet_length;
    s_GPP_FSP_Packet.type 		= FSP_PKT_TYPE_CMD_W_DATA;
    s_GPP_FSP_Packet.eof 		= FSP_PKT_EOF;
    s_GPP_FSP_Packet.crc16 		= crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &s_GPP_FSP_Packet.src_adr, s_GPP_FSP_Packet.length + 4);

    uint8_t encoded_frame[20] = { 0 };
    uint8_t frame_len;
    fsp_encode(&s_GPP_FSP_Packet, encoded_frame, &frame_len);

    UART_FSP(&GPC_UART, (char*)encoded_frame, frame_len);
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
