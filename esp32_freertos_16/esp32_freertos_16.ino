#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" 


#include "Arduino.h"
#include "WiFi.h"
#include "WiFiMulti.h"
#include "HTTPClient.h"
#include <geniotHTTPS_ESP32.h>


void prvSetupHardware( void );
void vTask1( void *pvParameters );

#define CORE_0 0 
#define CORE_1 1  //OU tskNO_AFFINITY 


#define WIFISSID "FSimplicio"       
#define PASSWORD "fsimpliciokzz5"     

#define TOKEN_GENIOT  "7f33f3ce94f774494feb8f1843511509"  
#define TEMPERATURA "S00"
#define UMIDADE     "S01"
#define ALARME      "S02"


Geniot client((char*)TOKEN_GENIOT);


void prvSetupHardware(){
  Serial.begin( 9600 ); 
  
loop:
  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }

 if(!(client.wifiConnection((char*)WIFISSID, (char*)PASSWORD) ) ){
    goto loop;
  }

}



void vTask1( void *pvParameters ){

  client.vPrintString( "Task1 Init..." );
  
  for( ;; ){

//--------------------------------------------------------------------
    //Envia os valores das variáveis Temperatura e Umidade para o servidor geniot.io
    client.add((char*)TEMPERATURA, random(300), (char*)"message teste");
    client.add((char*)UMIDADE, random(300), (char*)"outra msg");
    //client.add((char*)UMIDADE, random(300));

    if(client.send()){
      client.vPrintString("-----------------------------------------------");
      client.vPrintString((char*)"Enviado com sucesso!"); 
    } 
//--------------------------------------------------------------------      
    vTaskDelay( 2000 / portTICK_PERIOD_MS );
    if(client.read((char*)TEMPERATURA))
    {
      client.vPrintString("-----------------------------------------------");
      client.vPrintStringAndFloat((char*)"Temperatura: ", client.geniotValue); //float
      client.vPrintTwoStrings((char*)"Mensagem: ", client.geniotText);  //matriz char
    }

    vTaskDelay( 2000 / portTICK_PERIOD_MS );
    if(client.read((char*)UMIDADE))
    {
      client.vPrintString("-----------------------------------------------");
      client.vPrintStringAndFloat((char*)"Umidade: ", client.geniotValue);
      client.vPrintTwoStrings((char*)"Mensagem: ", client.geniotText);
    }

    vTaskDelay( 2000 / portTICK_PERIOD_MS );
    if(client.read((char*)ALARME))
    {
      client.vPrintString("-----------------------------------------------");
      client.vPrintStringAndFloat((char*)"Alarme: ", client.geniotValue);
    }

//--------------------------------------------------------------------
    vTaskDelay( 5000 / portTICK_PERIOD_MS );
  }
}


void setup() {
  prvSetupHardware(); 
  xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE+20000, NULL, 2, NULL, CORE_0 );   
}

void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




