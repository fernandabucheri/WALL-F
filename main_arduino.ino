#include "Ultrasonic.h" // Biblioteca referente ao sensor ultrassônico
#include <RF24.h> // Biblioteca referente ao Módulo de Rádio Frequência - Comunicação Robô - controle
/*
2 - pino do led
3 e 4 - sensor ultrassonico

*/

// Módulo de rádio frequência 
#define pinCE  7
#define pinCSN 8

// Motores
#define pinIN1 10    //Motor 1
#define pinIN2 9     //Motor 1
#define pinIN3 6     //Motor 2
#define pinIN4 5     //Motor 2 

// Variaveis para controle da direção do robô
int pDireita  = 100;
int pEsquerda = 100;

// LED
const int led = 2;

// Variaveis para registro do estado do Joystick
struct tipoDadosRF
{
   int pot1 = 512;
   int pot2 = 512;
   boolean botao = false;
};
tipoDadosRF dadosRF;

// Inicialização dos módulos de rádio frequência 
RF24 radio(pinCE,pinCSN);
const uint64_t pipeOut = 0xE8E8F0F0E1LL; // Endereço da comunicação entre os módulos de rádio frequência

// Sensor ultrassônico
const int echoPin = 3; //PINO DIGITAL UTILIZADO PELO HC-SR04 ECHO(RECEBE)
const int trigPin = 4; //PINO DIGITAL UTILIZADO PELO HC-SR04 TRIG(ENVIA)
int distancia; //VARIÁVEL DO TIPO INTEIRO
Ultrasonic ultrasonic(trigPin,echoPin); //INICIALIZANDO OS PINOS DO ARDUINO

void setup(){
  // * Módulo de rádio frequência *
  radio.begin(); // Liga o módulo 

 // Definindo o nível (potência) de comunicação  
  radio.setPALevel( RF24_PA_LOW );     //Possibilidades: RF24_PA_MIN  / RF24_PA_LOW / RF24_PA_HIGH / RF24_PA_MAX - quanto maior a potência, mais distante a comunicação, porém demanda mais bateria (alimentar o módulo com mais corrente)
  
  // Velocidade de comunicação entre os módulos 
  radio.setDataRate( RF24_250KBPS );   // Possibilidades: RF24_250KBPS / RF24_1MBPS  / RF24_2MBPS - para longas distâncias, colocar velocidade baixa 
  
  // Inicia o canal de comunicação - canal 1 e usando o endereço na variavel pipeOut
  radio.openReadingPipe(1, pipeOut);

  // Começa a ouvir - receber informações do controle
  radio.startListening();  

  // * Sensor ultrassônico *
  pinMode(echoPin, INPUT); // Define o pino de entrada (recebe dados)
  pinMode(trigPin, OUTPUT); // Define o pino de saída (envia dados)

  // * Motor *
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);

  // * LED *
  pinMode(led, OUTPUT);

  Serial.begin(9600);
}

void loop(){
  hcsr04(); // Chama função do sensor ultrassônico

  // Controle rádio frequência 
  if (radio.available()) { // controle para saber se existe comunicação - rádio está disponível? Sim (ou seja, controle enviou alguma informação para o robô)
     radio.read(&dadosRF, sizeof(tipoDadosRF)); // Lê a informação enviada pelo controle, guardando em uma struct dadosRF
  }   
  
  // Controle da direção do robô
  if (dadosRF.botao) {
     //Aciona o freio
     digitalWrite(pinIN1, HIGH);
     digitalWrite(pinIN2, HIGH);
     digitalWrite(pinIN3, HIGH);
     digitalWrite(pinIN4, HIGH);
  } else {
     //Controle da direção 
     if (dadosRF.pot2 < 512) {
        //Esquerda 
        pDireita  = 100;
        pEsquerda = map(dadosRF.pot2, 511, 0, 100, 0); 
     } else {
        //Direita
        pDireita  = map(dadosRF.pot2, 512, 1023, 100, 0);
        pEsquerda = 100;       
     }

     if (dadosRF.pot1 < 512) {
        //Reverso
        int velocidade = map(dadosRF.pot1, 511, 0, 0, 255);

        analogWrite(pinIN1, 0);
        analogWrite(pinIN2, velocidade * pDireita / 100);
     
        analogWrite(pinIN3, 0);
        analogWrite(pinIN4, velocidade * pEsquerda / 100); 
     } else {
        //Para frente
        int velocidade = map(dadosRF.pot1, 512, 1023, 0, 255);

        analogWrite(pinIN1, velocidade * pDireita / 100);
        analogWrite(pinIN2, 0);
     
        analogWrite(pinIN3, velocidade * pEsquerda / 100);
        analogWrite(pinIN4, 0);                         
     }
  }  
}

void piscarLed(int tempo){
  digitalWrite(led, HIGH);
  delay(tempo * 2);
  digitalWrite(led, LOW);
  delay(tempo);
}

// Calcular a distância
void hcsr04(){
    digitalWrite(trigPin, LOW); // Envia um pulso LOW para o pino 6 
    delayMicroseconds(2); 
    digitalWrite(trigPin, HIGH); // Envia um pulso HIGH para o pino 6 
    delayMicroseconds(10); 
    digitalWrite(trigPin, LOW); // Envia um pulso LOW para o pino 6 
    
    // A função Ranging faz a conversão do tempo de resposta do echo - em cm
    distancia = (ultrasonic.Ranging(CM));
    
    // Pisca o LED de acordo com a distância lida 
    if (distancia <= 10){
      piscarLed(50);
    }
    else if (distancia <= 50){
      piscarLed(100);
    }
    else if (distancia <= 100){
      piscarLed(200);
    }
    else if (distancia <= 200){
      piscarLed(300);
    }
    delay(500);
    
    Serial.print("Obstáculo detectado a "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(String(distancia)); ////IMPRIME NO MONITOR SERIAL A DISTÂNCIA MEDIDA
    Serial.println(" cm de distância."); //IMPRIME O TEXTO NO MONITOR SERIAL
 }