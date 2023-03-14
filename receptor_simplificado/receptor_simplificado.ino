#include <stdio.h>
//#include "Tipos.h"
#include "CRCx.h";

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



void setup() {
  Serial.begin(115200);  //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(ReceiverPin, INPUT);
}


//Tenho de por uma task a tratar do processamento e envio do pacote para a aplicação no PC
//Fazemos isto para evitar possíveis perdas devido aos tempos de processamento já que o tempo
//disponível entre a chegada de cada bit é pequena.

//Sem a aplicação ficamos um pouco limitados na testagem

void loop() {
  
  while (digitalRead(ReceiverPin) == LOW) {
    //Serial.print("EM LOW");
  }
  //while (bits_sync != 32) { 
  while(bits_sync != 9){
    if(bits_sync == 0){
      delayMicroseconds(1500);
    }else{
      delayMicroseconds(1000);
    }
    bit_lido = digitalRead(ReceiverPin);
    if(bits_sync != 8){
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
}

//Implementar Task(FreeRTOS) ou multi threading para conseguirmos enviar os pacotes à medida que são recebidos
