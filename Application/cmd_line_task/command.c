/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdbool.h>
#include <stdlib.h>

#include "stm32f0xx_ll_gpio.h"

#include "command.h"
#include "app.h"

#include "cmd_line_task.h"
#include "h_bridge_task.h"

#include "cmd_line.h"
#include "pwm.h"
#include "fsp.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef  RS232_UART;
extern uart_stdio_typedef  GPP_UART;

extern PWM_TypeDef H_Bridge_1_PWM;
extern PWM_TypeDef H_Bridge_2_PWM;

tCmdLineEntry g_psCmdTable[] =
{
    { "MARCO",              CMD_LINE_TEST,      "TEST" },
    { "PULSE_COUNT",        CMD_PULSE_COUNT,    "Set number of pulse" },
    { "PULSE_DELAY",        CMD_PULSE_DELAY,    "Set delay between pulse hv and lv"},
    { "PULSE_HV",           CMD_PULSE_HV,       "Set hs pulse on time and off time" },
    { "PULSE_LV",           CMD_PULSE_LV,       "Set ls pulse on time and off time" },
    { "PULSE_CONTROL",      CMD_PULSE_CONTROL,  "Start pulsing" },
    { "RELAY_SET",          CMD_RELAY_SET,      "Set up cuvette" },
    { "RELAY_CONTROL",      CMD_RELAY_CONTROL,  "Stop cuvette" },
    { "CHANNEL_SET",        CMD_CHANNEL_SET,    "Choose a cap channel"},
    { "CHANNEL_CONTROL",    CMD_CHANNEL_CONTROL,"Control the setted channel"},
	{0,0,0}
};
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int CMD_LINE_TEST(int argc, char *argv[])
{
    UART_Send_String(&RS232_UART, "> POLO\n");
    return CMDLINE_OK;
}

int CMD_PULSE_COUNT(int argc, char *argv[])
{
    if (argc < 3)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 3)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm[2];

    receive_argm[0] = atoi(argv[1]);
    receive_argm[1] = atoi(argv[2]);

    if ((receive_argm[0] > 20) || (receive_argm[1] > 20))
        return CMDLINE_INVALID_ARG;

    hv_pulse_count   = receive_argm[0];
    lv_pulse_count    = receive_argm[1];

    return CMDLINE_OK;
}

int CMD_PULSE_DELAY(int argc, char *argv[])
{
    if (argc < 2)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 2)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm;

    receive_argm = atoi(argv[1]);

    if ((receive_argm > 127) || (receive_argm < 2))
        return CMDLINE_INVALID_ARG;

    pulse_delay_ms = receive_argm;

    return CMDLINE_OK;
}

int CMD_PULSE_HV(int argc, char *argv[])
{
    if (argc < 3)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 3)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm[2];

    receive_argm[0] = atoi(argv[1]);
    receive_argm[1] = atoi(argv[2]);

    if ((receive_argm[0] > 20) || (receive_argm[0] < 1))
        return CMDLINE_INVALID_ARG;
    else if ((receive_argm[1] > 20) || (receive_argm[1] < 1))
        return CMDLINE_INVALID_ARG;

    hv_on_time_ms = receive_argm[0];
    hv_off_time_ms = receive_argm[1];

    return CMDLINE_OK;
}

int CMD_PULSE_LV(int argc, char *argv[])
{
    if (argc < 3)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 3)
        return CMDLINE_TOO_MANY_ARGS;
    
    int receive_argm[2];

    receive_argm[0] = atoi(argv[1]);
    receive_argm[1] = atoi(argv[2]);

    if ((receive_argm[0] > 500) || (receive_argm[0] < 1))
        return CMDLINE_INVALID_ARG;
    else if ((receive_argm[1] > 500) || (receive_argm[1] < 1))
        return CMDLINE_INVALID_ARG;

    lv_on_time_ms = receive_argm[0];
    lv_off_time_ms = receive_argm[1];

    return CMDLINE_OK;
}

int CMD_PULSE_CONTROL(int argc, char *argv[])
{
    if (argc < 2)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 2)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm;

    receive_argm = atoi(argv[1]);

    if ((receive_argm > 1) || (receive_argm < 0))
        return CMDLINE_INVALID_ARG;

    is_h_bridge_enable = receive_argm;

    return CMDLINE_OK;
}

