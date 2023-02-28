#include <printf-h>
#include <stdio.h>

#define SendingPin //Define o Pino pelo qual irá ser usado como pino de envio de dados
//Um pino que não têm nenhum informação importante para qualquer protocolo é o pino 32, um que apenas têm como funções extra ser touch sensitive

//Variável cabeçalho
//Variável cauda
//Variável pacote , os pacotes são adicionados na função de formar_pacote()



byte* formular_cabecalho(){
  
}

byte* formular_cauda(){
  
}

byte* formar_pacote(){
  
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

}
