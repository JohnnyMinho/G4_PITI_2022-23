#include <printf.h>
#include <stdio.h>
#include <Arduino.h>
#include "Tipos.h"
#include "CRCx.h"

#define SendingPin 14//Define o Pino pelo qual irá ser usado como pino de envio de dados
//Um pino que não têm nenhum informação importante para qualquer protocolo é o pino 32, um que apenas têm como funções extra ser touch sensitive

int bloco_sync[8] = {1, 1, 0, 1, 1, 1, 1, 1}; //bloco em formato numérico visto que este é transformado em bytes através de shifting
int flag_leitura = 0; //Flag para dizer que o esp32 está ocupado a ler informação (incluí o processo de envio).
int flag_dados_disp = 0; //Flag que indica ao esp32 que a aplicação têm dados para serem lidos.
byte cabecalho[6];
byte tag_fluxo[1]; //Identificação do grupo de fluxo a que uma trama pertence, ou seja, como a trama da parte de uma mensagem é retransmitida 3 vezes, é identificado o grupo de tramas a qual esta pertençe
byte tag_msg[1]; //Identificação da mensagem quanto à sua posição no seu grupo de fluxo
byte tamanho_msg[1];
int tamanho_boolean = 0;
int mensagem_boolean = 0;
int grupo_fluxo = 0;
int tamanho_recebido = 0;//Primeiros bytes recebidos representam o tamanho da mensagem que provém da aplicação, se esta variável for igual a 0 é sinal que a aplicação deixou de ter dados para enviar e deseja parar

//Definimos o modo em que o pino escolhido como emissor vai trabalhar

byte* formular_cabecalho(int grupo_fluxo_envio2, byte tmnh_msg, byte n_envio) {

  byte sync_block = {0b11011111}; //bloco de sincronização no qual 20 bits vão servir para sincronizar ambos os elementos, ou seja, ao receber uma mensagem o receptor vai começar por igualar todos os valores através de um ciclo.
  byte tipo_trama;
  int tamanho_ocupado = 0; //Variável de auxilio devido às características do C

  int option = 1; //A escolha para já é hardcoded , consoante o que for preciso é alterado, assumo que seja necessário indicar como um argumento ou então como uma questão a adicionar durante a comunicação já que a mesma não é constante
  switch (option) {
    case 0:
      tipo_trama = HELLO;
      break;
    case 1:
      tipo_trama = Dados0;
      break;
    case 2:
      tipo_trama = Dados1;
      break;
    case 3:
      tipo_trama = Dados2;
      break;
  }
  cabecalho[0] = sync_block;
  byte grupo_fluxo_byte = byte(grupo_fluxo_envio2);
  //Serial.println(grupo_fluxo_byte);
  cabecalho[1] = grupo_fluxo_byte;
  tamanho_ocupado = 1;
  //Serial.println(tipo_trama);
  cabecalho[2] = tipo_trama;
  cabecalho[3] = tmnh_msg;
  cabecalho[4] = grupo_fluxo_byte;
  //memcpy(cabecalho + tamanho_ocupado, (byte*)grupo_fluxo_byte, sizeof(grupo_fluxo_byte));
  cabecalho[5] = n_envio;
  //memcpy(cabecalho + tamanho_ocupado, (byte*)n_envio, sizeof(n_envio));
  cabecalho[6] = '\0';
  return cabecalho;
}

