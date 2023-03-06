#include <printf.h>
#include <stdio.h>
#include "Tipos.h"
#include "CRCx.h";

#define SendingPin 32//Define o Pino pelo qual irá ser usado como pino de envio de dados
//Um pino que não têm nenhum informação importante para qualquer protocolo é o pino 32, um que apenas têm como funções extra ser touch sensitive

int bloco_sync[10] = {1, 1, 0, 1, 1, 1, 1, 0, 1, 1}; //bloco em formato numérico visto que este é transformado em bytes através de shifting
boolean flag_leitura = false; //Flag para dizer que o esp32 está ocupado a ler informação (incluí o processo de envio).
boolean flag_dados_disp = false; //Flag que indica ao esp32 que a aplicação têm dados para serem lidos.
byte cabecalho[6];
byte tag_fluxo[1]; //Identificação do grupo de fluxo a que uma trama pertence, ou seja, como a trama da parte de uma mensagem é retransmitida 3 vezes, é identificado o grupo de tramas a qual esta pertençe
byte tag_msg[1]; //Identificação da mensagem quanto à sua posição no seu grupo de fluxo
byte cauda[17];
byte tamanho_msg[1];
byte* dados; //até 255 bytes
//Variável cabeçalho -
//Variável cauda
//Variável pacote , os pacotes são adicionados na função de formar_pacote()

//Definimos o modo em que o pino escolhido como emissor vai trabalhar

byte* formular_cabecalho(int grupo_fluxo_envio2, byte tmnh_msg, byte n_envio) {
  byte sync_block[2]; //bloco de sincronização no qual 20 bits vão servir para sincronizar ambos os elementos, ou seja, ao receber uma mensagem o receptor vai começar por igualar todos os valores através de um ciclo.
  byte tipo_trama;
  sync_block[3] = '\0';
  int i = 0; //varíavel temp para rodar o bloco de sync
  int x = 0; //varíavel para fazer shifting dos bits do bloco de sync
  for (i = 0; i < 3; i++) {
    for (x = 0 + x * i; x < 8 * i; x++) {
      sync_block[i] |= ~(bloco_sync[x] << x);
    }
  }
  int option = 0; //A escolha para já é hardcoded , consoante o que for preciso é alterado, assumo que seja necessário indicar como um argumento ou então como uma questão a adicionar durante a comunicação já que a mesma não é constante
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
  byte grupo_fluxo_byte = byte(grupo_fluxo_envio2);
  memcpy(cabecalho, sync_block, 2 * sizeof(byte));
  memcpy(cabecalho + sizeof(cabecalho), (byte*)tipo_trama, sizeof(sync_block));
  memcpy(cabecalho + sizeof(cabecalho), (byte*)tmnh_msg, sizeof(tmnh_msg));
  memcpy(cabecalho + sizeof(cabecalho), (byte*)grupo_fluxo_byte, sizeof(grupo_fluxo_byte));
  memcpy(cabecalho + sizeof(cabecalho), (byte*)n_envio, sizeof(n_envio));

  return cabecalho;
}

boolean formar_enviar_pacote(byte* cabec, byte* CRC_part, byte* mensagem, int grupo_fluxo_envio) {
  int n_reenvio = 0;
  int tamanho_msg = 0;

  dados = (byte*) malloc(tamanho_msg * sizeof(byte));
  for (int o = 0; o < tamanho_msg; o++) {
    dados[o] = mensagem[o];
  }
  byte* crc = formar_CRC(dados, tamanho_msg); //Formulação do CRC ou seja da cauda da trama
  byte tamanho_msg_byte = byte(tamanho_msg);
  byte n_reenvio_byte = byte(n_reenvio);
  byte* pacote = (byte*) malloc((4 + tamanho_msg + sizeof(CRC_part)) * sizeof(byte));
  byte* cabec_pacote = formular_cabecalho(grupo_fluxo_envio, tamanho_msg_byte, n_reenvio_byte);
  memcpy(pacote, cabec_pacote, sizeof(cabec_pacote));
  memcpy(pacote + sizeof(pacote), dados, sizeof(dados));
  memcpy(pacote + sizeof(pacote), crc, sizeof(crc));
  free(pacote);
  free(dados); //TÊMOS DE LIBERTAR A MEMÓRIA EM C , NÃO SE ESQUEÇAM
}

byte* formar_CRC(byte* dados, int tamanho_dados) { //Função que realiza a formulação do CRC e devolve o resultado como uma variável de 16 bytes sem associação a um tipo de variável em específico (unsigned)
  uint16_t result16 = crcx::crc16(dados, tamanho_dados);
  byte* crc_bytes = (byte*)result16;
  return crc_bytes;
}

void setup() {
  Serial.begin(115200); //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(SendingPin, OUTPUT);
}

void loop() {
  int grupo_fluxo = 0;
  if (Serial.available() && !flag_dados_disp) {
    int tamanho_recebido = Serial.read(); //Primeiros bytes recebidos representam o tamanho da mensagem que provém da aplicação, se esta variável for igual a 0 é sinal que a aplicação deixou de ter dados para enviar e deseja parar
    if (tamanho_recebido == 0) {
      flag_dados_disp = false;
    }
    flag_leitura = true;
    flag_dados_disp = true;
    byte temp_dados[tamanho_recebido + 1];
    temp_dados[tamanho_recebido + 1] = '\0';
    while (flag_leitura) {
      //Enquanto estiver a ler e não chegar alguma notificação de stop, ele continua a ler
      if (Serial.available()) {
        Serial.readBytes(temp_dados, tamanho_recebido); //Recebemos a quantidade de bytes dita pela aplicação e guardamos num buffer, neste caso a variável temporária para o registo dos mesmos
        //Funções de envio arduino - arduino;
        Serial.write(flag_leitura); //A aplicação que recebe os dados fica à espera de receber uma flag para saber se pode enviar mais dados
        //Na parte do receptor têmos de receber os bits todos e enquanto o receptor processo aos poucos ele pode receber mais, simplesmente fica num buffer;
      }
    }
  }
}
