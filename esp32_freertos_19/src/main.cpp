#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" 
#include "freertos/event_groups.h"

void vPrintTwoStrings(const char *pcString1, const char *pcString2);
void prvSetupHardware( void );

/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by the first task. */ //0x01
#define mainSECOND_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by the second task. *///0x02
#define mainTHIRD_TASK_BIT  ( 1UL << 2UL ) /* Event bit 2, which is set by the third task. */ //0x03

/* Three instances of this task are created. */
static void vSyncingTask( void *pvParameters );

/* Use by the pseudo random number generator. */
static uint32_t ulNextRand;

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
void vPrintTwoStrings(const char *pcString1, const char *pcString2){
  taskENTER_CRITICAL( &myMutex  );
  {
    char buffer [50]; 
    sprintf(buffer, "%s %s", pcString1, pcString2);
    Serial.println( (char*)buffer );
  }
  taskEXIT_CRITICAL( &myMutex );    
}

/*-----------------------------------------------------------*/

static void vSyncingTask( void *pvParameters )
{
const EventBits_t uxAllSyncBits = ( mainFIRST_TASK_BIT | mainSECOND_TASK_BIT | mainTHIRD_TASK_BIT );
const TickType_t xMaxDelay = pdMS_TO_TICKS( 4000UL );
const TickType_t xMinDelay = pdMS_TO_TICKS( 200UL );
TickType_t xDelayTime;
EventBits_t uxThisTasksSyncBit;

  /* Three instances of this task are created - each task uses a different
  event bit in the synchronization.  The event bit to use by this task
  instance is passed into the task using the task's parameter.  Store it in
  the uxThisTasksSyncBit variable. */
  uxThisTasksSyncBit = ( EventBits_t ) pvParameters;

  for( ;; )
  {
    /* Simulate this task taking some time to perform an action by delaying
    for a pseudo random time.  This prevents all three instances of this
    task from reaching the synchronization point at the same time, and
    allows the example's behavior to be observed more easily. */
    xDelayTime = ( rand() % xMaxDelay ) + xMinDelay;
    vTaskDelay( xDelayTime );

    /* Print out a message to show this task has reached its synchronization
    point.  pcTaskGetTaskName() is an API function that returns the name
    assigned to the task when the task was created. */
    vPrintTwoStrings( pcTaskGetTaskName( NULL ), "reached sync point" );

    /* Wait for all the tasks to have reached their respective
    synchronization points. */
    xEventGroupSync( /* The event group used to synchronize. */
             xEventGroup,

             /* The bit set by this task to indicate it has reached
             the synchronization point. */
             uxThisTasksSyncBit,

             /* The bits to wait for, one bit for each task taking
             part in the synchronization. */
             uxAllSyncBits,

             /* Wait indefinitely for all three tasks to reach the
             synchronization point. */
             portMAX_DELAY );

    /* Print out a message to show this task has passed its synchronization
    point.  As an indefinite delay was used the following line will only be
    reached after all the tasks reached their respective synchronization
    points. */
    vPrintTwoStrings( pcTaskGetTaskName( NULL ), "exited sync point" );
  }
}

void setup() {
  prvSetupHardware(); 

  /* The tasks created in this example block for a random time.  The block
  time is generated using rand() - seed the random number generator. */
  //prvSRand( ( uint32_t ) time( NULL ) );

  /* Before an event group can be used it must first be created. */
  xEventGroup = xEventGroupCreate();

  /* Create three instances of the task.  Each task is given a different name,
  which is later printed out to give a visual indication of which task is
  executing.  The event bit to use when the task reaches its synchronization
  point is passed into the task using the task parameter. */
  xTaskCreate( vSyncingTask, "Task 1", configMINIMAL_STACK_SIZE, ( void * ) mainFIRST_TASK_BIT, 1, NULL );
  xTaskCreate( vSyncingTask, "Task 2", configMINIMAL_STACK_SIZE, ( void * ) mainSECOND_TASK_BIT, 1, NULL );
  xTaskCreate( vSyncingTask, "Task 3", configMINIMAL_STACK_SIZE, ( void * ) mainTHIRD_TASK_BIT, 1, NULL );

}

void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