int CMD_RELAY_SET(int argc, char *argv[])
{
    if (argc < 3)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 3)
        return CMDLINE_TOO_MANY_ARGS;
    
    int receive_argm[2];

    receive_argm[0] = atoi(argv[1]);
    receive_argm[1] = atoi(argv[2]);

    if (receive_argm[0] == receive_argm[1])
        return CMDLINE_INVALID_ARG;
    else if ((receive_argm[0] > 7) || (receive_argm[0] < 0))
        return CMDLINE_INVALID_ARG;
    else if ((receive_argm[1] > 7) || (receive_argm[1] < 0))
        return CMDLINE_INVALID_ARG;

    decode_hs_relay(receive_argm[0]);
    decode_ls_relay(receive_argm[1]);

    return CMDLINE_OK;
}

int CMD_RELAY_CONTROL(int argc, char *argv[])
{
    if (argc < 2)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 2)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm;

    receive_argm = atoi(argv[1]);

    if ((receive_argm > 1) || (receive_argm < 0))
        return CMDLINE_INVALID_ARG;

    if (receive_argm == 0)
    {
        LL_GPIO_ResetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
        LL_GPIO_ResetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(DECOD_HS_EN_PORT, DECOD_HS_EN_PIN);
        LL_GPIO_SetOutputPin(DECOD_LS_EN_PORT, DECOD_LS_EN_PIN);
    }

    return CMDLINE_OK;
}

int CMD_CHANNEL_SET(int argc, char *argv[])
{
    if (argc < 2)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 2)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm;

    receive_argm = atoi(argv[1]);

    if ((receive_argm > 2) || (receive_argm < 1))
        return CMDLINE_INVALID_ARG;

    Channel_Set = receive_argm;

    return CMDLINE_OK;
}

int CMD_CHANNEL_CONTROL(int argc, char *argv[])
{
    if (argc < 2)
        return CMDLINE_TOO_FEW_ARGS;
    else if (argc > 2)
        return CMDLINE_TOO_MANY_ARGS;

    int receive_argm;

    receive_argm = atoi(argv[1]);

    if ((receive_argm > 1) || (receive_argm < 0))
        return CMDLINE_INVALID_ARG;

    is_v_switch_enable = receive_argm;

    return CMDLINE_OK;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void decode_ls_relay(uint8_t cuvette_code)
{
    if ((cuvette_code & 0b001) == 0b001)
    {
        LL_GPIO_SetOutputPin(DECOD_LS0_PORT, DECOD_LS0_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(DECOD_LS0_PORT, DECOD_LS0_PIN);
    }
    
    if ((cuvette_code & 0b010) == 0b010)
    {
        LL_GPIO_SetOutputPin(DECOD_LS1_PORT, DECOD_LS1_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(DECOD_LS1_PORT, DECOD_LS1_PIN);
    }

    if ((cuvette_code & 0b100) == 0b100)
    {
        LL_GPIO_SetOutputPin(DECOD_LS2_PORT, DECOD_LS2_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(DECOD_LS2_PORT, DECOD_LS2_PIN);
    }
}

void decode_hs_relay(uint8_t cuvette_code)
{
    if ((cuvette_code & 0b001) == 0b001)
    {
        LL_GPIO_SetOutputPin(DECOD_HS0_PORT, DECOD_HS0_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(DECOD_HS0_PORT, DECOD_HS0_PIN);
    }
    
    if ((cuvette_code & 0b010) == 0b010)
    {
        LL_GPIO_SetOutputPin(DECOD_HS1_PORT, DECOD_HS1_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(DECOD_HS1_PORT, DECOD_HS1_PIN);
    }

    if ((cuvette_code & 0b100) == 0b100)
    {
        LL_GPIO_SetOutputPin(DECOD_HS2_PORT, DECOD_HS2_PIN);
    }
    else
    {
        LL_GPIO_ResetOutputPin(DECOD_HS2_PORT, DECOD_HS2_PIN);
    }
}
