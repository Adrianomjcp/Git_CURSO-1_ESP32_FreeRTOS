/*
  --Este Projeto tem por objetivo criar duas threads no FreeRTOS.
 --Cada thread é responsável em enviar uma string pela UART do ESP32 de maneira concorrente.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
#define CORE_0 0 
#define CORE_1 1
//OU tskNO_AFFINITY   <- ver aula em 10:00

/*
 * Funções
*/
void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
}

void vPrintString( const char *pcString ){

  taskENTER_CRITICAL( &myMutex ); //ver aula em 08:00 min
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
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
  }
}


void setup() {
  prvSetupHardware(); 
  
  //essa função é exclusiva do ESP, é linguagem nativa da bagaça (é o que muda p primeiro programa _00)

  xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 1, NULL, CORE_0 );   
  xTaskCreatePinnedToCore( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 1, NULL, CORE_1 );
}




void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}





