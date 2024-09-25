/*
 * fsp.c
 *
 *  Created on: May 25, 2024
 *      Author: CAO HIEU
 */

#include "fsp.h"
#include "crc.h"

#include "board.h"
#include "h_bridge_task.h"
#include "stm32f0xx_ll_gpio.h"

#include <string.h>
#include <stdio.h>

static void decode_ls_cuvette(uint8_t cuvette_code);
static void decode_hs_cuvette(uint8_t cuvette_code);

uint8_t fsp_my_adr;

volatile uint8_t fsp_decode_pos = 0;

void fsp_init(uint8_t module_adr)
{
    fsp_my_adr = module_adr;

    fsp_decode_pos = 0;
}

void fsp_reset(void)
{
    fsp_decode_pos = 0;
}

void fsp_gen_data_pkt(uint8_t *data, uint8_t data_len, uint8_t dst_adr, uint8_t ack, fsp_packet_t *fsp)
{
    if (ack == FSP_PKT_WITH_ACK)
    {
        fsp_gen_pkt((void*)0, data, data_len, dst_adr, FSP_PKT_TYPE_DATA_WITH_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt((void*)0, data, data_len, dst_adr, FSP_PKT_TYPE_DATA, fsp);
    }
}

void fsp_gen_cmd_pkt(uint8_t cmd, uint8_t dst_adr, uint8_t ack, fsp_packet_t *fsp)
{
    if (ack == FSP_PKT_WITH_ACK)
    {
        fsp_gen_pkt(&cmd,(void*)0, 0, dst_adr, FSP_PKT_TYPE_CMD_WITH_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt(&cmd,(void*)0,  0, dst_adr, FSP_PKT_TYPE_CMD, fsp);
    }
}

void fsp_gen_cmd_w_data_pkt(uint8_t cmd, uint8_t *data, uint8_t data_len, uint8_t dst_adr, uint8_t ack, fsp_packet_t *fsp)
{
    if (ack == FSP_PKT_WITH_ACK)
    {
        fsp_gen_pkt(&cmd, data, data_len, dst_adr, FSP_PKT_TYPE_CMD_W_DATA_ACK, fsp);
    }
    else
    {
        fsp_gen_pkt(&cmd, data, data_len, dst_adr, FSP_PKT_TYPE_CMD_W_DATA, fsp);
    }
}


void fsp_gen_ack_pkt(uint8_t dst_adr, fsp_packet_t *fsp)
{
    fsp_gen_pkt((void*)0, (void*)0, 0, dst_adr, FSP_PKT_TYPE_ACK, fsp);
}

void fsp_gen_nack_pkt(uint8_t dst_adr, fsp_packet_t *fsp)
{
    fsp_gen_pkt((void*)0, (void*)0, 0, dst_adr, FSP_PKT_TYPE_NACK, fsp);
}


void fsp_gen_pkt(uint8_t *cmd, uint8_t *payload, uint8_t payload_len, uint8_t dst_adr, uint8_t type, fsp_packet_t *fsp)
{
    fsp->sod        = FSP_PKT_SOD;
    fsp->src_adr    = fsp_my_adr;
    fsp->dst_adr    = dst_adr;
    fsp->length     = payload_len;
    fsp->type       = type;

    uint8_t i = 0;
    uint8_t j = 0;

    // Copy cmd payload
    if (cmd != NULL) {
        fsp->length++; // length + byte cmd
        fsp->payload[j++] = *cmd;
    }

    // Copy payload fsp->payload
    for(i=0; i<payload_len; i++)
    {
        fsp->payload[j++] = payload[i];
    }

    fsp->crc16 = crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &fsp->src_adr, fsp->length + 4);

}


void frame_encode(fsp_packet_t *fsp, uint8_t *frame, uint8_t *frame_len)
{
    //frame
    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t encoded_length = 0;

    encoded_frame[encoded_length++] = fsp->sod;
    encoded_frame[encoded_length++] = fsp->src_adr;
    encoded_frame[encoded_length++] = fsp->dst_adr;
    encoded_frame[encoded_length++] = fsp->length;
    encoded_frame[encoded_length++] = fsp->type;

    for(int i=0; i<fsp->length; i++)
    {
        if (fsp->payload[i] == FSP_PKT_SOD) {
            encoded_frame[encoded_length++] = FSP_PKT_ESC;
            encoded_frame[encoded_length++] = FSP_PKT_TSOD;
        } else if (fsp->payload[i] == FSP_PKT_EOF) {
            encoded_frame[encoded_length++] = FSP_PKT_ESC;
            encoded_frame[encoded_length++] = FSP_PKT_TEOF;
        } else if (fsp->payload[i] == FSP_PKT_ESC) {
            encoded_frame[encoded_length++] = FSP_PKT_ESC;
            encoded_frame[encoded_length++] = FSP_PKT_TESC;
        } else {
            encoded_frame[encoded_length++] = fsp->payload[i];
        }
    }

    // CRC16
    uint8_t crc_msb = (uint8_t)(fsp->crc16 >> 8);
    uint8_t crc_lsb = (uint8_t)(fsp->crc16 & 0xFF);

    if (crc_msb == FSP_PKT_SOD) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TSOD;
    } else if (crc_msb == FSP_PKT_EOF) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TEOF;
    } else if (crc_msb == FSP_PKT_ESC) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TESC;
    } else {
        encoded_frame[encoded_length++] = crc_msb;
    }

    if (crc_lsb == FSP_PKT_SOD) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TSOD;
    } else if (crc_lsb == FSP_PKT_EOF) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TEOF;
    } else if (crc_lsb == FSP_PKT_ESC) {
        encoded_frame[encoded_length++] = FSP_PKT_ESC;
        encoded_frame[encoded_length++] = FSP_PKT_TESC;
    } else {
        encoded_frame[encoded_length++] = crc_lsb;
    }

    encoded_frame[encoded_length++] = FSP_PKT_EOF;

    memcpy(frame, encoded_frame, encoded_length);
    *frame_len = encoded_length;
}



