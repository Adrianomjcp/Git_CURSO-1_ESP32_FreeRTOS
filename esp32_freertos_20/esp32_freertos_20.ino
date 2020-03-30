#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" 
#include "freertos/event_groups.h"

void vPrintString( const char *pcString );
void prvSetupHardware( void );

/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by the first task. */ //0x01
#define mainSECOND_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by the second task. */ //0x02

const EventBits_t uxAllSyncBits = ( mainFIRST_TASK_BIT | mainSECOND_TASK_BIT ); //0x03


static void vTask1( void * pvParameters );
static void vTask2( void * pvParameters );
static void vTask3( void * pvParameters );

/* Declare the event group used to synchronize the three tasks. */
EventGroupHandle_t xEventGroup;

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

/*-----------------------------------------------------------*/

static void vTask1( void * pvParameters )
{
    for(;;)
    {
        xEventGroupSetBits( xEventGroup, mainFIRST_TASK_BIT );
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
}

static void vTask2( void * pvParameters )
{
    for(;;)
    {
         xEventGroupSetBits( xEventGroup, mainSECOND_TASK_BIT );
         vTaskDelay(250/portTICK_PERIOD_MS);
    }
}

static void vTask3( void * pvParameters )
{
    const TickType_t xDelay2000ms = pdMS_TO_TICKS( 2000UL );
    BaseType_t  result;
    for(;;)
    {
        result = xEventGroupWaitBits(   xEventGroup,
                                        uxAllSyncBits, //0x03
                                        pdTRUE,
                                        pdTRUE, 
                                        xDelay2000ms );
        
        if(result == uxAllSyncBits)
        {
            vPrintString( (char*) "Tasks Sincronizadas" );
            
        } else {
            if(!(result & mainFIRST_TASK_BIT ))
            {
                 vPrintString( (char*) "mainFIRST_TASK nao sincronizada" );
            }
            if(!(result & mainSECOND_TASK_BIT))
            {
                vPrintString( (char*) "mainSECOND_TASK nao sincronizada" );
            }
        }
        
    }
}


void setup() {
  prvSetupHardware(); 

/* Before an event group can be used it must first be created. */
  xEventGroup = xEventGroupCreate();

  /* Create three instances of the task.  Each task is given a different name,
  which is later printed out to give a visual indication of which task is
  executing.  The event bit to use when the task reaches its synchronization
  point is passed into the task using the task parameter. */
  xTaskCreate( vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  xTaskCreate( vTask3, "Task 3", configMINIMAL_STACK_SIZE, NULL, 2, NULL );


}

void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




