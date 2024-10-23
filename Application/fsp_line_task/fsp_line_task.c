/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "fsp_line_task.h"
#include <stdio.h>
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct _fsp_line_typedef {
	uint16_t buffer_size;
	char *p_buffer;

	volatile uint16_t write_index;
	volatile char RX_char;
};
extern Accel_Gyro_DataTypedef _gyro, _accel;
void convertTemperature(float temp, uint8_t buf[]);
void convertIntegerToBytes(int number, uint8_t arr[]);
typedef struct _fsp_line_typedef fsp_line_typedef;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//static const char * ErrorCode[7] =
//{
//    "OK\n",
//    "FSP_PKT_NOT_READY\n",
//    "FSP_PKT_INVALID\n",
//    "FSP_PKT_WRONG_ADR\n",
//    "FSP_PKT_ERROR\n",
//    "FSP_PKT_CRC_FAIL\n",
//    "FSP_PKT_WRONG_LENGTH\n"
//};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static void fsp_print(uint8_t packet_length);
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef RS232_UART;
uart_stdio_typedef GPC_UART;
extern double compensated_pressure;
extern double compensated_temperature;
char g_GPC_UART_TX_buffer[GPP_TX_SIZE];
char g_GPC_UART_RX_buffer[GPP_RX_SIZE];

fsp_packet_t 		s_GPC_FSP_Packet;
fsp_packet_t 		s_GPP_FSP_Packet;
GPC_FSP_Payload 	*pu_GPC_FSP_Payload;		//for RX
GPP_FSP_Payload 	*pu_GPP_FSP_Payload;		//for TX

fsp_line_typedef FSP_line;
char g_FSP_line_buffer[FSP_BUF_LEN];

bool is_receive_SOD = false;
bool escape = false;
float temp;
uint32_t press;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
void FSP_Line_Task_Init() {
	UART_Init(	&GPC_UART, GPC_UART_HANDLE, GPC_UART_IRQ, g_GPC_UART_TX_buffer,
				g_GPC_UART_RX_buffer, GPP_TX_SIZE, GPP_RX_SIZE);

	FSP_line.p_buffer = g_FSP_line_buffer;
	FSP_line.buffer_size = FSP_BUF_LEN;
	FSP_line.write_index = 0;

	pu_GPP_FSP_Payload = (GPP_FSP_Payload*) (&s_GPP_FSP_Packet.payload);
	pu_GPC_FSP_Payload = (GPC_FSP_Payload*) (&s_GPC_FSP_Packet.payload);

	if (FSP_line.buffer_size != 0) {
		memset((void*) FSP_line.p_buffer, 0, sizeof(FSP_line.p_buffer));
	}
	fsp_init(FSP_ADR_GPP);
}

/* :::::::::: CMD Line Task ::::::::::::: */

void FSP_Line_Task(void) {
	uint8_t time_out = 0;

	for (time_out = 50, escape = false; (!RX_BUFFER_EMPTY(&GPC_UART)) && (time_out != 0) && (escape == false); time_out--)
	{
		FSP_line.RX_char = UART_Get_Char(&GPC_UART);
		escape = false;

		if (FSP_line.RX_char == FSP_PKT_SOD)
		{
			FSP_line.write_index = 0;
			is_receive_SOD = true;
		} 
		else if ((FSP_line.RX_char == FSP_PKT_EOF) && (is_receive_SOD == true))
		{
			switch (frame_decode((uint8_t*) FSP_line.p_buffer, FSP_line.write_index, &s_GPC_FSP_Packet))
			{
			//process command
			case FSP_PKT_NOT_READY:
				break;
			case FSP_PKT_READY:
				UART_Send_String(&RS232_UART, "Received FSP packet\r\n");
				FSP_Line_Process();

				break;
			case FSP_PKT_INVALID:

				break;
			case FSP_PKT_WRONG_ADR:
				UART_Send_String(&RS232_UART, "Wrong module adr\r\n");

				break;
			case FSP_PKT_ERROR:
				UART_Send_String(&RS232_UART, "Packet error\r\n");

				break;
			case FSP_PKT_CRC_FAIL:
				UART_Send_String(&RS232_UART, "CRC error\r\n");
				break;
			default:

				break;
			}
			FSP_line.write_index = 0;
			is_receive_SOD = false;

			escape = true;
		} else {
			FSP_line.p_buffer[FSP_line.write_index] = FSP_line.RX_char;
			FSP_line.write_index++;

			if (FSP_line.write_index > FSP_line.buffer_size)
				FSP_line.write_index = 0;

		}
	}
}