void fsp_encode(fsp_packet_t *fsp, uint8_t *pkt, uint8_t *pkt_len)
{
    uint8_t i = 0;

    pkt[i++] = fsp->sod;
    pkt[i++] = fsp->src_adr;
    pkt[i++] = fsp->dst_adr;
    pkt[i++] = fsp->length;
    pkt[i++] = fsp->type;

    uint8_t j = 0;
    for(j=0; j<fsp->length; j++)
    {
        pkt[i++] = fsp->payload[j];
    }

    pkt[i++] = (uint8_t)(fsp->crc16 >> 8);
    pkt[i++] = (uint8_t)(fsp->crc16);

    *pkt_len = i;
}

int frame_decode(uint8_t *buffer, uint8_t length, fsp_packet_t *pkt){

    fsp_packet_t fsp_pkt;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t escape = 0;
    uint8_t decoded_payload[FSP_PAYLOAD_MAX_LENGTH];


	if (length < FSP_PKT_MIN_LENGTH - 2) {
	        return FSP_PKT_INVALID;
	}


	while (i < length){
        uint8_t byte = buffer[i++];
        if (escape) {
            if (byte == FSP_PKT_TSOD) {
                decoded_payload[j++] = FSP_PKT_SOD;
            } else if (byte == FSP_PKT_TEOF) {
                decoded_payload[j++] = FSP_PKT_EOF;
            } else if (byte == FSP_PKT_TESC) {
                decoded_payload[j++] = FSP_PKT_ESC;
            } else {
            	return FSP_PKT_INVALID;
            }
            escape = 0;
        } else if (byte == FSP_PKT_ESC) {
        	escape = 1;
        } else {
            decoded_payload[j++] = byte;
        }
	}

    i = 0;
    fsp_pkt.src_adr = decoded_payload[i++];
    fsp_pkt.dst_adr = decoded_payload[i++];
    fsp_pkt.length = decoded_payload[i++];
    fsp_pkt.type = decoded_payload[i++];

    if (fsp_pkt.length > FSP_PAYLOAD_MAX_LENGTH || fsp_pkt.length != j - FSP_PKT_HEADER_LENGTH  - FSP_PKT_CRC_LENGTH) {
        return FSP_PKT_WRONG_LENGTH;
    }

    memcpy(fsp_pkt.payload, &decoded_payload[i], fsp_pkt.length);
    i += fsp_pkt.length;
    //CRC
    uint16_t crc_received = (uint16_t)(decoded_payload[i++] << 8);
    crc_received |= (uint16_t)(decoded_payload[i++]);


    // CAL CRC
    uint16_t crc_calculated = crc16_CCITT(FSP_CRC16_INITIAL_VALUE, &fsp_pkt.src_adr, fsp_pkt.length + 4);


    // CHECK CRC
    if (crc_received != crc_calculated) {
        return FSP_PKT_CRC_FAIL;
    }

    // Address
    if (fsp_pkt.dst_adr != fsp_my_adr) {
        return FSP_PKT_WRONG_ADR;
    }

    *pkt = fsp_pkt;

    frame_processing(&fsp_pkt);
    return 0;
}

//char pos_str2[10];

