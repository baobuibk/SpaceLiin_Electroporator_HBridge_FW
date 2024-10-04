/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "fsp_line_task.h"

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
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef RS232_UART;
uart_stdio_typedef GPC_UART;

char g_GPC_UART_TX_buffer[GPC_TX_BUF_LEN];
char g_GPC_UART_RX_buffer[GPC_RX_BUF_LEN];

fsp_packet_t s_GPC_FspPacket;
fsp_packet_t s_GPP_FspPacket;
GPC_Sfp_Payload *s_pGPC_Sfp_Payload;		//for RX
GPP_Sfp_Payload *s_pGPP_Sfp_Payload;		//for TX

fsp_line_typedef FSP_line;
char g_FSP_line_buffer[FSP_BUF_LEN];

bool is_receive_SOD = false;
bool escape = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
void FSP_Line_Task_Init() {
	UART_Init(&GPC_UART, GPC_UART_HANDLE, GPC_UART_IRQ, g_GPC_UART_TX_buffer,
			g_GPC_UART_RX_buffer,
			GPC_TX_BUF_LEN, GPC_RX_BUF_LEN);

	FSP_line.p_buffer = g_FSP_line_buffer;
	FSP_line.buffer_size = FSP_BUF_LEN;
	FSP_line.write_index = 0;

	s_pGPP_Sfp_Payload = (GPC_Sfp_Payload*) (&s_GPP_FspPacket.payload);
	s_pGPC_Sfp_Payload = (GPC_Sfp_Payload*) (&s_GPC_FspPacket.payload);

	if (FSP_line.buffer_size != 0) {
		memset((void*) FSP_line.p_buffer, 0, sizeof(FSP_line.p_buffer));
	}
	fsp_init(FSP_ADR_GPP);
}

/* :::::::::: CMD Line Task ::::::::::::: */

