/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "app.h"

#include "fsp_line_task.h"
#include "fsp.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct _fsp_line_typedef
{
                uint16_t    buffer_size;
                char*       p_buffer;

    volatile    uint16_t    write_index;
    volatile    char        RX_char;
};
typedef struct _fsp_line_typedef fsp_line_typedef;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static const char * ErrorCode[7] =
{
    "OK\n",
    "FSP_PKT_NOT_READY\n",
    "FSP_PKT_INVALID\n",
    "FSP_PKT_WRONG_ADR\n",
    "FSP_PKT_ERROR\n",
    "FSP_PKT_CRC_FAIL\n",
    "FSP_PKT_WRONG_LENGTH\n"
};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern  uart_stdio_typedef  RS232_UART;
        uart_stdio_typedef  GPC_UART;
        char                g_GPC_UART_TX_buffer[64];
        char                g_GPC_UART_RX_buffer[64];

fsp_line_typedef    FSP_line;
char                g_FSP_line_buffer[64];
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
void FSP_Line_Task_Init()
{
    UART_Init(  &GPC_UART, GPC_UART_HANDLE, GPC_UART_IRQ,
                g_GPC_UART_TX_buffer, g_GPC_UART_RX_buffer,
                sizeof(g_GPC_UART_TX_buffer), sizeof(g_GPC_UART_RX_buffer));

    FSP_line.p_buffer       = g_FSP_line_buffer;
    FSP_line.buffer_size    = 64;
    FSP_line.write_index 	= 0;

    if(FSP_line.buffer_size != 0)
    {
        memset((void *)FSP_line.p_buffer, 0, sizeof(FSP_line.p_buffer));
    }

    fsp_init(FSP_ADR_GPP);
}

/* :::::::::: CMD Line Task ::::::::::::: */
bool is_receive_SOD = false;
bool escape         = false;
void FSP_Line_Task(void*)
{
    uint8_t fsp_return, time_out;
    fsp_packet_t  fsp_pkt;

    for(time_out = 50, escape = false; (!RX_BUFFER_EMPTY(&GPC_UART)) && (time_out != 0) && (escape == false); time_out--)
    {
        FSP_line.RX_char = UART_Get_Char(&GPC_UART);
        escape = false;

        if(FSP_line.RX_char == FSP_PKT_SOD)
        {
            FSP_line.write_index = 0;
            is_receive_SOD = true;
        }
        else if((FSP_line.RX_char == FSP_PKT_EOF) && (is_receive_SOD == true))
        {
            fsp_return = frame_decode((uint8_t *)FSP_line.p_buffer, FSP_line.write_index, &fsp_pkt);
            FSP_line.write_index = 0;
            is_receive_SOD = false;

            UART_Printf(&RS232_UART, ErrorCode[fsp_return]);
            escape = true;
        }
        else
        {
            FSP_line.p_buffer[FSP_line.write_index] = FSP_line.RX_char;
            FSP_line.write_index++;

            if (FSP_line.write_index > FSP_line.buffer_size)
            {
                // SDKLFJSDFKS
                // > CMD too long!
                // > 
                UART_Send_String(&RS232_UART, "\n> CMD too long!\n> ");
                //FSP_line.write_index = FSP_line.read_index;
                FSP_line.write_index    = 0;
            }
        }
    }
}

/* :::::::::: IRQ Handler ::::::::::::: */
void GPC_UART_IRQHandler(void)
{
    if(LL_USART_IsActiveFlag_TXE(GPC_UART.handle) == true)
    {
        if(TX_BUFFER_EMPTY(&GPC_UART))
        {
            // Buffer empty, so disable interrupts
            LL_USART_DisableIT_TXE(GPC_UART.handle);
        }
        else
        {
            // There is more data in the output buffer. Send the next byte
            UART_Prime_Transmit(&GPC_UART);
        }
    }

    if(LL_USART_IsActiveFlag_RXNE(GPC_UART.handle) == true)
    {
        GPC_UART.RX_irq_char = LL_USART_ReceiveData8(GPC_UART.handle);

        // NOTE: On win 10, default PUTTY when hit enter only send back '\r',
        // while on default HERCULES when hit enter send '\r\n' in that order.
        // The code bellow is modified so that it can work on PUTTY and HERCULES.
        //if((!RX_BUFFER_FULL(&GPC_UART)) && (GPC_UART.RX_irq_char != '\n'))
        //{
            //if (GPC_UART.RX_irq_char == '\r')
            //{
                //GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] = '\n';
                //ADVANCE_RX_WRITE_INDEX(&GPC_UART);
            //}
            //else
            //{
                GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] = GPC_UART.RX_irq_char;
                ADVANCE_RX_WRITE_INDEX(&GPC_UART);
            //}
        //}
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