void formar_enviar_pacote(byte* mensagem, int grupo_fluxo_envio, int tamanho_msg) {
  int n_reenvio = 0;
  /*dados = (byte*) malloc(sizeof(mensagem));
    for (int o = 0; o < tamanho_msg; o++) {
    dados[o] = mensagem[o];
    }*/
  //Formação do CRC
  uint16_t result16 = crcx::crc16(mensagem, tamanho_msg);
  byte crc[2];
  crc[0] = (result16 >> 8) & 0xFF; //O MSB fica na primeira posição do array
  crc[1] = (result16 & 0xFF);// O LSB fica na ultima posição do array
  //Fim da formação do CRC
  byte tamanho_msg_byte = byte(tamanho_msg);
  byte n_reenvio_byte = byte(n_reenvio);
  int len = 5 + tamanho_msg + 2;
  //Serial.print("Mem livre:");
  //Serial.println(ESP.getFreeHeap());
  byte pacote[len];
  byte* cabec_pacote = formular_cabecalho(grupo_fluxo_envio, tamanho_msg_byte, n_reenvio_byte);
  //A função de formular o cabeçalho em algum erro ao nível da memória
  memcpy(pacote, cabec_pacote, 5); //Muitos warnings neste tipo de operações, ver uma maneira mais "aceitável" de fazer este tipo de op.
  int tamanho_ocupado = 5;
  memcpy(pacote + tamanho_ocupado , mensagem, tamanho_msg);
  tamanho_ocupado = tamanho_ocupado + tamanho_msg;
  pacote[tamanho_ocupado] = crc[0];
  pacote[tamanho_ocupado + 1] = crc[1];
  tamanho_ocupado = tamanho_ocupado + 2;
  pacote[tamanho_ocupado] = '\0';
  Serial.println(pacote[5]);
  Serial.println(pacote[6]);
  Serial.println(pacote[7]);
  Serial.println(pacote[8]);
  Serial.println(pacote[9]);
  Serial.println(pacote[10]);
  //Soluções -> Ver se falta o bit de fecho ou então mudar para um array de tamanho fixo

  //Falta implementar o método de envio com recurso ao digitalwrite() ler documentação e ver exemplos do digitalwrite() https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/
  //Segundo a documentação é melhor utilizar um pino de input que pussa uma resistência de pull-up (ver o porque de ser melhor)

  //------ OPERAÇÕES AO NÍVEL DO BIT, verifiquem no dia 8/3 se isto está a apresentar os valores certos no receptor (o valor HIGH equivale a 5V e o LOW a 0V).
  for (int tentativas = 0; tentativas < 3; tentativas++) { //Acho que o que está no interiro não é a melhor maneira de fazer isto, mas revejam sff.
    byte n_reenvio_byte = byte(n_reenvio);
    byte* cabec_pacote = formular_cabecalho(grupo_fluxo_envio, tamanho_msg_byte, n_reenvio_byte);
    int tamanho_retirar = tamanho_msg + 2;
    memcpy(pacote - tamanho_retirar, cabec_pacote, 6);
    Serial.println("Começa o envio");
    for (int x = 0; x < tamanho_ocupado - 1; x++) {
      for (int i = 7; i >= 0 ; i--) {
        int bit_lido = (pacote[x] >> i) & 0x01;   // obter o valor do bit

        if (bit_lido == 1) {
          digitalWrite(SendingPin, HIGH);    // o pino fica com o valor de 1 ou 0 , HIGH ou LOW respetivamente
        } else {
          digitalWrite(SendingPin, LOW);
        }
        Serial.println(bit_lido);
        //delayMicroseconds(7); // PARA A PRIMEIRA TESTAGEM COM O RECEPTOR, quando o receptor conseguir receber mensagens tentem sem este delay
      }
    }
  }
}

/*byte* formar_CRC(byte* dados, int tamanho_dados) { //Função que realiza a formulação do CRC e devolve o resultado como uma variável de 16 bytes sem associação a um tipo de variável em específico (unsigned)
  uint16_t result16 = crcx::crc16(dados, tamanho_dados);
  byte crc_bytes[2];
  crc_bytes[0] = (result16 >> 8) & 0xFF; //O MSB fica na primeira posição do array
  crc_bytes[1] = (result16 & 0xFF);// O LSB fica na ultima posição do array
  return crc_bytes;
  } //Como a linguagem C dealoca a memória usada para a função mal esta acabe, é uma boa prática evitar que tentemos devolver arrays como um resultado.*/

void setup() {
  Serial.begin(115200); //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(SendingPin, OUTPUT);
}

void loop() {
  if (Serial.available() && !flag_dados_disp) {
    Serial.println("Hello1");
    flag_dados_disp = 1;
    while (flag_dados_disp == 1) {
      Serial.println("Hello2");
      flag_leitura = 1;
      //Enquanto estiver a ler e não chegar alguma notificação de stop, ele continua a ler
      while (flag_leitura == 1 && flag_dados_disp == 1) { //Por variavél para sair.
        //Aguarda o input do tamanho
        while (tamanho_boolean == 0) {
          if (Serial.available() >= 2 ){
            tamanho_recebido = Serial.readStringUntil('\n').toInt(); //-> O tamanho recebido têm de ser dito denovo;
            Serial.println(tamanho_recebido);
            tamanho_boolean = 1;
          }
        }
        grupo_fluxo++;
        if (tamanho_recebido == 0) {
          Serial.println("Acabaram os dados");
          flag_dados_disp = 0;
          grupo_fluxo--;
        }
        byte temp_dados[tamanho_recebido + 1];
        temp_dados[tamanho_recebido + 1] = '\0';
        if (flag_dados_disp == 1) {
          while (mensagem_boolean == 0) {
            if (Serial.available() >= tamanho_recebido) {
              Serial.println("Ler mensagem");
              Serial.readBytes(temp_dados, tamanho_recebido); //Recebemos a quantidade de bytes dita pela aplicação e guardamos num buffer, neste caso a variável temporária para o registo dos mesmos
              mensagem_boolean = 1;
            }
          }
          //Funções de envio arduino - arduino;
          formar_enviar_pacote(temp_dados, grupo_fluxo, tamanho_recebido);
          Serial.println("Sai do envio");
          Serial.write(flag_leitura); //A aplicação que envia os dados fica à espera de receber uma flag para saber se pode enviar mais dados
          //Na parte do receptor têmos de receber os bits todos e enquanto o receptor processo aos poucos ele pode receber mais, simplesmente fica num buffer;
        }
        mensagem_boolean = 0;
        tamanho_boolean = 0;
      }
    }
  }
}
