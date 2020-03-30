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
#include "freertos/queue.h"  //necessário!

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "cJSON.h"
#include <driver/adc.h>
#include "esp_system.h"
#include "esp_adc_cal.h"

#include <Wire.h>
#include "SSD1306.h" 
SSD1306  display(0x3c, 5, 4);

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

#define CORE_0 0 
#define CORE_1 1  //OU tskNO_AFFINITY 

#define WIFISSID "FSimplicio"       
#define PASSWORD "fsimpliciokzz5"   

#define DEBUG 0
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
void vTask3( void *pvParameters );
void vTask4( void *pvParameters );

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
QueueHandle_t xQueue_oled;
QueueHandle_t xQueue_adc;


/*
 * Funções
*/
void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }

  wifiMulti.addAP( WIFISSID, PASSWORD );


  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
}


void drawFontFaceDemo() {
  // Font Demo1
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Hello world");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 10, "Hello world");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 26, "Hello world");
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

  float value; 
  int adc_result; 

  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
  BaseType_t xStatus;
  
  if( DEBUG )
    vPrintString( "Task1 Init..." );
  
  for( ;; ){

    xStatus = xQueueReceive( xQueue_adc, &adc_result, 0);
    if( xStatus != pdPASS ){
      continue; 
    }

    if( (wifiMulti.run() == WL_CONNECTED) ) {

      HTTPClient http;
      http.begin( "https://www.geniot.io/things/services/api/v1/variables/S00/value/?token=7f33f3ce94f774494feb8f1843511509", ca ); //HTTPS
      http.addHeader( "Content-Type", "application/json" );
      http.addHeader( "Connection", "close" ); 

      String dta = String(adc_result); 

      if( DEBUG )
        vPrintStringAndInteger( "ADC:", adc_result );

      int httpCode = http.POST( "{\"value\":" + dta + "}" );

      if( httpCode > 0 ) {

        if( DEBUG )
          vPrintStringAndInteger("HTTP_CODE_RETURN:", httpCode);


        if( httpCode == HTTP_CODE_OK ) { 
          String payload = http.getString();

          if( DEBUG )
            vPrintString( payload.c_str() );

          if(vJsonConverter(payload, &value) == true )
          {                         
            xStatus = xQueueSendToBack( xQueue_oled, &value, portMAX_DELAY );
            if( xStatus != pdPASS ){

              if( DEBUG )
                 vPrintString( "Fila cheia." );
            }
          }

        }
      } else {

        if( DEBUG )
          vPrintTwoStrings( "[HTTP] POST.. falha, error: %s\n", http.errorToString(httpCode).c_str() );
      }

      http.end();
    }

    if( DEBUG )
    {
      uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
      vPrintStringAndInteger( "Task1 Stack-Size: ", uxHighWaterMark );
    }

    vTaskDelay( 5000 / portTICK_PERIOD_MS );
  }
}

void vTask2( void *pvParameters ){
  
  if( DEBUG )
    vPrintString( "Task2 Init..." );
  
  for( ;; ){

    vTaskDelay( 500 / portTICK_PERIOD_MS );
  }
}


void vTask3( void *pvParameters ){

  float lReceivedValue;
  BaseType_t xStatus;

  if( DEBUG )
    vPrintString( "Task3 Init..." );
  
  for( ;; ){ 


    xStatus = xQueueReceive( xQueue_oled, &lReceivedValue, portMAX_DELAY );
    if( xStatus == pdPASS ){
      
      display.clear();    
      display.drawString(0, 0, String(lReceivedValue));
      display.display();
    }

    vTaskDelay( 1000 / portTICK_PERIOD_MS );
  }
}

void vTask4( void *pvParameters ){
 
  BaseType_t xStatus;
  int value;

  if( DEBUG )
    vPrintString( "Task4 Init..." );
  
  for( ;; ){

    /*
     * Leitura ADC 
    */
    //adc1_config_width(ADC_WIDTH_12Bit);
    //adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_0db);
    //value = adc1_get_voltage(ADC1_CHANNEL_0);
    
    #define V_REF 1100  // ADC reference voltage
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
    // Calculate ADC characteristics i.e. gain and offset factors
    esp_adc_cal_characteristics_t characteristics;
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_11db, ADC_WIDTH_12Bit, &characteristics);
    // Read ADC and obtain result in mV
    value = adc1_to_voltage(ADC1_CHANNEL_0, &characteristics);

  
    //if( DEBUG )
      vPrintStringAndInteger( "Task 4 ADC:", value );

    xStatus = xQueueSendToBack( xQueue_adc, &value, portMAX_DELAY );
    if( xStatus != pdPASS ){             
      
      if( DEBUG )
        vPrintString( "Fila cheia." );

    }
    vTaskDelay(2000 / portTICK_PERIOD_MS );
  }
}

bool vJsonConverter( String &payload, float * result)
{
  bool retorno = false; 
  //https://github.com/DaveGamble/cJSON
  cJSON *json = cJSON_Parse(payload.c_str());
  if(json != NULL)
  {
    /*
    const cJSON *name = NULL;
    name = cJSON_GetObjectItem(json, "name");
    if((name->valuestring != NULL))
    {
      if(debug())
        vPrintTwoStrings("Checking monitor: ", name->valuestring);
    }
   */
    const cJSON *lastvalue = NULL;
    lastvalue = cJSON_GetObjectItem(json, "last_value");
    if((lastvalue->valuestring != NULL))
    {
      if( DEBUG )
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

  xQueue_oled = xQueueCreate( 5, sizeof( float ) );
  if( xQueue_oled == NULL )
  {
    while(1) {vPrintString("Queue error");}
  }

  xQueue_adc = xQueueCreate( 1, sizeof( int ) );
  if( xQueue_adc == NULL )
  {
    while(1) {vPrintString("Queue_adc error");}
  }

  xTaskCreatePinnedToCore( vTask1, "Task 1", configMINIMAL_STACK_SIZE+8000, NULL, 2, NULL, CORE_0 );  //https 
  xTaskCreatePinnedToCore( vTask2, "Task 2", configMINIMAL_STACK_SIZE+5000, NULL, 2, NULL, CORE_1 );  //Em branco
  xTaskCreatePinnedToCore( vTask3, "Task 3", configMINIMAL_STACK_SIZE+1000, NULL, 2, NULL, CORE_1 );  //display
  xTaskCreatePinnedToCore( vTask4, "Task 4", configMINIMAL_STACK_SIZE+1000, NULL, 3, NULL, CORE_1 );  //adc

}

void loop() {   //TASK loop
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}