/* :::::::::: IRQ Handler ::::::::::::: */
void GPC_UART_IRQHandler(void) {
	if (LL_USART_IsActiveFlag_TXE(GPC_UART.handle) == true) {
		if (TX_BUFFER_EMPTY(&GPC_UART)) {
			// Buffer empty, so disable interrupts
			LL_USART_DisableIT_TXE(GPC_UART.handle);
		} else {
			// There is more data in the output buffer. Send the next byte
			UART_Prime_Transmit(&GPC_UART);
		}
	}

	if (LL_USART_IsActiveFlag_RXNE(GPC_UART.handle) == true) {
		GPC_UART.RX_irq_char = LL_USART_ReceiveData8(GPC_UART.handle);

		if (!RX_BUFFER_FULL(&GPC_UART))
		{
			GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] = GPC_UART.RX_irq_char;
			ADVANCE_RX_WRITE_INDEX(&GPC_UART);
		}
	}
}

uint8_t hs_relay_pole, ls_relay_pole, relay_state;
void FSP_Line_Process()
{
	switch (pu_GPC_FSP_Payload->commonFrame.Cmd)
	{
	case FSP_CMD_SET_PULSE_COUNT:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_COUNT\r\n");
		hv_pulse_pos_count 	= pu_GPC_FSP_Payload->set_pulse_count.HV_pos_count;
		hv_pulse_neg_count 	= pu_GPC_FSP_Payload->set_pulse_count.HV_neg_count;

		lv_pulse_pos_count 	= pu_GPC_FSP_Payload->set_pulse_count.LV_pos_count;
		lv_pulse_neg_count 	= pu_GPC_FSP_Payload->set_pulse_count.LV_neg_count;
		break;
	case FSP_CMD_SET_PULSE_DELAY:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_DELAY\r\n");
		hv_delay_ms = pu_GPC_FSP_Payload->set_pulse_delay.HV_delay;
		lv_delay_ms	= pu_GPC_FSP_Payload->set_pulse_delay.LV_delay;

		pulse_delay_ms = pu_GPC_FSP_Payload->set_pulse_delay.Delay_high;
		pulse_delay_ms <<= 8;
		pulse_delay_ms |= pu_GPC_FSP_Payload->set_pulse_delay.Delay_low;
		break;
	case FSP_CMD_SET_PULSE_HV:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_HV\r\n");
		hv_on_time_ms = pu_GPC_FSP_Payload->set_pulse_HV.OnTime;
		hv_off_time_ms = pu_GPC_FSP_Payload->set_pulse_HV.OffTime;
		break;
	case FSP_CMD_SET_PULSE_LV:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_LV\r\n");
		lv_on_time_ms 	= pu_GPC_FSP_Payload->set_pulse_LV.OnTime_high;
		lv_on_time_ms   <<= 8;
		lv_on_time_ms	|= pu_GPC_FSP_Payload->set_pulse_LV.OnTime_low;

		lv_off_time_ms	= pu_GPC_FSP_Payload->set_pulse_LV.OffTime_high;
		lv_off_time_ms	<<= 8;
		lv_off_time_ms	|= pu_GPC_FSP_Payload->set_pulse_LV.OffTime_low;
		break;
	case FSP_CMD_SET_PULSE_CONTROL:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_CONTROL\r\n");
		is_h_bridge_enable = pu_GPC_FSP_Payload->set_pulse_control.State;
		SchedulerTaskEnable(0, 1);
		break;
	
	case FSP_CMD_GET_PULSE_COUNT:
		pu_GPP_FSP_Payload->send_pulse_count.Cmd = FSP_CMD_SEND_PULSE_COUNT;

		pu_GPP_FSP_Payload->send_pulse_count.HV_pos_count = hv_pulse_pos_count;
		pu_GPP_FSP_Payload->send_pulse_count.HV_neg_count = hv_pulse_neg_count;

		pu_GPP_FSP_Payload->send_pulse_count.LV_pos_count = lv_pulse_pos_count;
		pu_GPP_FSP_Payload->send_pulse_count.LV_neg_count = lv_pulse_neg_count;

		fsp_print(5);
		break;
	case FSP_CMD_GET_PULSE_DELAY:
		pu_GPP_FSP_Payload->send_pulse_delay.Cmd = FSP_CMD_SEND_PULSE_DELAY;

		pu_GPP_FSP_Payload->send_pulse_delay.HV_delay = hv_delay_ms;
		pu_GPP_FSP_Payload->send_pulse_delay.LV_delay = lv_delay_ms;

		pu_GPP_FSP_Payload->send_pulse_delay.Delay_low = pulse_delay_ms;
		pu_GPP_FSP_Payload->send_pulse_delay.Delay_high = pulse_delay_ms >> 8;

		fsp_print(5);
		break;
	case FSP_CMD_GET_PULSE_HV:
		pu_GPP_FSP_Payload->send_pulse_HV.Cmd = FSP_CMD_SEND_PULSE_HV;

		pu_GPP_FSP_Payload->send_pulse_HV.OnTime = hv_on_time_ms;
		pu_GPP_FSP_Payload->send_pulse_HV.OffTime = hv_off_time_ms;

		fsp_print(3);
		break;
	case FSP_CMD_GET_PULSE_LV:
		pu_GPP_FSP_Payload->send_pulse_LV.Cmd = FSP_CMD_SEND_PULSE_LV;

		pu_GPP_FSP_Payload->send_pulse_LV.OnTime_low = lv_on_time_ms;
		pu_GPP_FSP_Payload->send_pulse_LV.OnTime_high = lv_on_time_ms >> 8;

		pu_GPP_FSP_Payload->send_pulse_LV.OffTime_low = lv_off_time_ms;
		pu_GPP_FSP_Payload->send_pulse_LV.OffTime_high = lv_off_time_ms >> 8;

		fsp_print(5);
		break;
	case FSP_CMD_GET_PULSE_CONTROL:
		pu_GPP_FSP_Payload->send_pulse_control.Cmd = FSP_CMD_SEND_PULSE_CONTROL;

		pu_GPP_FSP_Payload->send_pulse_control.State = is_h_bridge_enable;

		fsp_print(2);
		break;
	case FSP_CMD_GET_PULSE_ALL:
		pu_GPP_FSP_Payload->send_pulse_all.Cmd = FSP_CMD_SEND_PULSE_ALL;

		pu_GPP_FSP_Payload->send_pulse_all.HV_pos_count = hv_pulse_pos_count;
		pu_GPP_FSP_Payload->send_pulse_all.HV_neg_count = hv_pulse_neg_count;

		pu_GPP_FSP_Payload->send_pulse_all.LV_pos_count = lv_pulse_pos_count;
		pu_GPP_FSP_Payload->send_pulse_all.LV_neg_count = lv_pulse_neg_count;

		pu_GPP_FSP_Payload->send_pulse_all.HV_delay = hv_delay_ms;
		pu_GPP_FSP_Payload->send_pulse_all.LV_delay = lv_delay_ms;

		pu_GPP_FSP_Payload->send_pulse_all.Delay_low = pulse_delay_ms;
		pu_GPP_FSP_Payload->send_pulse_all.Delay_high = pulse_delay_ms >> 8;

		pu_GPP_FSP_Payload->send_pulse_all.OnTime = hv_on_time_ms;
		pu_GPP_FSP_Payload->send_pulse_all.OffTime = hv_off_time_ms;

		pu_GPP_FSP_Payload->send_pulse_all.OnTime_low = lv_on_time_ms;
		pu_GPP_FSP_Payload->send_pulse_all.OnTime_high = lv_on_time_ms >> 8;

		pu_GPP_FSP_Payload->send_pulse_all.OffTime_low = lv_off_time_ms;
		pu_GPP_FSP_Payload->send_pulse_all.OffTime_high = lv_off_time_ms >> 8;

		pu_GPP_FSP_Payload->send_pulse_all.State = is_h_bridge_enable;

		fsp_print(16);
		break;

	case FSP_CMD_SET_RELAY_POLE:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_RELAY_SET\r\n");
		hs_relay_pole = pu_GPC_FSP_Payload->set_relay_pole.HvRelay;
		ls_relay_pole = pu_GPC_FSP_Payload->set_relay_pole.LvRelay;

		decode_hs_relay(hs_relay_pole);
		decode_ls_relay(ls_relay_pole);
		break;
	case FSP_CMD_SET_RELAY_CONTROL:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_RELAY_CONTROL\r\n");
		relay_state = pu_GPC_FSP_Payload->set_relay_control.State;

		if (relay_state == 0) {
			LL_GPIO_ResetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
			LL_GPIO_ResetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
		} else {
			LL_GPIO_SetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
			LL_GPIO_SetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
		}
		break;

	case FSP_CMD_GET_RELAY_POLE:
		pu_GPP_FSP_Payload->send_relay_pole.Cmd = FSP_CMD_SEND_RELAY_POLE;

		pu_GPP_FSP_Payload->send_relay_pole.HvRelay = hs_relay_pole;
		pu_GPP_FSP_Payload->send_relay_pole.LvRelay = ls_relay_pole;

		fsp_print(3);
		break;
	case FSP_CMD_GET_RELAY_CONTROL:
		pu_GPP_FSP_Payload->send_relay_control.Cmd = FSP_CMD_SEND_RELAY_CONTROL;

		pu_GPP_FSP_Payload->send_relay_control.State = relay_state;

		fsp_print(2);
		break;
	case FSP_CMD_GET_RELAY_ALL:
		pu_GPP_FSP_Payload->send_relay_all.Cmd = FSP_CMD_SEND_RELAY_ALL;

		pu_GPP_FSP_Payload->send_relay_all.HvRelay = hs_relay_pole;
		pu_GPP_FSP_Payload->send_relay_all.LvRelay = ls_relay_pole;

		pu_GPP_FSP_Payload->send_relay_all.State = relay_state;

		fsp_print(4);
		break;


	case FSP_CMD_CHANNEL_SET:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_CHANNEL_SET\r\n");
		Channel_Set = pu_GPC_FSP_Payload->channelSet.Channel;
		break;
	case FSP_CMD_CHANNEL_CONTROL:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_CHANNEL_CONTROL\r\n");
		is_v_switch_enable = pu_GPC_FSP_Payload->channelControl.State;
		break;

	case FSP_CMD_GET_CURRENT:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_GET_CURRENT\r\n");
		Current_Sense_Period = 1000;
		LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);
		SchedulerTaskEnable(2, 1);
		break;

	case FSP_CMD_GET_IMPEDANCE:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_GET_IMPEDANCE\r\n");
		Current_Sense_Period	= pu_GPC_FSP_Payload->get_impedance.Period * 1000;
		is_h_bridge_enable 		= false;
		is_Measure_Impedance 	= true;

		V_Switch_Set_Mode(V_SWITCH_MODE_HV_ON);
		H_Bridge_Set_Mode(&H_Bridge_2, H_BRIDGE_MODE_LS_ON);
		H_Bridge_Set_Mode(&H_Bridge_1, H_BRIDGE_MODE_HS_ON);

		LL_ADC_REG_StartConversion(ADC_I_SENSE_HANDLE);

		SchedulerTaskEnable(2, 1);
		break;

	case FSP_CMD_HANDSHAKE:
		UART_Send_String(&RS232_UART, "Received FSP_CMD_HANDSHAKE\r\n");
		pu_GPP_FSP_Payload->handshake.Cmd 	= FSP_CMD_HANDSHAKE;
		pu_GPP_FSP_Payload->handshake.Check = 0xAB;
		
		fsp_print(2);
		break;
	case FSP_CMD_GET_BMP390:
		UART_Send_String(&RS232_UART, "Received BMP_390 command\r\n");
		pu_GPP_FSP_Payload->getBMP390.Cmd = FSP_CMD_GET_BMP390	;
		float temp = (float)compensated_temperature;
		uint32_t press= (uint32_t)compensated_pressure;

		convertTemperature(temp, pu_GPP_FSP_Payload->getBMP390.temp);

		sprintf(pu_GPP_FSP_Payload->getBMP390.pressure, "%d", press);

		fsp_print(12);
		break;
	case FSP_CMD_GET_LMSDOX:
		UART_Send_String(&RS232_UART, "Received GET_LSMDOX command\r\n");
		pu_GPP_FSP_Payload->getLSMDOX.Cmd = FSP_CMD_GET_LMSDOX;

		convertIntegerToBytes(_accel.x, pu_GPP_FSP_Payload->getLSMDOX.accel_x);
		convertIntegerToBytes(_accel.y, pu_GPP_FSP_Payload->getLSMDOX.accel_y);
		convertIntegerToBytes(_accel.z, pu_GPP_FSP_Payload->getLSMDOX.accel_z);

		convertIntegerToBytes(_gyro.x, pu_GPP_FSP_Payload->getLSMDOX.gyro_x);
		convertIntegerToBytes(_gyro.y, pu_GPP_FSP_Payload->getLSMDOX.gyro_y);
		convertIntegerToBytes(_gyro.z, pu_GPP_FSP_Payload->getLSMDOX.gyro_z);

		fsp_print(25);
		break;
	default:
		break;
	}
}

