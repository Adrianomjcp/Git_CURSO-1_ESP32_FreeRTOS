/*
 Curso Online NodeMCU ESP32 FreeRTOS
 Autor: Fernando Simplicio
 www.microgenios.com.br

 --Este Projeto tem por objetivo criar duas threads no FreeRTOS.
 --Cada thread é responsável em enviar uma string pela UART do ESP32 de maneira concorrente.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "HardwareSerial.h"


/*
 * Protótipos de Função 
*/
void prvSetupHardware( void );
void vPrintString( const char *pcString);

void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

/*
 * Global
*/

SemaphoreHandle_t xMutex;
const TickType_t xMaxBlockTimeTicks = 1;

/*
 * Funções
*/
void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
}



void vPrintString( const char *pcString ){
  xSemaphoreTake( xMutex, portMAX_DELAY );
  {
    Serial.println( (char*)pcString );
  }
  xSemaphoreGive( xMutex );//libero o MUTEX
}



void vTask1( void *pvParameters ){
  const char *pcTaskName = "Task 1 is running";
  volatile uint32_t ul;

  for( ;; ){
    vPrintString( pcTaskName );      
    vTaskDelay( rand() % xMaxBlockTimeTicks );
  }
}

void vTask2( void *pvParameters ){
  const char *pcTaskName = "Task 2 is running";
  volatile uint32_t ul;

  for( ;; ){
    vPrintString( pcTaskName );
    vTaskDelay( rand() % xMaxBlockTimeTicks );
  }
}


void setup() {

  prvSetupHardware(); 

  xMutex = xSemaphoreCreateMutex();
  if( xMutex != NULL )
  {
    xTaskCreate( vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 2, NULL );   
    xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  }  

}

void loop() {   //TASK loop
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}