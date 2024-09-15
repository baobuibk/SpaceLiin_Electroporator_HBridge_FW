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
#include "cmd_line_task.h"

// INCLUDE LIB //
#include "uart.h"

void App_Main(void);
void H_BRIDGE_TX_IRQHandler(void);
void H_BRIDGE_RX_IRQHandler(void);

#endif /* APP_H_ */
