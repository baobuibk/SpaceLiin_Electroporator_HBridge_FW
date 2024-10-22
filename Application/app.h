#ifndef APP_H_
#define APP_H_

#include <stdbool.h>

#include "stm32f030xc.h"

// SYSTEM DRIVER //
#include "board.h"

// USER DRIVER //
#include "scheduler.h"

// INCLUDE TASK //
#include "adc_task.h"
#include "h_bridge_task.h"
#include "v_switch_task.h"
#include "cmd_line_task.h"
#include "fsp_line_task.h"
#include "impedance_task.h"
#include "BMP390.h"
#include "i2c/i2c.h"
// INCLUDE DRIVER //
#include "h_bridge_driver.h"

// INCLUDE LIB //
#include "uart.h"

void App_Main(void);

#endif /* APP_H_ */
