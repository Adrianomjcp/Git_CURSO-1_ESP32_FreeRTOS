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
#include "freertos/queue.h"  //necessário!

#include "HardwareSerial.h"


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include "cJSON.h"

WiFiMulti wifiMulti;


const char* ca = \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

#define DEBUG 0

#define CORE_0 0 
#define CORE_1 1  //OU tskNO_AFFINITY 

#define WIFISSID "ap 202"       
#define PASSWORD "miragem23"   

/*
 * Protótipos de Função 
*/
void prvSetupHardware( void );
void vPrintString( const char *pcString);
void vPrintStringAndFloat( const char *pcString, float ulValue );
void vPrintStringAndInteger( const char *pcString, uint32_t ulValue );
void vPrintTwoStrings(const char *pcString1, const char *pcString2);
bool vJsonConverter( String &payload, float * result);

void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

/*
 * Funções
*/
QueueHandle_t xQueue;

void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }

  wifiMulti.addAP( WIFISSID, PASSWORD );

}

void vPrintString( const char *pcString ){
  taskENTER_CRITICAL( &myMutex );
  {
    Serial.println( (char*)pcString );
  }
  taskEXIT_CRITICAL( &myMutex );
}
//Atenção Modificado para imprimir FLOAT com duas casas após a vírgula
void vPrintStringAndFloat( const char *pcString, float ulValue ){
  taskENTER_CRITICAL( &myMutex );
  {
    char buffer [50]; 
    sprintf( buffer, "%s %.2f", pcString, ulValue ); 
    Serial.println( (char*)buffer );
  }
  taskEXIT_CRITICAL( &myMutex );
}

void vPrintStringAndInteger( const char *pcString, uint32_t ulValue ){
  taskENTER_CRITICAL( &myMutex );
  {
    char buffer [50]; 
    sprintf( buffer, "%s %lu", pcString, ulValue );
    Serial.println( (char*)buffer );
  }
  taskEXIT_CRITICAL( &myMutex );
}

void vPrintTwoStrings(const char *pcString1, const char *pcString2){
  taskENTER_CRITICAL( &myMutex  );
  {
    char buffer [50]; 
    sprintf(buffer, "%s %s", pcString1, pcString2);
    Serial.println( (char*)buffer );
  }
  taskEXIT_CRITICAL( &myMutex );    
}

void vTask1( void *pvParameters ){

  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

  BaseType_t xStatus;
  vPrintString( "Task1 Init..." ); 
  float value; 

  for( ;; ){

    if( (wifiMulti.run() == WL_CONNECTED) ) {

        HTTPClient http;
      
        if( DEBUG )
          vPrintString( "[HTTP] Enviando..." );

        http.begin( "https://www.geniot.io/things/services/api/v1/variables/S00/value/?token=f4f0b6089b0fd1be614ea384a94d63a3", ca ); //HTTPS
        http.addHeader( "Content-Type", "application/json" );
        http.addHeader( "Connection", "close" );
        
        int httpCode = http.POST( "{\"value\": 76.12}" );

        if( httpCode > 0 ) {

            if( httpCode == HTTP_CODE_OK ) {  //codigo 200
              String payload = http.getString();

              //
              if( DEBUG )
                vPrintString( payload.c_str() ); //estou lendo o JSON (OBJETO DO JAVA)
              
              //https://github.com/DaveGamble/cJSON
              //C:\...\Documents\Arduino\hardware\espressif\esp32\tools\sdk\include\json
              if(vJsonConverter(payload, &value) == true)
              {                         
                  xStatus = xQueueSendToBack( xQueue, &value, portMAX_DELAY );
                  if( xStatus != pdPASS ){

                    if( DEBUG )
                      vPrintString( "Fila cheia." );
                  }
              }

            }
          } else {

            if( DEBUG )
              vPrintTwoStrings( "[HTTP] POST.. falha, error: %s", http.errorToString(httpCode).c_str() );
          }

          http.end();
        }

        //Imprime a quantidade de bytes livres do Stack reservado para esta Task.

        if( DEBUG )
        {
          uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
          vPrintStringAndInteger( "Task1 Stack Size: ", uxHighWaterMark );       
        }


        vTaskDelay( 1000 / portTICK_PERIOD_MS );
      }
    }

void vTask2( void *pvParameters ){

  float lReceivedValue;
  BaseType_t xStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS( 5000UL );

  vPrintString( "Task2 Init..." );
  
  for( ;; ){

    xStatus = xQueueReceive( xQueue, &lReceivedValue, xTicksToWait );
    if( xStatus == pdPASS ){

      //if( DEBUG )
        vPrintStringAndFloat( "Task2 Recebe valor = ", lReceivedValue );

    } else {
      
      if( DEBUG )
        vPrintString( "timeout." );
    }

    vTaskDelay( 500 / portTICK_PERIOD_MS );
  }
}

bool vJsonConverter( String &payload, float * result){

  bool retorno = false; 

  cJSON *json = cJSON_Parse(payload.c_str());
  if(json != NULL)
  {
    const cJSON *name = NULL;
    name = cJSON_GetObjectItem(json, "name");
    if((name->valuestring != NULL))
    {

      if( DEBUG )
        vPrintTwoStrings("Variavel: ", name->valuestring);

    }

    const cJSON *lastvalue = NULL;
    lastvalue = cJSON_GetObjectItem(json, "last_value");
    if((lastvalue->valuestring != NULL))
    {

      if(DEBUG)
        vPrintTwoStrings("last_value: ", lastvalue->valuestring);

      cJSON *json_value = cJSON_Parse(lastvalue->valuestring);
      if(json_value != NULL)
      {
        const cJSON *value = NULL;
        value = cJSON_GetObjectItem(json_value, "value");

        *result = value->valuedouble;
        retorno = true; 
        
        cJSON_Delete(json_value);               
      }

    }
    cJSON_Delete(json);
  } else {retorno = false;}

  return retorno;
}

void setup() {
  prvSetupHardware(); 

  xQueue = xQueueCreate( 5, sizeof( float ) );
  if( xQueue == NULL )
  {
    while(1) {vPrintString("error");}
  }

  xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE+8000, NULL, 2, NULL, CORE_0 );   
  xTaskCreatePinnedToCore( vTask2, "Task 2", configMINIMAL_STACK_SIZE+5000, NULL, 2, NULL, CORE_1 );
}

void loop() {
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




