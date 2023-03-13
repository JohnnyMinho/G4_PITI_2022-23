#include <printf.h>
#include <stdio.h>
#include "Tipos.h"
#include "CRCx.h";

#define SendingPin 14//Define o Pino pelo qual irá ser usado como pino de envio de dados
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
int grupo_fluxo = 0;
//Variável cabeçalho -
//Variável cauda
//Variável pacote , os pacotes são adicionados na função de formar_pacote()

//Definimos o modo em que o pino escolhido como emissor vai trabalhar

byte* formular_cabecalho(int grupo_fluxo_envio2, byte tmnh_msg, byte n_envio) {
  
  byte sync_block[2] = {0b11011110, 0b11000000}; //bloco de sincronização no qual 20 bits vão servir para sincronizar ambos os elementos, ou seja, ao receber uma mensagem o receptor vai começar por igualar todos os valores através de um ciclo.
  byte tipo_trama;
  int tamanho_ocupado = 0; //Variável de auxilio devido às características do C
  Serial.println(sync_block[0]);
  Serial.println(sync_block[1]);
  /*
   * int i = 0; //varíavel temp para rodar o bloco de sync
  int x = 0; //varíavel para fazer shifting dos bits do bloco de sync
   * for (i = 1; i < 3; i++) {
    for (x = 0 + x * i; x < 8 * i; x++) {
      if(x<11){
      Serial.println(bloco_sync[x]);
      sync_block[i-1] |= ~(bloco_sync[x] << x);
      }else{
        x = 8*i;
      }
    }
    if(i!=3){ 
      Serial.print("Bloco:");
      Serial.println(sync_block[i-1]);
      cabecalho[i] = sync_block[i-1];
    }
    Serial.print("x:");
    Serial.println(x);
    Serial.println(i);
  }*/
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
  memcpy(cabecalho, sync_block, 2);
  byte grupo_fluxo_byte = byte(grupo_fluxo_envio2);
  //Serial.println(grupo_fluxo_byte);
  cabecalho[2] = grupo_fluxo_byte;
  tamanho_ocupado = 3;
  //Serial.println(tipo_trama);
  cabecalho[tamanho_ocupado + 1] = tipo_trama;
  tamanho_ocupado = tamanho_ocupado + sizeof(tipo_trama);
  cabecalho[tamanho_ocupado + 1] = tmnh_msg;
  tamanho_ocupado = tamanho_ocupado + sizeof(tmnh_msg);
  cabecalho[tamanho_ocupado + 1] = grupo_fluxo_byte;
  //memcpy(cabecalho + tamanho_ocupado, (byte*)grupo_fluxo_byte, sizeof(grupo_fluxo_byte));
  tamanho_ocupado = tamanho_ocupado + sizeof(grupo_fluxo_byte);
  cabecalho[tamanho_ocupado + 1] = n_envio;
  //memcpy(cabecalho + tamanho_ocupado, (byte*)n_envio, sizeof(n_envio));
  for(int i = 0; i<4;i++){
    Serial.println(cabecalho[i]);
  }
  return cabecalho;
}

