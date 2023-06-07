#include <BTAddress.h>
#include <BTScan.h>
#include <BluetoothSerial.h>
#include <BTAdvertisedDevice.h>

#include <RS-FEC.h>
#include <stdio.h>
//#include <printf.h>
#include <Arduino.h>
#include "Tipos.h"
#include "CRCx.h"

#define SendingPin 14  //Define o Pino pelo qual irá ser usado como pino de envio de dados
#define T 100
#define THalf 50
//Um pino que não têm nenhum informação importante para qualquer protocolo é o pino 32, um que apenas têm como funções extra ser touch sensitive



//Sender TxD;
int bloco_sync[8] = { 1, 1, 0, 1, 1, 1, 1, 1 };  //bloco em formato numérico visto que este é transformado em bytes através de shifting
//0000 0010 Start of Text
//0000 0011 End of Text
byte bloco_start[8] = {0,1,1,0,0,1,0,1};
byte bloco_end = 0b00000011;
int flag_leitura = 0;                            //Flag para dizer que o esp32 está ocupado a ler informação (incluí o processo de envio).
int flag_dados_disp = 0;                         //Flag que indica ao esp32 que a aplicação têm dados para serem lidos.
byte cabecalho[5];
byte tag_fluxo[1];  //Identificação do grupo de fluxo a que uma trama pertence, ou seja, como a trama da parte de uma mensagem é retransmitida 3 vezes, é identificado o grupo de tramas a qual esta pertençe
byte tag_msg[1];    //Identificação da mensagem quanto à sua posição no seu grupo de fluxo
byte tamanho_msg[1];
int tamanho_boolean = 0;
int mensagem_boolean = 0;
int grupo_fluxo = 0;
int tamanho_recebido = 0;  //Primeiros bytes recebidos representam o tamanho da mensagem que provém da aplicação, se esta variável for igual a 0 é sinal que a aplicação deixou de ter dados para enviar e deseja parar


//Definimos o modo em que o pino escolhido como emissor vai trabalhar

//Funções provenientes da biblioteca Manchester 

void send_sync()
{
  //sending a sync signal
  for (byte i=0;i<32;i++)
  {
    digitalWrite(SendingPin,HIGH);
    delayMicroseconds(THalf);
    digitalWrite(SendingPin,LOW);
    delayMicroseconds(THalf);
  }
  digitalWrite(SendingPin,HIGH);
}

void send_load(byte start[], boolean state)
//send a byte but without sync
//start must be an array of 8 bytes, no more no less !
//state is the current state of the ouput line
{
  for (byte i = 0; i < 8; i++)
  {
    if (start[i] == 1)
    {
      if (state == LOW)
      {
        delayMicroseconds(T);
        digitalWrite(SendingPin, HIGH);
        state = HIGH;
      }
      else //state==HIGH
      {
        delayMicroseconds(THalf);
        digitalWrite(SendingPin, LOW);
        delayMicroseconds(THalf);
        digitalWrite(SendingPin, HIGH);
      }
    }
    else //start[i]==0
    {
      if (state == LOW)
      {
        delayMicroseconds(THalf);
        digitalWrite(SendingPin, HIGH);
        delayMicroseconds(THalf);
        digitalWrite(SendingPin, LOW);
      }
      else //state==HIGH
      {
        delayMicroseconds(T);
        digitalWrite(SendingPin, LOW);
        state = LOW;
      }
    }
  }
}

void ascii_to_array(byte s, byte arr[]) {
  byte mask = 0b00000001;
  for (int i = 0; i < 8; i++) {
    arr[i] = s & mask;
    arr[i] = arr[i] >> i;
    mask = mask << 1;
  }
}

void send_byte(byte car)
//send a sync signal + a start byte + load
//load is an array of 8 bit, no more no less !!
{
  byte load[8];
  ascii_to_array(car, load);
  boolean state; //state of the line

  send_sync();
  state = HIGH; //Como o sync é constituido apenas por valores HIGH, este é feito desta maneira

  //Envia o bloco_start para indicar que vamos começar a enviar dados e desbloquear o receptor
  send_load(bloco_start,state);
  //send the payload
  send_load(load, state);
  return;
}

