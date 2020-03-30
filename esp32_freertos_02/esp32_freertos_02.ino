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
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
#define CORE_0 0 
#define CORE_1 1
//OU tskNO_AFFINITY 

#define mainDELAY_LOOP_COUNT    ( 0xffffff )

/*
 * Funções
*/
void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
}

void vPrintString( const char *pcString ){
  taskENTER_CRITICAL( &myMutex );
  {
    Serial.println( (char*)pcString );
  }
  taskEXIT_CRITICAL( &myMutex );
}

void vTask1( void *pvParameters ){
  const char *pcTaskName = "Task 1 is running\r\n";
  volatile uint32_t ul;

  for( ;; ){
    vPrintString( pcTaskName );      
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
  }
}

void vTask2( void *pvParameters ){
  const char *pcTaskName = "Task 2 is running\r\n";
  volatile uint32_t ul;

  for( ;; ){
    vPrintString( pcTaskName );
    
    //você deverá tirar esse for e colocar o vTaskDelay no lugar. 
    //Esse programa mostra o impacto em deixar a task "travada" no ESP32. Estamos presos nesta task2.
    for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ ){

    }
  }
}


void setup() {
  prvSetupHardware(); 
   
  xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 2, NULL, CORE_0 );   
  xTaskCreatePinnedToCore( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 3, NULL, CORE_0 );
}

void loop() {   //TASK loop
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}





