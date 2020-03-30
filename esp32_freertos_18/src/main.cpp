#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" 

void vPrintString( const char *pcString );
void prvSetupHardware( void );
void vStringSendingTask( void *pvParameters );
void vStringReceivingTask( void *pvParameters );

//Declare a variable of type QueueHandle_t to hold the handle of the queue being created. */
QueueHandle_t xPointerQueue;

void prvSetupHardware(){
  Serial.begin( 9600 ); 
  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }

}

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
void vPrintString( const char *pcString ){
  taskENTER_CRITICAL( &myMutex );
  {
    Serial.println( (char*)pcString );
  }
  taskEXIT_CRITICAL( &myMutex );
}

/* A task that obtains a buffer, writes a string to the buffer, then sends the address of the 
buffer to the queue created in Listing 52. */
void vStringSendingTask( void *pvParameters )
{

  const char xMaxStringLength = 50;
  BaseType_t xStatus;
  BaseType_t xStringNumber = 0;
  char txt[xMaxStringLength]; 
  char * pcStringToSend;

  for( ;; ){
        snprintf( txt, xMaxStringLength, "String number %d\r\n", xStringNumber );
        pcStringToSend = txt;
        /* Obtain a buffer that is at least xMaxStringLength characters big. The implementation 
        of prvGetBuffer() is not shown ? it might obtain the buffer from a pool of pre-allocated 
        buffers, or just allocate the buffer dynamically. */

        /* Write a string into the buffer. */
       // snprintf( pcStringToSend, xMaxStringLength, "String number %d\r\n", xStringNumber );
        /* Increment the counter so the string is different on each iteration of this task. */
        xStringNumber++;
        /* Send the address of the buffer to the queue that was created in Listing 52. The
        address of the buffer is stored in the pcStringToSend variable.*/
        xStatus = xQueueSend( xPointerQueue,   /* The handle of the queue. */
                              &pcStringToSend, /* The address of the pointer that points to the buffer. */
                              portMAX_DELAY );
        
        if( xStatus != pdPASS )
        {
          /* We could not write to the queue because it was full - this must
          be an error as the receiving task should make space in the queue
          as soon as both sending tasks are in the Blocked state. */
          vPrintString( "Taks 1  Could not send to the queue.\r\n" );
        }
      }
    }
/*-----------------------------------------------------------*/

/* A task that receives the address of a buffer from the queue created in Listing 52, and 
written to in Listing 53. The buffer contains a string, which is printed out. */
void vStringReceivingTask( void *pvParameters )
{
  BaseType_t xStatus;
  char *pcReceivedString;
  for( ;; ) {
        /* Receive the address of a buffer. */
        xStatus = xQueueReceive(    xPointerQueue,   /* The handle of the queue. */
                                    &pcReceivedString, /* Store the buffer?s address in pcReceivedString. */
                                    portMAX_DELAY );
        if( xStatus == pdPASS )
        {
            /* The buffer holds a string, print it out. */
          vPrintString( pcReceivedString );
            /* The buffer is not required any more - release it so it can be freed, or re-used. */
            //prvReleaseBuffer( pcReceivedString );
        }
        else
        {

          vPrintString( "Task2 Could not send to the queue.\r\n" );
        }
  }
}

void setup() {
  prvSetupHardware(); 

/* Create a queue that can hold a maximum of 5 pointers, in this case character pointers. */
  xPointerQueue = xQueueCreate( 5, sizeof( char * ) );
  if( xPointerQueue != NULL ) {
  /* Create the 'handler' task, which is the task to which interrupt
  processing is deferred, and so is the task that will be synchronized
  with the interrupt.  The handler task is created with a high priority to
  ensure it runs immediately after the interrupt exits.  In this case a
  priority of 3 is chosen. */
    xTaskCreate( vStringSendingTask, "Send", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

  /* Create the task that will periodically generate a software interrupt.
  This is created with a priority below the handler task to ensure it will
  get preempted each time the handler task exits the Blocked state. */
    xTaskCreate( vStringReceivingTask, "Receive", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  }

}

void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