void formar_enviar_pacote(byte* mensagem, int grupo_fluxo_envio, int tamanho_msg) {
  int n_reenvio = 0;
  //Simplificação do código para realizar apenas o envio do pacote recebido através da aplicação do lado do emissor

  byte pacote[tamanho_msg + 1];
  pacote[tamanho_msg + 1] = '\0';
  memcpy(pacote, mensagem, tamanho_msg);
  //------ OPERAÇÕES AO NÍVEL DO BIT, verifiquem no dia 8/3 se isto está a apresentar os valores certos no receptor (o valor HIGH equivale a 5V e o LOW a 0V).
  for (int x = 0; x < tamanho_msg; x++) {
    //Serial.println(pacote[x]);
    send_byte(pacote[x]);
    delayMicroseconds(10);
  }
}



void setup() {
  Serial.begin(115200);  //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(SendingPin, OUTPUT);
  digitalWrite(SendingPin, LOW);
  delay(100);
}

void loop() {
  tamanho_recebido = 0;
  /*
    if(Serial.available()){
    Serial.println(Serial.read());
    }
  */
  if (!flag_dados_disp) {
    //Serial.print("Hello");
    flag_dados_disp = 1;
    while (flag_dados_disp == 1) {
      //Serial.println("Hello2");
      flag_leitura = 1;
      //Enquanto estiver a ler e não chegar alguma notificação de stop, ele continua a ler
      while (flag_leitura == 1 && flag_dados_disp == 1) {  //Por variavél para sair.
        //Aguarda o input do tamanho
        while (tamanho_boolean == 0) {
          //Serial.println(Serial.available());
          if (Serial.available() >= 2) {
            byte buf_temp[2];
            //buf_temp[2] = '\0';
            Serial.readBytes(buf_temp, 2); //-> O tamanho recebido têm de ser dito denovo;
            // Serial.print(buf_temp[0]);
            // Serial.print(buf_temp[1]);
            if (buf_temp[0] == 48) {
              //Serial.println("Tagala");
              char temp[0];
              //temp[1] = '\0';
              sprintf(temp, "%c", buf_temp[1]);
              tamanho_recebido = atoi(temp);
            } else {
              char temp[1];
              //temp[2] = '\0';
              sprintf(temp, "%c%c", buf_temp[0], buf_temp[1]);
              tamanho_recebido = atoi(temp);
            }
            //Serial.print(tamanho_recebido);
            tamanho_boolean = 1;
          }
        }
        if (tamanho_recebido != 99) {
          grupo_fluxo++;
          byte temp_dados[tamanho_recebido + 1];
          temp_dados[tamanho_recebido + 1] = '\0';
          if (flag_dados_disp == 1) {
            while (mensagem_boolean == 0) {
              //Serial.println(Serial.available());
              if (Serial.available() >= tamanho_recebido) {
                Serial.readBytes(temp_dados, tamanho_recebido);  //Recebemos a quantidade de bytes dita pela aplicação e guardamos num buffer, neste caso a variável temporária para o registo dos mesmos
                mensagem_boolean = 1;
              }
            }
            //Funções de envio arduino - arduino;
            formar_enviar_pacote(temp_dados, grupo_fluxo, tamanho_recebido);
            //Serial.println("Sai do envio");
            Serial.write(flag_leitura);  //A aplicação que envia os dados fica à espera de receber uma flag para saber se pode enviar mais dados
            //Na parte do receptor têmos de receber os bits todos e enquanto o receptor processo aos poucos ele pode receber mais, simplesmente fica num buffer;
          }
          mensagem_boolean = 0;
          tamanho_boolean = 0;
        } else {
          int tamanho_temp = 3;
          byte dados[5] = {0b00000010, 0b00000101, 0b00000101,0b00000101, 0b00000011};
          formar_enviar_pacote(dados, grupo_fluxo, tamanho_temp);
          flag_dados_disp = 0;
          grupo_fluxo--;
          //Serial.print("STOP");
        }
      }
    }
  }
}
