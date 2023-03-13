#include <stdio.h>
//#include "Tipos.h"
#include "CRCx.h";

#define ReceiverPin 14

int bloco_sync[8] = { 1, 1, 0, 1, 1, 1, 1, 1};  //bloco em formato numérico visto que este é transformado em bytes através de shifting, no receptor ele não precisa de saber qual é o bloco de sync só precisa de saber o tamanho
boolean flag_leitura = false;                           //Flag para dizer que o esp32 está ocupado a ler informação (incluí o processo de envio).
boolean flag_dados_disp = false;                        //Flag que indica ao esp32 que a aplicação têm dados para serem lidos.
byte cabecalho[6];
byte tag_fluxo[1];  //Identificação do grupo de fluxo a que uma trama pertence, ou seja, como a trama da parte de uma mensagem é retransmitida 3 vezes, é identificado o grupo de tramas a qual esta pertençe
byte tag_msg[1];    //Identificação da mensagem quanto à sua posição no seu grupo de fluxo
byte cauda[17];
byte tamanho_msg[1];
int pacotes_recebidos;
byte* dados;  //até 255 bytes
int grupo_fluxo = 0;
int bits_sync = 0;
int bit_lido = 0;
//byte buffer_max[];
byte byte_recebido;
//Variável cabeçalho
//Variável cauda
//Variável pacote , os pacotes são adicionados na função de formar_pacote()


void setup() {
  Serial.begin(115200);  //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(ReceiverPin, INPUT);
}


//Tenho de por uma task a tratar do processamento e envio do pacote para a aplicação no PC
//Fazemos isto para evitar possíveis perdas devido aos tempos de processamento já que o tempo
//disponível entre a chegada de cada bit é pequena.

void loop() {
  while (digitalRead(ReceiverPin) == LOW){
    //Serial.print("EM LOW");
  }
  //Muito simplificida
  while(bits_sync != 32){
    bit_lido = digitalRead(ReceiverPin);
    Serial.println(bit_lido); 
    pacotes_recebidos++;
    bits_sync++;
  }
  while (digitalRead(ReceiverPin) == HIGH){
    //Serial.print("SO HIGH");
  }
}

/*void setup() {
  // setup code
}

void loop() {
  // wait for start bit
  while (digitalRead(dataPin) == HIGH) {
    // do nothing
  }
  
  // read data bits
  int receivedData = 0;
  for (int i = 0; i < 8; i++) {
    receivedData |= (digitalRead(dataPin) << i);
    delay(10);
  }
  
  // wait for stop bit
  while (digitalRead(dataPin) == LOW) {
    // do nothing
  }
  
  // process received data
  // ...
}*/
