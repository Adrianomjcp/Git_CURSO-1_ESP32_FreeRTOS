/*
 Curso Online NodeMCU ESP32 FreeRTOS
 Autor: Fernando Simplicio
 www.microgenios.com.br

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "HardwareSerial.h"


#include <Wire.h>
#include "SSD1306.h" 
SSD1306  display(0x3c, 5, 4);

/*
 * Protótipos de Função 
*/
void prvSetupHardware( void );
void vPrintString( const char *pcString);

void vTask1( void *pvParameters );


/*
 * Global
*/
SemaphoreHandle_t xMutex;
SemaphoreHandle_t xMutex_oled;

/*
 * Funções
*/
void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
  Serial.println(); 

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
}


void vPrintString( const char *pcString ){
  xSemaphoreTake( xMutex, portMAX_DELAY );
  {
    Serial.println( (char*)pcString );
  }
  xSemaphoreGive( xMutex );
}

void vTask1( void *pvParameters ){
  volatile uint32_t ul;
  char *pcTaskName;
  pcTaskName = ( char * ) pvParameters;  
  vPrintString( pcTaskName );  
  
  for( ;; ){

    xSemaphoreTake( xMutex_oled, portMAX_DELAY );
      display.clear();    
      display.drawString(0, 0, String(millis()));
      display.display();
    xSemaphoreGive( xMutex_oled );
      
    vTaskDelay( 500 / portTICK_PERIOD_MS );
  }
}



void setup() {

  prvSetupHardware(); 

  xMutex = xSemaphoreCreateMutex();
  xMutex_oled = xSemaphoreCreateMutex();
  if( xMutex != NULL && xMutex_oled != NULL )
  {
    //obs: as duas tasks apontas para a mesma função, elas concorrem entre si
    xTaskCreate( vTask1, "Task 1", configMINIMAL_STACK_SIZE, (char*)"Task1 Inicializou", 2, NULL );   
    xTaskCreate( vTask1, "Task 2", configMINIMAL_STACK_SIZE, (char*)"Task2 Inicializou", 2, NULL );
  }  

}

void loop() {   //TASK loop
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}