#include <stdio.h>
//#include "Tipos.h"
#include "CRCx.h"
#include "time.h"

#define ReceiverPin 14

//Periodo 150micros
//Meio 75 ms
//Tol 45 ms
#define T 120
#define D 40

//int bloco_sync[8] = { 1, 1, 0, 1, 1, 1, 1, 1};  //bloco em formato numérico visto que este é transformado em bytes através de shifting, no receptor ele não precisa de saber qual é o bloco de sync só precisa de saber o tamanho
//0000 0010 Start of Text
//0000 0011 End of Text

int n_bytes_guardados = 0;
byte bloco_start[8] = { 0, 1, 1, 0, 0, 1, 0, 1};
int bloco_end[8] = { 0, 0, 0, 0, 0, 0, 1, 1};
int array_check_flags[9];
boolean flag_leitura = false;                           //Flag para dizer que o esp32 está ocupado a ler informação (incluí o processo de envio).
boolean flag_dados_disp = false;                        //Flag que indica ao esp32 que a aplicação têm dados para serem lidos.
int serial_flag_recebido = 0;
int tamanho_dados = 0;
int buffer_max_position = 0;
int pacotes_recebidos;
int bits_sync = 0;
int bit_lido = 0;
int bit_counter = 0;
time_t time_bit = 0;
time_t tempo_bit_errado = 0;
int obtidos = 0;
byte buffer_payload[8];
int bit_receber;
int somador = 0;
byte byte_queue = 0;
byte byte_recebido = 0;

//Funções provenientes da biblioteca Manchester criada por mdeudon , disponível em:https://github.com/mdeudon/Manchester                                                                                                                                                                                                                                                                                                                                                    

void seek_start()
/*seek for the start signal*/
/*this is a blocking funtion*/
{
  int i=0;
  do
  { 
    if (read_signal()==bloco_start[i]) {i++;}
    else i=0;
  }
  while (i<8);
  return;  
}

void read_byte(byte load[])
{
  for (int i=0;i<8;i++)
  {
  load[i]=read_signal();
  }
}

byte array_to_ascii(byte arr[]) {
    byte ascii = 0;
    for (int i = 7; i >= 0; i--) {
        ascii = ascii << 1;
        ascii = arr[i] | ascii;
    }

    return ascii;
}


byte read_signal()                                                    
/*detect an edge in the signal and determine if it is a synchrone rising/falling edge*/
/*return the corresponding bit*/
/*this is a blocking function*/
{
  boolean state = digitalRead(ReceiverPin);
  long start=micros();
  long delta; 
  long time_out = T+D; //if we do not detect 2 valids edges in this interval someting's wrong
  
  do
  {  
      do
      {
        //looking for a next edge
        //analyse the period to check if it can be a sync bit
        delta = micros()-start;
      }
      while ( (state==digitalRead(ReceiverPin))); //  && delta<time_out );
      if (delta>=time_out) 
      {
        //something's wrong
        return 3;
      }   
      else
      {
        state=!state; //it is not a time out so the state of the ligne has changed
      }
      if ( (delta<=T+D) && (delta>=T-D) ) 
      {
      return (byte)state;
      }
      else
      {
        //it is not a sync edge
        //could be a transiant edge at T/2 or a noise
        //we do nothing
        //if the next edge is not sync the function will throw a time out
      }
      //let restart it for the next edge
  }
  while (true);
}


void setup() {
  Serial.begin(115200);  //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(ReceiverPin, INPUT);
  delay(100);
}

void loop() {

    seek_start(); //Procura pelo bloco start na mensagem
    read_byte(buffer_payload); //Recebe o conteúdo enviado
    char to_send = (char)array_to_ascii(buffer_payload);
    Serial.write(to_send);
    //Serial.println(to_send);
    //Serial.print(byte_recebido);
  }
