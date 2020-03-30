#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" 
#include "freertos/event_groups.h"

void vPrintStringAndInteger( const char *pcString, uint32_t ulValue );
void prvSetupHardware( void );

void vSenderTask( void *pvParameters );
void vReceiverTask( void *pvParameters );


typedef struct xExampleStructure
{
    TickType_t xTimeStamp;
    uint32_t ulValue;
} Example_t;

QueueHandle_t xMailbox;


void prvSetupHardware(){
  Serial.begin( 9600 ); 
  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }
}

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
void vPrintStringAndInteger( const char *pcString, uint32_t ulValue ){
  taskENTER_CRITICAL( &myMutex );
  {
    char buffer [50]; 
    sprintf( buffer, "%s %lu", pcString, ulValue );
    Serial.println( (char*)buffer );
  }
  taskEXIT_CRITICAL( &myMutex );
}

void vSenderTask( void *pvParameters ){
    Example_t xData;
    BaseType_t xStringNumber = 0;
    const TickType_t xBlockTime = pdMS_TO_TICKS( 100 );
    for( ;; )
    {

          xStringNumber++;       
          /* Write the new data into the Example_t structure.*/
          xData.ulValue = xStringNumber;
          /* Use the RTOS tick count as the time stamp stored in the Example_t structure. */
          xData.xTimeStamp = xTaskGetTickCount();
          /* Send the structure to the mailbox - overwriting any data that is already in the 
          mailbox. */
          xQueueOverwrite( xMailbox, &xData );

           /* Block for 100ms. */
          vTaskDelay( xBlockTime );

    }
}

void vReceiverTask( void *pvParameters ){
    BaseType_t xStatus;
    Example_t xData;
    BaseType_t xDataUpdated;
    const TickType_t xDelay500ms = pdMS_TO_TICKS(500);

    for( ;; )
    {

          xStatus = xQueuePeek( xMailbox, &xData, xDelay500ms);
          if( xStatus == pdPASS )
          {  
             vPrintStringAndInteger( "Value = ", xData.ulValue);
             vPrintStringAndInteger( "TimeStamp = ", xData.xTimeStamp);
          }   
          
          vTaskDelay( xDelay500ms );
    }
}


void setup() {
  prvSetupHardware(); 

  xMailbox = xQueueCreate( 1, sizeof( Example_t ) );
  xTaskCreate( vSenderTask, "Sender1", configMINIMAL_STACK_SIZE+2000, NULL, 1, NULL );
  xTaskCreate( vReceiverTask, "Receiver", configMINIMAL_STACK_SIZE+2000, NULL, 1, NULL );
}

void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