boolean formar_enviar_pacote(byte* mensagem, int grupo_fluxo_envio,int tamanho_msg) {
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
  int len = 6+tamanho_msg+2;
  byte* pacote = (byte*) malloc(len*sizeof(byte)); //confirmem os tamanhos aqui descritos e no resto das funções.
  byte* cabec_pacote = formular_cabecalho(grupo_fluxo_envio, tamanho_msg_byte, n_reenvio_byte);
  //A função de formular o cabeçalho em algum erro ao nível da memória
  memcpy(pacote, cabec_pacote, 6); //Muitos warnings neste tipo de operações, ver uma maneira mais "aceitável" de fazer este tipo de op.
  Serial.println("Helloprememcpy2");
  int tamanho_ocupado = 6;
  memcpy(pacote + tamanho_ocupado , mensagem, tamanho_msg);
  Serial.println("Helloprememcpy3");
  tamanho_ocupado = tamanho_ocupado + tamanho_msg;
  Serial.println(tamanho_ocupado);
  Serial.print("Testagem do pacote:");
  Serial.println(pacote[tamanho_ocupado+1]);
  Serial.println(crc[0]);
  pacote[tamanho_ocupado+1]=crc[0];
  pacote[tamanho_ocupado+2]=crc[1];
  //memcpy(pacote + tamanho_ocupado, crc, 2); 
  Serial.println("Helloenvio1");

  //Soluções -> Ver se falta o bit de fecho ou então mudar para um array de tamanho fixo
  
  //Falta implementar o método de envio com recurso ao digitalwrite() ler documentação e ver exemplos do digitalwrite() https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/
  //Segundo a documentação é melhor utilizar um pino de input que pussa uma resistência de pull-up (ver o porque de ser melhor)

  //------ OPERAÇÕES AO NÍVEL DO BIT, verifiquem no dia 8/3 se isto está a apresentar os valores certos no receptor (o valor HIGH equivale a 5V e o LOW a 0V).
  for (int tentativas; tentativas < 3; tentativas++) { //Acho que o que está no interiro não é a melhor maneira de fazer isto, mas revejam sff.
    byte n_reenvio_byte = byte(n_reenvio);
    byte* cabec_pacote = formular_cabecalho(grupo_fluxo_envio, tamanho_msg_byte, n_reenvio_byte);
    int tamanho_retirar = sizeof(dados) + sizeof(crc);
    memcpy(pacote - tamanho_retirar, cabec_pacote, sizeof(cabec_pacote));
    for (int x = 0; x < sizeof(pacote); x++) {
      for (int i = 0; i < 8 * sizeof(pacote); i++) {
        int bit = (pacote[x] >> i) & 0x01;   // obter o valor do bit
        if(bit == 1){
          digitalWrite(SendingPin, HIGH);    // o pino fica com o valor de 1 ou 0 , HIGH ou LOW respetivamente
        }else{
          digitalWrite(SendingPin, LOW);
        }
        delayMicroseconds(10);           // PARA A PRIMEIRA TESTAGEM COM O RECEPTOR, quando o receptor conseguir receber mensagens tentem sem este delay
      }
    }
  }
  Serial.println("Helloenvio2");
  //------
  free(pacote); //Têmos algum erro com o free
  //free(dados); //TÊMOS DE LIBERTAR A MEMÓRIA EM C , NÃO SE ESQUEÇAM
}

byte* formar_CRC(byte* dados, int tamanho_dados) { //Função que realiza a formulação do CRC e devolve o resultado como uma variável de 16 bytes sem associação a um tipo de variável em específico (unsigned)
  uint16_t result16 = crcx::crc16(dados, tamanho_dados);
  byte crc_bytes[2];
  crc_bytes[0] = (result16 >> 8) & 0xFF; //O MSB fica na primeira posição do array
  crc_bytes[1] = (result16 & 0xFF);// O LSB fica na ultima posição do array
  return crc_bytes;
} //Como a linguagem C dealoca a memória usada para a função mal esta acabe, é uma boa prática evitar que tentemos devolver arrays como um resultado.

void setup() {
  Serial.begin(115200); //Caso precisemos de usar o manchester o output verdadeiro vai ser 57600 bits por segundo
  pinMode(SendingPin, OUTPUT);
}

void loop() {
  if (Serial.available() && !flag_dados_disp) {
    flag_dados_disp = true;
    while (flag_dados_disp == true) {
      int tamanho_recebido = 5; //Serial.read(); //Primeiros bytes recebidos representam o tamanho da mensagem que provém da aplicação, se esta variável for igual a 0 é sinal que a aplicação deixou de ter dados para enviar e deseja parar
      if (tamanho_recebido == 0) {
        flag_dados_disp = false;
        grupo_fluxo--;
      }
      flag_leitura = true;
      byte temp_dados[tamanho_recebido + 1];
      temp_dados[tamanho_recebido + 1] = '\0';
      while (flag_leitura && flag_dados_disp) { //Por variavél para sair.
        grupo_fluxo++;
        //Enquanto estiver a ler e não chegar alguma notificação de stop, ele continua a ler
        if (Serial.available()){
          Serial.readBytes(&temp_dados[0], tamanho_recebido); //Recebemos a quantidade de bytes dita pela aplicação e guardamos num buffer, neste caso a variável temporária para o registo dos mesmos
          //Funções de envio arduino - arduino;
          formar_enviar_pacote(temp_dados, grupo_fluxo,tamanho_recebido); //-> Esta a dar erros de acesso de memória
          Serial.write(flag_leitura); //A aplicação que envia os dados fica à espera de receber uma flag para saber se pode enviar mais dados
          //Na parte do receptor têmos de receber os bits todos e enquanto o receptor processo aos poucos ele pode receber mais, simplesmente fica num buffer;
        }
      }
    }
  }
}
