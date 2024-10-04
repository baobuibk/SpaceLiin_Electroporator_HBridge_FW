/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "app.h"
#include "fsp.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct _fsp_typedef
{
                uint16_t    buffer_size;
                char*       p_buffer;

    volatile    uint16_t    write_index;
    volatile    char        RX_char;
};
struct _cmd_line_typedef
{
                uint16_t    buffer_size;
                char*       p_buffer;

    volatile    uint16_t    write_index;
    volatile    char        RX_char;
};
static const char * ErrorCode[6] =
{
    "OK\n",
    "CMDLINE_BAD_CMD\n",
    "CMDLINE_TOO_MANY_ARGS\n",
    "CMDLINE_TOO_FEW_ARGS\n",
    "CMDLINE_INVALID_ARG\n",
    "CMDLINE_INVALID_CMD\n",
};
typedef struct _cmd_line_typedef cmd_line_typedef;
typedef struct _fsp_typedef fsp_typedef;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef  GPC_UART;
extern cmd_line_typedef    CMD_line;
extern uart_stdio_typedef  RS232_UART;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
void FSP_Decode_Task_Init()
{
    ;
}

/* :::::::::: CMD Line Task ::::::::::::: */
void FSP_Decode_Task(void*)
{
    uint8_t cmd_return, time_out;

    for(time_out = 50; (!RX_BUFFER_EMPTY(&GPC_UART)) && (time_out != 0); time_out--)
    {
        CMD_line.RX_char = UART_Get_Char(&GPC_UART);

        if((CMD_line.RX_char == '\r') || (CMD_line.RX_char == '\n'))
        {
            if(CMD_line.write_index > 0)
            {
                // Add a NUL char at the end of the CMD
                CMD_line.p_buffer[CMD_line.write_index] = 0;
                CMD_line.write_index++;

                cmd_return = CmdLineProcess(CMD_line.p_buffer);
                //CMD_line.read_index = CMD_line.write_index;
                CMD_line.write_index    = 0;

                UART_Send_String(&RS232_UART, "> ");
                UART_Printf(&RS232_UART, ErrorCode[cmd_return]);
                UART_Send_String(&RS232_UART, "> ");
            }
            else
            {
                UART_Send_String(&RS232_UART, "> ");
            }
        }
        else
        {
            CMD_line.p_buffer[CMD_line.write_index] = CMD_line.RX_char;
            CMD_line.write_index++;

            if (CMD_line.write_index > CMD_line.buffer_size)
            {
                // SDKLFJSDFKS
                // > CMD too long!
                // > 
                UART_Send_String(&RS232_UART, "\n> CMD too long!\n> ");
                //CMD_line.write_index = CMD_line.read_index;
                CMD_line.write_index    = 0;
            }
        }
    }
}

/* :::::::::: IRQ Handler ::::::::::::: */
//void RS232_IRQHandler(void)
//{
//    if(LL_USART_IsActiveFlag_TXE(RS232_UART.handle) == true)
//    {
//        if(TX_BUFFER_EMPTY(&RS232_UART))
//        {
//            // Buffer empty, so disable interrupts
//            LL_USART_DisableIT_TXE(RS232_UART.handle);
//        }
//        else
//        {
//            // There is more data in the output buffer. Send the next byte
//            UART_Prime_Transmit(&RS232_UART);
//        }
//    }
//
//    if(LL_USART_IsActiveFlag_RXNE(RS232_UART.handle) == true)
//    {
//        RS232_UART.RX_irq_char = LL_USART_ReceiveData8(RS232_UART.handle);
//
//        // NOTE: On win 10, default PUTTY when hit enter only send back '\r',
//        // while on default HERCULES when hit enter send '\r\n' in that order.
//        // The code bellow is modified so that it can work on PUTTY and HERCULES.
//        if((!RX_BUFFER_FULL(&RS232_UART)) && (RS232_UART.RX_irq_char != '\n'))
//        {
//            if (RS232_UART.RX_irq_char == '\r')
//            {
//                RS232_UART.p_RX_buffer[RS232_UART.RX_write_index] = '\n';
//                ADVANCE_RX_WRITE_INDEX(&RS232_UART);
//            }
//            else
//            {
//                RS232_UART.p_RX_buffer[RS232_UART.RX_write_index] = RS232_UART.RX_irq_char;
//                ADVANCE_RX_WRITE_INDEX(&RS232_UART);
//            }
//        }
//    }
//}
//
//void GPC_UART_IRQHandler(void)
//{
//    if(LL_USART_IsActiveFlag_TXE(GPC_UART.handle) == true)
//    {
//        if(TX_BUFFER_EMPTY(&GPC_UART))
//        {
//            // Buffer empty, so disable interrupts
//            LL_USART_DisableIT_TXE(GPC_UART.handle);
//        }
//        else
//        {
//            // There is more data in the output buffer. Send the next byte
//            UART_Prime_Transmit(&GPC_UART);
//        }
//    }
//
//    if(LL_USART_IsActiveFlag_RXNE(GPC_UART.handle) == true)
//    {
//        GPC_UART.RX_irq_char = LL_USART_ReceiveData8(GPC_UART.handle);
//
//        // NOTE: On win 10, default PUTTY when hit enter only send back '\r',
//        // while on default HERCULES when hit enter send '\r\n' in that order.
//        // The code bellow is modified so that it can work on PUTTY and HERCULES.
//        if((!RX_BUFFER_FULL(&GPC_UART)) && (GPC_UART.RX_irq_char != '\n'))
//        {
//            if (GPC_UART.RX_irq_char == '\r')
//            {
//                GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] = '\n';
//                ADVANCE_RX_WRITE_INDEX(&GPC_UART);
//            }
//            else
//            {
//                GPC_UART.p_RX_buffer[GPC_UART.RX_write_index] = GPC_UART.RX_irq_char;
//                ADVANCE_RX_WRITE_INDEX(&GPC_UART);
//            }
//        }
//    }
//}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
