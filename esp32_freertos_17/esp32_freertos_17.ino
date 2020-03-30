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

#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

#include <geniotHTTPS_ESP32.h>


void prvSetupHardware( void );
void vTask1( void *pvParameters );
#define V_REF 1100  // ADC reference voltage

#define CORE_0 0 
#define CORE_1 1  //OU tskNO_AFFINITY 

QueueHandle_t xQueue_adc;

#define WIFISSID "FSimplicio"       
#define PASSWORD "fsimpliciokzz5"     

#define TOKEN_GENIOT  "7f33f3ce94f774494feb8f1843511509"  
#define TEMPERATURA "S00"
#define UMIDADE     "S01"
#define ALARME      "S02"


Geniot client((char*)TOKEN_GENIOT);

const byte relay = 25;

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

  pinMode(relay, OUTPUT);

}



void vTask1( void *pvParameters ){

  BaseType_t xStatus;
  int adc_result; 
  client.vPrintString( "Task1 Init..." );
  
  for( ;; ){

//--------------------------------------------------------------------
    xStatus = xQueueReceive( xQueue_adc, &adc_result, 0);
    if( xStatus == pdPASS ){
          //Envia os valores das variáveis Temperatura e Umidade para o servidor geniot.io
          client.add((char*)TEMPERATURA, (float)adc_result);

          if(client.send()){
            client.vPrintString("-----------------------------------------------");
            client.vPrintString((char*)"Enviado com sucesso!"); 
          } 
    }
//--------------------------------------------------------------------      
    vTaskDelay( 200 / portTICK_PERIOD_MS );
    if(client.read((char*)ALARME))
    {
      client.vPrintString("-----------------------------------------------");
      client.vPrintStringAndFloat((char*)"Alarme: ", client.geniotValue);

      if( (int)client.geniotValue >= 89){
          digitalWrite(relay, HIGH); 
          client.vPrintString("Rele Ligado");  
      } else {
          digitalWrite(relay, LOW);   
          client.vPrintString("Rele Desligado");  
      }

    }

//--------------------------------------------------------------------
    vTaskDelay( 10000 / portTICK_PERIOD_MS );
  }
}




void vTask2( void *pvParameters ){


  BaseType_t xStatus;

  for( ;; ){

    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
    esp_adc_cal_characteristics_t characteristics;
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_11db, ADC_WIDTH_12Bit, &characteristics);
    uint32_t temperatura = adc1_to_voltage(ADC1_CHANNEL_0, &characteristics);
   
    xStatus = xQueueSend( xQueue_adc, &temperatura, 0);
    if( xStatus != pdPASS ){   

    }

    vTaskDelay( 20 / portTICK_PERIOD_MS );

  }
}

void setup() {
  prvSetupHardware(); 

  xQueue_adc = xQueueCreate( 5, sizeof( int ) );
  if( xQueue_adc != NULL ){
    xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE+10000, NULL, 2, NULL, CORE_0 );  
    xTaskCreatePinnedToCore( vTask2, "Task 1", configMINIMAL_STACK_SIZE+1000, NULL, 2, NULL, CORE_1 );   
  }
}

void loop() {  
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




