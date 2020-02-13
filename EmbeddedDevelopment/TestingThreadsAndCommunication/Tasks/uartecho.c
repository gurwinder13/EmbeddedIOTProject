/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "Board.h"
#include "taskDefinitions.h"
#include <DataStructures/llMessage.h>
#include <DataStructures/circularBuffer.h>
#include <mqueue.h>


void uartReadCallback();


char BufferTotal[MAX_LENGTH];
mqd_t tQm = NULL;
/*
 *  ======== mainThread ========
 */
void *uartTask(void *arg0)
{
    char        input;
    const char  echoPrompt[] = "Echoing characters:\r\n";
    UART_Handle uart;
    UART_Params uartParams;

    /* Call driver init functions */
    UART_init();

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    //uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readCallback = &uartReadCallback;
    uartParams.readMode = UART_MODE_CALLBACK;

    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Initialize the receiving queue to get data from the Rx */

    mqd_t rQm = NULL;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = MAX_LENGTH;
    attr.mq_curmsgs = 0;
    rQm = mq_open(receiveQueue, O_CREAT | O_RDONLY, 0644, &attr);

    char messageReceived[MAX_LENGTH];
    ssize_t bytes_read;
    /* Loop forever echoing */

    while (1)
    {
        bytes_read = mq_receive(rQm, (char *)messageReceived, MAX_LENGTH, NULL);
        if(bytes_read)
        {
            UART_write( uart,&(messageReceived) ,sizeof(messageReceived));
        }
        UART_read(uart, &input, 1);

        usleep(50);
    }
}
//This callback is called once the read number is completed
int i = 0;
uint8_t data;
void uartReadCallback(UART_Handle handle, void *rxBuf, size_t size)
{
    char *Buffer = (char*)rxBuf;

    //Creates the instance if it does not exist
    if(tQm==NULL)
    {
       tQm = mq_open(sendQueue, O_WRONLY);
    }

    if(i<MAX_LENGTH)
    {
        BufferTotal[i] = *Buffer;
        i++;
    }
    else
    {
        UART_write(handle, &(BufferTotal), sizeof(BufferTotal));
        mq_send(tQm, (char *)&BufferTotal, MAX_LENGTH, 0);
        i=0;
    }
    if(*(Buffer)==13)
    {
        UART_write(handle, &(BufferTotal), sizeof(BufferTotal));
        mq_send(tQm, (char *)&BufferTotal, MAX_LENGTH, 0);
        i=0;
    }
}
