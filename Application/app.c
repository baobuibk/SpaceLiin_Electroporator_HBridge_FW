// APP HEADER //
#include "app.h"

//TODO: Create a system to handle hard fault or smth like that.

#define         SCHEDULER_TASK_COUNT  3
uint32_t 		g_ui32SchedulerNumTasks = SCHEDULER_TASK_COUNT;
tSchedulerTask 	g_psSchedulerTable[SCHEDULER_TASK_COUNT] =
                {
                    {
                            &ADC_Task,
                            (void *) 0,
                            62,                         //call every 125us
                            0,			                //count from start
                            false		                //is active
                    },
                    {
                            &PID_Task,
                            (void *) 0,
                            125,                        //call every 1ms
                            0,			                //count from start
                            false		                //is active
                    },
                    {
                            &CMD_Line_Task,
                            (void *) 0,
                            125,                        //call every 1ms
                            0,                          //count from start
                            true                        //is active
                    },
                };

void App_Main(void)
{   
    // STM32F030CCT6 @ 36MHz, 
    // can run scheduler tick max @ 4us.
    SchedulerInit(250000);

    //ADC_Task_Init(LL_ADC_SAMPLINGTIME_7CYCLES_5);
    //PID_Task_Init();
    CMD_Line_Task_Init();

    while (1)
    {
        SchedulerRun();
    }
}