int frame_processing(fsp_packet_t *fsp_pkt){
	switch (fsp_pkt->type)
	{
		case FSP_PKT_TYPE_DATA:

			// Uart_sendstring(USART1, "DATA: ");
			// sprintf(pos_str2, "%d", fsp_pkt->payload[0]);
			// Uart_sendstring(USART1, pos_str2);
			// sprintf(pos_str2, "%d", fsp_pkt->payload[1]);
			// Uart_sendstring(USART1, pos_str2);
			// sprintf(pos_str2, "%d", fsp_pkt->payload[2]);
			// Uart_sendstring(USART1, pos_str2);

			break;
		case FSP_PKT_TYPE_DATA_WITH_ACK:

			// Uart_sendstring(USART1, "DATA ACK:");
			// sprintf(pos_str2, "%d", fsp_pkt->payload[0]);
			// Uart_sendstring(USART1, pos_str2);
			// sprintf(pos_str2, "%d", fsp_pkt->payload[1]);
			// Uart_sendstring(USART1, pos_str2);
			// sprintf(pos_str2, "%d", fsp_pkt->payload[2]);
			// Uart_sendstring(USART1, pos_str2);

			break;
		case FSP_PKT_TYPE_CMD:

			// Uart_sendstring(USART1, "CMD: ");
			// sprintf(pos_str2, "%d", fsp_pkt->payload[0]);
			// Uart_sendstring(USART1, pos_str2);

			break;
		case FSP_PKT_TYPE_CMD_WITH_ACK:

			// Uart_sendstring(USART1, "CMD ACK:");
			// sprintf(pos_str2, "%d", fsp_pkt->payload[0]);
			// Uart_sendstring(USART1, pos_str2);

			break;
		case FSP_PKT_TYPE_ACK:

    		// Uart_sendstring(USART1, "ACK: ");

			break;
		case FSP_PKT_TYPE_NACK:

    		// Uart_sendstring(USART1, "NACK: ");

			break;
		case FSP_PKT_TYPE_CMD_W_DATA:
            switch (fsp_pkt->payload[0])
            {
            case FSP_CMD_GPC_PULSE_COUNT:
                high_side_set_pulse_count   = fsp_pkt->payload[1];
                low_side_set_pulse_count    = fsp_pkt->payload[2];
                break;
            case FSP_CMD_GPC_PULSE_HS_DURATION:
                hs_on_time_ms   = fsp_pkt->payload[1];
                hs_off_time_ms  = fsp_pkt->payload[2];
                break;
            case FSP_CMD_GPC_PULSE_LS_DURATION:
                ls_on_time_ms   = fsp_pkt->payload[1];
                ls_off_time_ms  = fsp_pkt->payload[2];
                break;
            case FSP_CMD_GPC_PULSE_CONTROL:
                is_h_bridge_enable = fsp_pkt->payload[1];
                break;
            case FSP_CMD_GPC_CUVETTE_ELECTRODE:
                decode_hs_cuvette(fsp_pkt->payload[1]);
                decode_ls_cuvette(fsp_pkt->payload[2]);
                break;
            case FSP_CMD_GPC_CUVETTE_CONTROL:
                if (fsp_pkt->payload[1] == 0)
                {
                    LL_GPIO_ResetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
                    LL_GPIO_ResetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
                }
                else
                {
                    LL_GPIO_SetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
                    LL_GPIO_SetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
                }
                break;

            default:
                break;
            }
			break;
		case FSP_PKT_TYPE_CMD_W_DATA_ACK:

   //  		Uart_sendstring(USART1, "DATA CMD ACK: ");
			// sprintf(pos_str2, "%d", fsp_pkt->payload[0]);
			// Uart_sendstring(USART1, pos_str2);
			// sprintf(pos_str2, "%d", fsp_pkt->payload[1]);
			// Uart_sendstring(USART1, pos_str2);
			// sprintf(pos_str2, "%d", fsp_pkt->payload[2]);
			// Uart_sendstring(USART1, pos_str2);

			break;

		default:

			// Uart_sendstring(USART1, "DEFAULT");

			break;


	}
	return 0;
}

static void decode_ls_cuvette(uint8_t cuvette_code)
{
    if (cuvette_code & 0b001)
    {
        LL_GPIO_ResetOutputPin(DECOD_LS0_PORT, DECOD_LS0_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_LS0_PORT, DECOD_LS0_PIN);
    }
    
    if (cuvette_code & 0b010)
    {
        LL_GPIO_ResetOutputPin(DECOD_LS1_PORT, DECOD_LS1_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_LS1_PORT, DECOD_LS1_PIN);
    }

    if (cuvette_code & 0b100)
    {
        LL_GPIO_ResetOutputPin(DECOD_LS2_PORT, DECOD_LS2_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_LS2_PORT, DECOD_LS2_PIN);
    }
}

static void decode_hs_cuvette(uint8_t cuvette_code)
{
    if (cuvette_code & 0b001)
    {
        LL_GPIO_ResetOutputPin(DECOD_HS0_PORT, DECOD_HS0_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_HS0_PORT, DECOD_HS0_PIN);
    }
    
    if (cuvette_code & 0b010)
    {
        LL_GPIO_ResetOutputPin(DECOD_HS1_PORT, DECOD_HS1_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_HS1_PORT, DECOD_HS1_PIN);
    }

    if (cuvette_code & 0b100)
    {
        LL_GPIO_ResetOutputPin(DECOD_HS2_PORT, DECOD_HS2_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_HS2_PORT, DECOD_HS2_PIN);
    }
}