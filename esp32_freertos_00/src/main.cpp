/*
 --Este Projeto tem por objetivo criar duas threads no FreeRTOS.
 --Cada thread é responsável em enviar uma string pela UART do ESP32 de maneira concorrente.
*/

/*CHAMAR BIBLIOTECAS BÁSICAS PAR O C*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

/*BIBLIOTECAS PARA CRIAÇÃO DAS TASKS*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "HardwareSerial.h"

/*PROTÓTIPO DE FUNÇÕES */ 
void prvSetupHardware( void );
void vPrintString( const char *pcString);

void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

// GLOBAL
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;


/*FUNÇÕES*/

/*configuração do hardware */
void prvSetupHardware( void ){
  Serial.begin( 9600 ); 
}



/*função para envelopar o setor crítico*/
void vPrintString( const char *pcString ){

  taskENTER_CRITICAL( &myMutex ); /*PULO DO GATO - envelopo o setor crítico com o comando ENTER_CRITICAL (exclusivo do FreeRTOS)*/
  /*desliga a chave geral as interrupções - estou desligando a interrupção de um único core, ainda é posível que o oturo core acesse esse dispositivo - ação
  além de desligar a interrupção também tenho que habilitar o mutex (banheiro, quem entra usa fica uma espera) - é tipo um semáforo de uma única posição, contador
  de uma única função, o semáforo qualquer um pode dar uma release e implementar o contador.. no caso do mutex somente a task que entrou no mutex pode liberar*/
  {
    Serial.println( (char*)pcString ); /*esse cara é um setor crítico*/
  }
  taskEXIT_CRITICAL( &myMutex );
  /*liga a chave geral das interrupções*/


}


//função que fica escrevendo na serial
void vTask1( void *pvParameters ){
  const char *pcTaskName = "Task 1 is running\r\n";
  volatile uint32_t ul;

  for( ;; ){
    vPrintString( pcTaskName );      
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); /*vTaskDelay é função do FreeRTOS*/
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


/*MAIN DO MEU PROGRAMA*/
void setup() {
  prvSetupHardware(); //chamo a função para configurar o hardware 
   
  /*FUNÇÃO PARA CRIAÇÃO DA TASK*/ 
  xTaskCreate( vTask1, //ponteiro da função, chamo a função vTask1
              "Task 1", //é um nome qualquer, utlizo para saber qual a task que está em operação 
              configMINIMAL_STACK_SIZE, //é onde eu vou determinar o comprimento da stack, quanto de memória vou reservar?
              NULL, //se eu vou carregar algum vamor na criação da task, esse valor é do tipo void, pode ser qualquer coisa 
              1, //nesse ponto defino a PRIORIDADE da task (task 2 > task 1), quanto maior for o índice maior a prioridade de interrupção
              NULL ); //variavel do tipo task handle que possibilita a manipulação da task criada, criação, mudar prioriadde, mudar nome e tc..  

  xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
}



void loop() {   //TASK loop
  vTaskDelay( 100 / portTICK_PERIOD_MS );
}
