#include "app.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Include~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdint.h>
#include <stdbool.h>

#include "stm32f0xx_ll_gpio.h"

#include "app.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Defines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Enum ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Struct ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Private Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* :::::::::: Calib Task State ::::::::::::: */
typedef enum
{
	CALIB_OFF_STATE,
    CALIB_START_STATE,
    CALIB_PROCESS_STATE,
} Calib_State_typedef;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//static Calib_State_typedef Calib_State = CALIB_OFF_STATE;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Prototype ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
static inline void charge_cap_procedure(uint16_t hv_set_volt, uint16_t lv_set_volt);
static inline void stop_cap_procedure();
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
extern uart_stdio_typedef  RS232_UART;
extern uart_stdio_typedef  RF_UART;

uint16_t g_HV_Calib_set = 0;
uint16_t g_LV_Calib_set = 0;

uint32_t g_HV_Measure_mv = 0;
uint32_t g_LV_Measure_mv = 0;

uint8_t calib_state_count = 0;

bool g_is_calib_running     = false;
bool g_is_measure_available = false;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Public Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* :::::::::: Discharge Task Init ::::::::::::: */
//void Impedance_Task_Init(void)
//{
	//;
//}

/* :::::::::: Discharge Task ::::::::::::: */
//void Impedance_Task(void*)
//{
    //;
//}

//void Impedance_Calculate(uint16_t hv_set_voltage, uint16_t lv_set_voltage)
//{
    //;
//}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Private Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
//static inline void charge_cap_procedure(uint16_t hv_set_volt, uint16_t lv_set_volt)
//{
    //;
//}

//static inline void stop_cap_procedure()
//{
    //;
//}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of the program ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
