/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "app.h"

#include "cmd_line_task.h"
#include "cmd_line.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
struct _cmd_line_typedef
{
                uint16_t    buffer_size;
                char*       p_buffer;

    volatile    uint16_t    write_index;
    volatile    uint16_t    read_index;
    volatile    char        RX_char;
};
typedef struct _cmd_line_typedef cmd_line_typedef;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
uart_stdio_typedef  RS232_UART;
char                g_RS232_UART_TX_buffer[2048];
char                g_RS232_UART_RX_buffer[64];

cmd_line_typedef    CMD_line;
char                g_CMD_line_buffer[64];

static const char * ErrorCode[5] = 
{
    "OK\n",
    "CMDLINE_BAD_CMD\n",
    "CMDLINE_TOO_MANY_ARGS\n",
    "CMDLINE_TOO_FEW_ARGS\n",
    "CMDLINE_INVALID_ARG\n" 
};

const char SPLASH[][65] = 
{
{"\r\n"},
{".........................................................\r\n"},
{".........................................................\r\n"},
{"..    ____                       _     _               ..\r\n"},
{"..   / ___| _ __   __ _  ___ ___| |   (_)_ __  _ __    ..\r\n"},
{"..   \\___ \\| '_ \\ / _` |/ __/ _ \\ |   | | '_ \\| '_ \\   ..\r\n"},
{"..    ___) | |_) | (_| | (_|  __/ |___| | | | | | | |  ..\r\n"},
{"..   |____/| .__/ \\__,_|\\___\\___|_____|_|_| |_|_| |_|  ..\r\n"},
{"..         |_|    _   _ _____ _____                    ..\r\n"},
{"..               | | | | ____| ____|                   ..\r\n"},
{"..               | |_| |  _| |  _|                     ..\r\n"},
{"..               |  _  | |___| |___                    ..\r\n"},
{"..               |_| |_|_____|_____|                   ..\r\n"},
{"..            __     _____   ___   ___                 ..\r\n"},
{"..            \\ \\   / / _ \\ / _ \\ / _ \\                ..\r\n"},
{"..             \\ \\ / / | | | | | | | | |               ..\r\n"},
{"..              \\ V /| |_| | |_| | |_| |               ..\r\n"},
{"..               \\_/  \\___(_)___(_)___/                ..\r\n"},
{".........................................................\r\n"},
{".........................................................\r\n"},                                                   
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static uint8_t      is_buffer_full(volatile uint16_t *pui16Read,
                            volatile uint16_t *pui16Write, uint16_t ui16Size);

static uint8_t      is_buffer_empty(volatile uint16_t *pui16Read,
                            volatile uint16_t *pui16Write);

static uint16_t     get_buffer_count(volatile uint16_t *pui16Read,
                            volatile uint16_t *pui16Write, uint16_t ui16Size);

static uint16_t     advance_buffer_index(volatile uint16_t* pui16Index, uint16_t ui16Size);

static uint16_t     retreat_buffer_index(volatile uint16_t* pui16Index, uint16_t ui16Size);

static void         CMD_send_splash(uart_stdio_typedef* p_uart);

       int          CMD_debug_led_on(int argc, char *argv[]);

       int          CMD_debug_led_off(int argc, char *argv[]);

//*****************************************************************************
//
// Macros to determine number of free and used bytes in the receive buffer.
//
//*****************************************************************************
#define CMD_BUFFER_SIZE(p_cmd_line)          ((p_cmd_line)->buffer_size)

#define CMD_BUFFER_USED(p_cmd_line)          (get_buffer_count(&(p_cmd_line)->read_index,  \
                                                                  &(p_cmd_line)->write_index, \
                                                                  (p_cmd_line)->buffer_size))

#define CMD_BUFFER_FREE(p_cmd_line)          (CMD_BUFFER_SIZE - CMD_BUFFER_USED(p_cmd_line))

#define CMD_BUFFER_EMPTY(p_cmd_line)         (is_buffer_empty(&(p_cmd_line)->read_index,   \
                                                                  &(p_cmd_line)->write_index))

#define CMD_BUFFER_FULL(p_cmd_line)          (is_buffer_full(&(p_cmd_line)->read_index,  \
                                                                 &(p_cmd_line)->write_index, \
                                                                 (p_cmd_line)->buffer_size))

#define ADVANCE_CMD_WRITE_INDEX(p_cmd_line)  (advance_buffer_index(&(p_cmd_line)->write_index, \
                                                                      (p_cmd_line)->buffer_size))

