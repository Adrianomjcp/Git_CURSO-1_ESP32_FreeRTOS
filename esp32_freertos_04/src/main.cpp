#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "HardwareSerial.h"


//QUAL É MEU CERTIFICADO?
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

WiFiMulti wifiMulti;

//CONEXÃO COM A WIFI
#define WIFISSID "ap 202"       
#define PASSWORD "miragem23"      

void setup() {

    Serial.begin( 9600 );
    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf( "[SETUP] WAIT %d...\n", t );
        Serial.flush();
        delay( 1000 );
    }

    wifiMulti.addAP( WIFISSID, PASSWORD );

}

//OBS: NÃO ESTOU UTILIZANDO OS DOIS NÚCLEOS NESSE PROGRAMA..


void loop() {
    // wait for WiFi connection
    if( (wifiMulti.run() == WL_CONNECTED) ) {

        HTTPClient http;

        Serial.print( "[HTTP] begin...\n" );

        //CONEXÃO SEGURA
        http.begin( "https://www.geniot.io/things/services/api/v1/variables/S00/value/?token=f4f0b6089b0fd1be614ea384a94d63a3", ca );
        //a string acima utiliza a plataforma geniot.io

        //CONEXÃO NÃO SEGURA
        //Conexão sem o certificado de segurança <- ATENÇÃO PARA ESSA PARTE
        //http.begin( "https://www.geniot.io/things/services/api/v1/variables/S00/value/?token=f4f0b6089b0fd1be614ea384a94d63a3");  
        http.addHeader( "Content-Type", "application/json" );
        http.addHeader( "Connection", "close" );

        //int httpCode = http.GET();
        int httpCode = http.POST( "{\"value\": 76.12}" );
        


        //A PARTIR DAQUI ANALISO O CÓDIGO DE RETORNO DO SERVIDOR..
        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf( "[HTTP] POST... code: %d\n", httpCode );

            // file found at server
            if( httpCode == HTTP_CODE_OK ) {   //COMPARA COM O VALOR 200
                String payload = http.getString();
                Serial.println( payload );
            }
        } else {
            Serial.printf( "[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str() );
        }

        http.end();
    }

    delay(10000);
}



