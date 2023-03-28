#include <stdio.h>
//#include "Tipos.h"
#include "CRCx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define ReceiverPin 14

int bloco_sync[8] = { 1, 1, 0, 1, 1, 1, 1, 1};  //bloco em formato numérico visto que este é transformado em bytes através de shifting, no receptor ele não precisa de saber qual é o bloco de sync só precisa de saber o tamanho
boolean flag_leitura = false;                           //Flag para dizer que o esp32 está ocupado a ler informação (incluí o processo de envio).
boolean flag_dados_disp = false;                        //Flag que indica ao esp32 que a aplicação têm dados para serem lidos.
int buffer_max_position = 0;
int pacotes_recebidos;
int bits_sync = 0;
int bit_lido = 0;
int obtidos = 0;
byte buffer_max[1024];
byte byte_recebido;
TaskHandle_t Task1;
TaskHandle_t Task2;
QueueHandle_t queue;

void ReceberF( void * pvParameters ) {
  for (;;) {
    /*while (digitalRead(ReceiverPin) == LOW) {
      vTaskDelay( pdMS_TO_TICKS(1) );
    }*/
    //while (bits_sync != 32) {
    while (bits_sync != 9) {
      if (bits_sync == 0) {
        vTaskDelay( pdMS_TO_TICKS( 15 ) );
      } else {
        vTaskDelay( pdMS_TO_TICKS( 15 ) );
      }
      bit_lido = digitalRead(ReceiverPin);
      if (bits_sync != 8) {
        //Serial.println(bit_lido);
        xQueueSendToBack(queue, &bit_lido, 0); // Função de envio do bit para ser processado
        //Manual de ref. FreeRTOS capítulo 3.22
      }
      bits_sync++;
    }
   /* while (digitalRead(ReceiverPin) == HIGH) {
      vTaskDelay( pdMS_TO_TICKS(1) );
    } */
    bits_sync = 0;
    pacotes_recebidos++;

  }
}
void ProcEnvioF( void * pvParameters ) {
  for (;;) {
    if (xQueueReceive(queue, buffer_max, 0) == pdPASS) {
      Serial.println(buffer_max[0]);
    } else {
      //A task de receção não recebeu nada;
    }
  }
}
//Tenho de por uma task a tratar do processamento e envio do pacote para a aplicação no PC
//Fazemos isto para evitar possíveis perdas devido aos tempos de processamento já que o tempo
//disponível entre a chegada de cada bit é pequena.

//Sem a aplicação ficamos um pouco limitados na testagem


void setup() {
  Serial.begin(115200);  //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(ReceiverPin, INPUT);
  queue = xQueueCreate(1024, sizeof(byte)); //Criamos uma fila com o mesmo espaço que o buffer máximo;
  //xQueueAddToRegistry(queue, "RecebEnvio" );
  if (queue == NULL) {
    Serial.println("Queue não foi criada");
  }
  xTaskCreatePinnedToCore(
    ProcEnvioF,   /* Task function. */
    "ProcEnvio",     /* nome da task. */
    10000,       /* Tamanho da stack da task */
    NULL,        /* parameteros da task */
    tskIDLE_PRIORITY,           /* prioridade da task */
    &Task1,      /* Handler da Task */
    0);          /* a task vai correr no core 0 */
  delay(200);
  xTaskCreatePinnedToCore(
    ReceberF,   /* Task function. */
    "Receber",     /* nome da task. */
    10000,       /* Tamanho da stack da task */
    NULL,        /* parameteros da task */
    tskIDLE_PRIORITY,           /* prioridade da task */
    &Task2,      /* Handler da Task */
    1);          /* a task vai correr no core 1 */
    delay(200);
}


void loop() {
 /* while (digitalRead(ReceiverPin) == LOW) {
    //Serial.print("EM LOW");
  }
  //while (bits_sync != 32) {
  while (bits_sync != 9) {
    if (bits_sync == 0) {
      delayMicroseconds(1500);
    } else {
      delayMicroseconds(1000);
    }
    bit_lido = digitalRead(ReceiverPin);
    if (bits_sync != 8) {
      Serial.println(bit_lido);
    }
    bits_sync++;
  }
  bits_sync = 0;
  pacotes_recebidos++;
  while (digitalRead(ReceiverPin) == HIGH) {
    //Serial.print("EM high");
  }
  buffer_max_position = 0; 
  while (digitalRead(ReceiverPin) == LOW) {
      taskYIELD();
    }*/
}




//Implementar Task(FreeRTOS) ou multi threading para conseguirmos enviar os pacotes à medida que são recebidos