#define ADVANCE_CMD_READ_INDEX(p_cmd_line)   (advance_buffer_index(&(p_cmd_line)->read_index, \
                                                                      (p_cmd_line)->buffer_size))

#define RETREAT_CMD_WRITE_INDEX(p_cmd_line)  (retreat_buffer_index(&(p_cmd_line)->write_index, \
                                                                      (p_cmd_line)->buffer_size))

#define RETREAT_CMD_READ_INDEX(p_cmd_line)   (retreat_buffer_index(&(p_cmd_line)->read_index, \
                                                                      (p_cmd_line)->buffer_size))


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
tCmdLineEntry g_psCmdTable[] =
{
    { "GPC_DEBUG_LED_ON", CMD_debug_led_on, "Turn on debug led" },
    { "GPC_DEBUG_LED_OFF", CMD_debug_led_off, "Turn off debug led" },
	{0,0,0}
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: CMD Line Task Init :::::::: */
void CMD_Line_Task_Init()
{

    UART_Init(  &RS232_UART, RS232_UART_HANDLE, RS232_UART_IRQ,
                g_RS232_UART_TX_buffer, g_RS232_UART_RX_buffer,
                sizeof(g_RS232_UART_TX_buffer), sizeof(g_RS232_UART_RX_buffer));
    
    CMD_line.p_buffer       = g_CMD_line_buffer;
    CMD_line.buffer_size    = 64;
    CMD_line.write_index 	= 0;
    CMD_line.read_index		= 0;

    if(CMD_line.buffer_size != 0)
    {
        memset((void *)CMD_line.p_buffer, 0, sizeof(CMD_line.p_buffer));
    }

    UART_Write(&RS232_UART, "GPP FIRMWARE V1.0.0\n", 20);
    UART_Write(&RS232_UART, "> ", 2);
    CMD_send_splash(&RS232_UART);
}

/* :::::::::: CMD Line Task ::::::::::::: */
void CMD_Line_Task(void*)
{
    uint8_t return_value;
    uint8_t time_out = 50;

    while((!RX_BUFFER_EMPTY(&RS232_UART)) && (time_out != 0))
    {
        CMD_line.RX_char = UART_Get_Char(&RS232_UART);
        UART_Write(&RS232_UART, &CMD_line.RX_char, 1);

        if((CMD_line.RX_char == '\r') || (CMD_line.RX_char == '\n'))
        {
            if(!CMD_BUFFER_EMPTY(&CMD_line))
            {
                // Add a NUL char at the end of the CMD
                CMD_line.p_buffer[CMD_line.write_index] = 0;
                ADVANCE_CMD_WRITE_INDEX(&CMD_line);

                return_value = CmdLineProcess(&CMD_line.p_buffer[CMD_line.read_index]);
                CMD_line.read_index = CMD_line.write_index;

                UART_Write(&RS232_UART, "> ", 2);
                UART_Printf(&RS232_UART, ErrorCode[return_value]);
                UART_Write(&RS232_UART, "> ", 2);
            }
            else
            {
                UART_Write(&RS232_UART, "> ", 2);
            }
        }
        else if((CMD_line.RX_char == 8) || (CMD_line.RX_char == 127))
        {
            if(!CMD_BUFFER_EMPTY(&CMD_line))
                RETREAT_CMD_WRITE_INDEX(&CMD_line);
        }
        else
        {
            CMD_line.p_buffer[CMD_line.write_index] = CMD_line.RX_char;
            ADVANCE_CMD_WRITE_INDEX(&CMD_line);

            if (CMD_BUFFER_FULL(&CMD_line))
            {
                CMD_line.read_index = CMD_line.write_index;
            }
        }
        time_out --;
    }
}

int CMD_debug_led_on(int argc, char *argv[])
{
    LL_GPIO_SetOutputPin(DEBUG_LED_PORT, DEBUG_LED_PIN);
    return CMDLINE_OK;
}

int CMD_debug_led_off(int argc, char *argv[])
{
    LL_GPIO_ResetOutputPin(DEBUG_LED_PORT, DEBUG_LED_PIN);
    return CMDLINE_OK;
}

/* :::::::::: IRQ Handler ::::::::::::: */
void RS232_IRQHandler(void)
{
    if(LL_USART_IsActiveFlag_TXE(RS232_UART.handle) == true)
    {
        if(TX_BUFFER_EMPTY(&RS232_UART))
        {
            // Buffer empty, so disable interrupts
            LL_USART_DisableIT_TXE(RS232_UART.handle);
        }
        else
        {
            // There is more data in the output buffer. Send the next byte
            UART_Prime_Transmit(&RS232_UART);
        }
    }

    if(LL_USART_IsActiveFlag_RXNE(RS232_UART.handle) == true)
    {
        RS232_UART.RX_irq_char = LL_USART_ReceiveData8(RS232_UART.handle);

        if((!RX_BUFFER_FULL(&RS232_UART)) && (RS232_UART.RX_irq_char != '\r'))
        {
            RS232_UART.p_RX_buffer[RS232_UART.RX_write_index] = RS232_UART.RX_irq_char;
            ADVANCE_RX_WRITE_INDEX(&RS232_UART);
        }
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//*****************************************************************************
//
//! Determines whether the ring buffer whose pointers and size are provided
//! is full or not.
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//! \param ui16Size is the size of the buffer in bytes.
//!
//! This function is used to determine whether or not a given ring buffer is
//! full.  The structure of the code is specifically to ensure that we do not
//! see warnings from the compiler related to the order of volatile accesses
//! being undefined.
//!
//! \return Returns \b 1 if the buffer is full or \b 0 otherwise.
//
//*****************************************************************************

static uint8_t is_buffer_full(volatile uint16_t *pui16Read,
             volatile uint16_t *pui16Write, uint16_t ui16Size)
{
    uint16_t ui16Write;
    uint16_t ui16Read;

    ui16Write = *pui16Write;
    ui16Read = *pui16Read;

    return((((ui16Write + 1) % ui16Size) == ui16Read) ? 1 : 0);
}


//*****************************************************************************
//
//! Determines whether the ring buffer whose pointers and size are provided
//! is empty or not.
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//!
//! This function is used to determine whether or not a given ring buffer is
//! empty.  The structure of the code is specifically to ensure that we do not
//! see warnings from the compiler related to the order of volatile accesses
//! being undefined.
//!
//! \return Returns \b 1 if the buffer is empty or \b 0 otherwise.
//
//*****************************************************************************

static uint8_t is_buffer_empty(volatile uint16_t *pui16Read,
              volatile uint16_t *pui16Write)
{
    uint16_t ui16Write;
    uint16_t ui16Read;

    ui16Write = *pui16Write;
    ui16Read = *pui16Read;

    return((ui16Read == ui16Write) ? 1 : 0);
}


//*****************************************************************************
//
//! Determines the number of bytes of data contained in a ring buffer.
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//! \param ui16Size is the size of the buffer in bytes.
//!
//! This function is used to determine how many bytes of data a given ring
//! buffer currently contains.  The structure of the code is specifically to
//! ensure that we do not see warnings from the compiler related to the order
//! of volatile accesses being undefined.
//!
//! \return Returns the number of bytes of data currently in the buffer.
//
//*****************************************************************************

static uint16_t get_buffer_count(volatile uint16_t *pui16Read,
               volatile uint16_t *pui16Write, uint16_t ui16Size)
{
    uint16_t ui16Write;
    uint16_t ui16Read;

    ui16Write = *pui16Write;
    ui16Read = *pui16Read;

    return((ui16Write >= ui16Read) ? (ui16Write - ui16Read) :
           (ui16Size - (ui16Read - ui16Write)));
}

//*****************************************************************************
//
//! Adding +1 to the index
//!
//! \param pui16Read points to the read index for the buffer.
//! \param pui16Write points to the write index for the buffer.
//! \param ui16Size is the size of the buffer in bytes.
//!
//! This function is use to advance the index by 1, if the index
//! already hit the uart size then it will reset back to 0.
//!
//! \return Returns the number of bytes of data currently in the buffer.
//
//*****************************************************************************

static uint16_t advance_buffer_index(volatile uint16_t* pui16Index, uint16_t ui16Size)
{
    *pui16Index = (*pui16Index + 1) % ui16Size;

    return(*pui16Index);
}

static uint16_t retreat_buffer_index(volatile uint16_t* pui16Index, uint16_t ui16Size)
{
    ((*pui16Index) == 0) ? ((*pui16Index) = ui16Size) : ((*pui16Index) -= 1);

    return(*pui16Index);
}

static void CMD_send_splash(uart_stdio_typedef* p_uart)
{
    for(uint8_t i = 0 ; i < 21 ; i++)
    {
		UART_Write(p_uart, &SPLASH[i][0], 65);
	}
	UART_Write(p_uart, "> ", 2);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
