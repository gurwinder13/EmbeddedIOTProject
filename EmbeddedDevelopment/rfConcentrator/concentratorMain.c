
/*
 *  ======== rfEasyLinkEchoRx.c ========
 */
/* Standard C Libraries */
#include <stdlib.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>

/* Board Header files */
#include "Board.h"

#include <Drivers/startUart.h>

#include <Task/taskDefinition.h>

/* Undefine to not use async mode */
#define RFEASYLINKECHO_ASYNC

#define RFEASYLINKECHO_TASK_STACK_SIZE    1024
#define RFEASYLINKECHO_TASK_PRIORITY      2


static Task_Params radioTaskParams;
static Task_Handle radioTaskHandle;
static uint8_t echoTaskStack[RFEASYLINKECHO_TASK_STACK_SIZE];
//
///* Pin driver handle */
static PIN_Handle pinHandle;
static PIN_State pinState;
static UART_Handle uart = NULL;

//
///*
// * Application LED pin configuration table:
// *   - All LEDs board LEDs are off.
// */
PIN_Config pinTable[] = {
    Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_PIN_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call driver init functions. */
    Board_initGeneral();

    /* Open LED pins */
    pinHandle = PIN_open(&pinState, pinTable);
    Assert_isTrue(pinHandle != NULL, NULL);

    /* Clear LED pins */
    PIN_setOutputValue(pinHandle, Board_PIN_LED1, 0);
    PIN_setOutputValue(pinHandle, Board_PIN_LED2, 0);

    startUART(&uart);

    Task_Params_init(&radioTaskParams);
    radioTaskParams.stackSize = RFEASYLINKECHO_TASK_STACK_SIZE;
    radioTaskParams.priority = RFEASYLINKECHO_TASK_PRIORITY;
    radioTaskParams.stack = &echoTaskStack;
    radioTaskParams.arg0 = (UInt)1000000;
    radioTaskParams.arg1 = (UArg)uart;

    radioTaskHandle = Task_create((Task_FuncPtr)radioTask,&radioTaskParams,NULL);
    if(radioTaskHandle == NULL)
    {
        while(1);
    }

    /* Start BIOS */
    BIOS_start();

    return (0);
}