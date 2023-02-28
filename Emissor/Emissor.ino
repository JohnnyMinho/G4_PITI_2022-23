#include <printf.h>
#include <stdio.h>
#include "Tipos.h"


#define SendingPin 32//Define o Pino pelo qual irá ser usado como pino de envio de dados
//Um pino que não têm nenhum informação importante para qualquer protocolo é o pino 32, um que apenas têm como funções extra ser touch sensitive

int bloco_sync[20] = {1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0}; //bloco em formato numérico visto que este é transformado em bytes através de shifting
byte cabecalho[33];
byte cauda[18];
byte* dados; //até 255 bytes
//Variável cabeçalho -
//Variável cauda
//Variável pacote , os pacotes são adicionados na função de formar_pacote()

//Definimos o modo em que o pino escolhido como emissor vai trabalhar

byte* formular_cabecalho() {
  byte sync_block[3]; //bloco de sincronização no qual 20 bits vão servir para sincronizar ambos os elementos, ou seja, ao receber uma mensagem o receptor vai começar por igualar todos os valores através de um ciclo.
  byte tipo_trama;
  sync_block[3] = '\0';
  int i = 0; //varíavel temp para rodar o bloco de sync
  int x = 0; //varíavel para fazer shifting dos bits do bloco de sync
  for (i = 0; i < 3; i++){
    for (x = 0 + x * i; x < 8 * i; x++){
      sync_block[i] |= ~(bloco_sync[x] << x);
    }
  }
  int option = 0; //A escolha para já é hardcoded , consoante o que for preciso é alterado, assumo que seja necessário indicar como um argumento ou então como uma questão a adicionar durante a comunicação já que a mesma não é constante
  switch (option) {
    case 0:
      tipo_trama = HELLO;
      break;
    case 1:
      tipo_trama = Sync;
      break;
    case 2:
      tipo_trama = Dados0;
      break;
    case 3:
      tipo_trama = Dados1;
      break;
    case 4:
      tipo_trama = Dados2;
      break;
  }
  int tamanho_msg = 0;
  dados = (byte*) malloc(tamanho_msg * sizeof(byte));
  for(int o=0;o<tamanho_msg; o++){
    
  }
  byte start_bit = 0b00000001;
  free(dados); //TÊMOS DE LIBERTAR A MEMÓRIA EM C , NÃO SE ESQUEÇAM
}

byte* formular_cauda() {

}

byte* formar_pacote() {

}

byte* formar_CRC() {

}

void enviar(byte* mensagem) {

}

void setup() {
  Serial.begin(115200);
  pinMode(SendingPin, OUTPUT);
}

void loop() {

}