void FSP_Line_Task(void) {
	uint8_t time_out = 0;

	for (time_out = 50, escape = false;
			(!RX_BUFFER_EMPTY(&GPC_UART)) && (time_out != 0)
					&& (escape == false); time_out--) {
		FSP_line.RX_char = UART_Get_Char(&GPC_UART);
		escape = false;

		if (FSP_line.RX_char == FSP_PKT_SOD) {
			FSP_line.write_index = 0;
			is_receive_SOD = true;
		} else if ((FSP_line.RX_char == FSP_PKT_EOF)
				&& (is_receive_SOD == true)) {
			switch (frame_decode((uint8_t*) FSP_line.p_buffer,
					FSP_line.write_index, &s_GPC_FspPacket)) {
			//process command
			case FSP_PKT_NOT_READY:
				break;
			case FSP_PKT_READY:
				UART_Send_String(&RS232_UART, "Received FSP packet\r\n");
				FSP_PROCESS();
				//					COPC_Process();
				break;
			case FSP_PKT_INVALID:

				break;
			case FSP_PKT_WRONG_ADR:
				UART_Send_String(&RS232_UART, "Wrong module adr\r\n");
				//					usart1_send_string("Wrong module adr \r\n");
				//					usart1_send_array((const char *)COPC_RX_Buff,COPC_RX_Index);
				break;
			case FSP_PKT_ERROR:
				UART_Send_String(&RS232_UART, "Packet error\r\n");
				//					usart1_send_string("Packet error \r\n");
				//					usart1_send_array((const char *)COPC_RX_Buff,COPC_RX_Index);
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

		// NOTE: On win 10, default PUTTY when hit enter only send back '\r',
		// while on default HERCULES when hit enter send '\r\n' in that order.
		// The code bellow is modified so that it can work on PUTTY and HERCULES.
		if (!RX_BUFFER_FULL(&GPC_UART)) {
//            if (GPC_UART.RX_irq_char == '\r')
//            {
//                GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] = '\n';
//                ADVANCE_RX_WRITE_INDEX(&GPC_UART);
//            }
//            else
//            {
			GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] =
					GPC_UART.RX_irq_char;
			ADVANCE_RX_WRITE_INDEX(&GPC_UART);
//            }
		}
	}
}

void FSP_PROCESS() {
	switch (s_GPC_FspPacket.type) {
	case FSP_PKT_TYPE_DATA:

		break;
	case FSP_PKT_TYPE_DATA_WITH_ACK:

		break;
	case FSP_PKT_TYPE_CMD:

		break;
	case FSP_PKT_TYPE_CMD_WITH_ACK:

		break;
	case FSP_PKT_TYPE_ACK:

		break;
	case FSP_PKT_TYPE_NACK:

		break;
	case FSP_PKT_TYPE_CMD_W_DATA:
		switch (s_pGPC_Sfp_Payload->commonFrame.Cmd) {
		case FSP_CMD_PULSE_COUNT:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_COUNT\r\n");
			hv_pulse_count = s_pGPC_Sfp_Payload->pulseCount.HV_count;
			lv_pulse_count = s_pGPC_Sfp_Payload->pulseCount.LV_count;
			break;
		case FSP_CMD_PULSE_DELAY:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_DELAY\r\n");
			pulse_delay_ms = s_pGPC_Sfp_Payload->pulseDelay.Delay;
			break;
		case FSP_CMD_PULSE_HV:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_HV\r\n");
			hv_on_time_ms = s_pGPC_Sfp_Payload->pulseHV.OnTime;
			hv_off_time_ms = s_pGPC_Sfp_Payload->pulseHV.OffTime;
			break;
		case FSP_CMD_PULSE_LV:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_LV\r\n");
			lv_on_time_ms = s_pGPC_Sfp_Payload->pulseLV.OnTime;
			lv_off_time_ms = s_pGPC_Sfp_Payload->pulseLV.OffTime;
			break;
		case FSP_CMD_PULSE_CONTROL:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_PULSE_CONTROL\r\n");
			is_h_bridge_enable = s_pGPC_Sfp_Payload->pulseControl.State;
			break;
		case FSP_CMD_RELAY_SET:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_RELAY_SET\r\n");
			decode_hs_relay(s_pGPC_Sfp_Payload->relaySet.HvRelay);
			decode_ls_relay(s_pGPC_Sfp_Payload->relaySet.LvRelay);
			break;
		case FSP_CMD_RELAY_CONTROL:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_RELAY_CONTROL\r\n");
			if (s_pGPC_Sfp_Payload->relayControl.State == 0) {
				LL_GPIO_ResetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
				LL_GPIO_ResetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
			} else {
				LL_GPIO_SetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
				LL_GPIO_SetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
			}
			break;
		case FSP_CMD_CHANNEL_SET:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_CHANNEL_SET\r\n");
			Channel_Set = s_pGPC_Sfp_Payload->channelSet.Channel;
			break;
		case FSP_CMD_CHANNEL_CONTROL:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_CHANNEL_CONTROL\r\n");
			is_v_switch_enable = s_pGPC_Sfp_Payload->channelControl.State;
			break;

		case FSP_CMD_GET_CURRENT:

			break;

		case FSP_CMD_HANDSHAKE:
			UART_Send_String(&RS232_UART, "Received FSP_CMD_HANDSHAKE\r\n");
			s_pGPP_Sfp_Payload->handshake.Cmd = FSP_CMD_HANDSHAKE;
			s_pGPP_Sfp_Payload->handshake.Check = 0xAB;
			s_GPP_FspPacket.sod = FSP_PKT_SOD;
			s_GPP_FspPacket.src_adr = fsp_my_adr;
			s_GPP_FspPacket.dst_adr = FSP_ADR_GPC;
			s_GPP_FspPacket.length = 2;
			s_GPP_FspPacket.type = FSP_PKT_TYPE_CMD_W_DATA;
			s_GPP_FspPacket.eof = FSP_PKT_EOF;
			s_GPP_FspPacket.crc16 = crc16_CCITT(FSP_CRC16_INITIAL_VALUE,
					&s_GPP_FspPacket.src_adr, s_GPP_FspPacket.length + 4);

			uint8_t encoded_frame[10] = { 0 };
			uint8_t frame_len;
			fsp_encode(&s_GPP_FspPacket, encoded_frame, &frame_len);

			UART_FSP(&GPC_UART, encoded_frame, frame_len);
			break;
		default:
			break;
		}
		break;
	case FSP_PKT_TYPE_CMD_W_DATA_ACK:

		break;

	default:

		break;

	}
	return 0;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
