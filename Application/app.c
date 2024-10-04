// APP HEADER //
#include "app.h"

//TODO: Create a system to handle hard fault or smth like that.

#define         SCHEDULER_TASK_COUNT  3

uint8_t read_uncompensated_value(uint32_t *pressure, uint32_t *temperature);
uint32_t 		g_ui32SchedulerNumTasks = SCHEDULER_TASK_COUNT;
tSchedulerTask 	g_psSchedulerTable[SCHEDULER_TASK_COUNT] =
                {
                    {
                            &H_Bridge_Task,
                            (void *) 0,
                            5,                          //call every 500us
                            0,                          //count from start
                            true                        //is active
                    },
                    {
                            &CMD_Line_Task,
                            (void *) 0,
                            5,                         //call every 1ms
                            0,                          //count from start
                            true                        //is active
                    },
					 {
					      &BMP390_Task,
					      (void *) 0,
					     500,                         //call every 1ms
					     0,                          //count from start
					     true                        //is active
					        },
                };

void App_Main(void)
{   
    // STM32F030CCT6 @ 36MHz, 
    // can run scheduler tick max @ 100us.
    SchedulerInit(10000);

    H_Bridge_Task_Init();
    CMD_Line_Task_Init();
    BMP390_init();

    while (1)
    {
        SchedulerRun();

    }
}