static void fsp_print(uint8_t packet_length)
{
	s_GPP_FSP_Packet.sod 		= FSP_PKT_SOD;
	s_GPP_FSP_Packet.src_adr 	= fsp_my_adr;
	s_GPP_FSP_Packet.dst_adr 	= FSP_ADR_GPC;
	s_GPP_FSP_Packet.length 	= packet_length;
	s_GPP_FSP_Packet.type 		= FSP_PKT_TYPE_CMD_W_DATA;
	s_GPP_FSP_Packet.eof 		= FSP_PKT_EOF;
	s_GPP_FSP_Packet.crc16 		= crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &s_GPP_FSP_Packet.src_adr, s_GPP_FSP_Packet.length + 4);

	uint8_t encoded_frame[100] = { 0 };
	uint8_t frame_len;
	fsp_encode(&s_GPP_FSP_Packet, encoded_frame, &frame_len);

	UART_FSP(&GPC_UART, encoded_frame, frame_len);
}

void convertTemperature(float temp, uint8_t buf[]) {
	// temperature is xxx.x format
	//float to byte

	gcvt(temp, 5, buf);

}
void convertIntegerToBytes(int number, uint8_t arr[]) {
	arr[0]= number & 0xff;
	arr[1]= (number >>8 ) & 0xff;
	arr[2] = (number >>16) & 0xff;
	arr[3] = (number >>24) & 0xff;

}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
