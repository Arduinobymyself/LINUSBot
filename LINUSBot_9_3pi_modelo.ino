
// Inclusão de bibliotecas 
// Libraries include
#include <Button.h>
#include <QTRSensors.h>
#include <Wire.h>
#include <LiquidCrystal.h>

// Definição de constantes
// Constants and variables definition
#define NUMREADINGS 10 // número de leituras para tirar a média da tensão da bateria
                       // Sample numbers to take the median batery voltage

// Definção de objetos
// Objects definition
Button button = Button(2,BUTTON_PULLUP_INTERNAL); // botão no pino 2
                                                  // pin 2 - button
// sensores nos pinos analógicos 0, 1, 2, 3 e 4
// usados como digital 14, 15, 16, 17 e 18
// 5 sensores, time-out 2000, sem pino de LED

// the 5 sensor are in the analogic pins
// used as digital pins named 14, 15, 16, 17 and 18
// time-out 2000 and without LED pin
QTRSensorsRC qtr((unsigned char[]) {14, 15, 16, 17, 18}, 5, 2000, QTR_NO_EMITTER_PIN);
// display nos pinos R/W - 13, Enable - 12, dados - 9, 8, 7 e 4
// display in the pins: R/W - 13, Enable - 12, data - 9, 8, 7 e 4
LiquidCrystal lcd(13, 12, 9, 8, 7, 4);

// Definição de variáveis
// Variables definition
unsigned int sensors[5]; // Matriz para armazenar valores dos sensores
                         // Array to store the sensors values


int inA1 = 10; // Dual H-Bridge pins
int inA2 = 11;
int inB1 = 5;
int inB2 = 6;

//int pinButton = 2; // Pino do botão
                     // Button pin
int pinBatery = 5; // Pino do sensor de bateria
                   // batery sensor pin
// |GND|---/\/\/\/------/\/\/\/-----|VCC|
//           10K     |    5K
//                   |
int pinAudio = 3;  // Pino do Buzzer/Speaker
                   // Buzzer/Speaker pin


int voltage;
int readings[NUMREADINGS];
float volts = 0;
int total = 0;
float average = 0;
int index = 0;


// Executado na inicialização do Arduino
// performed at the Arduino initialization
void setup(){
  lcd.begin(16, 2);
  Serial.begin(9600);        // Inicializa a comunicação serial
                             // Initialize the serial communication
  pinMode(pinAudio, OUTPUT); // Define pino de audio como saída
                             // Define the Audio pin as an output
  set_motors(0,0); // Enquanto espera, motores permanecem parados
                   // while wait, motors are stopped
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    LINUSBot");
  lcd.setCursor(0, 1);
  lcd.print(" Line Follower ");
  delay(2000);  
  voltage = read_batery(); // Lê a tensão da bateria
                           // take the batery voltage measure
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bateria");
  lcd.setCursor(0, 1);
  lcd.print(voltage);
  lcd.print(" mV");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");
  delay(1500);
  lcd.setCursor(0, 1);
  lcd.print("OK!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pressione Botao");
  while(!button.isPressed()){    
  }
  delay(500); // Atraso para dar tempo de tirar o dedo do botão
  // Sempre espere por um botão ser pressionado antes de que
  // seu robot possa iniciar a movimentação
  
  // Delay to allow time to take your finger off the button
  // Always wait for a button pressed before your robot
  // get in movement 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Auto-calibracao");
  //Serial.println("Auto-calibracao");
  // Auto-calibração: gira para a direita e depois esquerda e volta ao início
  // calibrando os sensores
  // Autocalibration: turns to right and after to left and returns to the start position
  // sensors' calibration
  unsigned int counter; // usado como um simples contador
                        // used as a simple counter
  for(counter=0; counter<80; counter++){
    if(counter < 20 || counter >= 60){
      set_motors(50,-50); // Gira para a direita
                          // turn to right
    }
    else{
      set_motors(-50,50); // Gira para a esquerda
                          // turn to left
    }
    // Esta função armazena um conjunto de leituras dos sensores, e mantém
    // informações sobre o máximo e mínimo valores encontrados
    // This function stores a set of reads of the sensors and 
    // keep information about the maximum and minimum values found.
    qtr.calibrate();
    // Desde que contamos até 80, o total do tempo de calibração será de 80 * 10 = 800 ms
    // Since w count till 80, the whole time of calibration will be: 80 * 10 = 800 ms
    delay(10);
  }
  set_motors(0,0); // Garante motores parados após o processo de calibração
                   // ensures that the motors are stopped after calibration proccess
  
  lcd.setCursor(0, 1);
  lcd.print("Calibrado!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pressione botao");
  
  // Enquanto botão não é pressionado mostra a posição da linha em relação aos sensores no console serial usado para debug
  // While button is not pressed shows the position of the line relative to the sensors; used to debug via serial console.
  /*
  while(!button.isPressed()){
    unsigned int position = qtr.readLine(sensors);
    Serial.println(position);
  }
  */
}


// Esta é a função principal, onde o código inicia. Todo programa Arduino
// deve ter uma função loop() definida em algum lugar
// This is the main function, where the code begin. All Arduino code
// need to have a loop() function defined anywhere
void loop(){
  while(!button.isPressed()){
  }
  delay(200);
  playMusic();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Go! go! go! ...");
  delay(500);
  // Este é o loop principal - irá rodar para sempre
  while(1){
    // Obtém a posição da linha
    // Aqui não estamos interessados nos valores individuais de cada sensor
    unsigned int position = qtr.readLine(sensors);
    
    /*
    if (sensors[0] > 750 && sensors[1] > 750 && sensors[2] > 750 && sensors[3] > 750 && sensors[4] > 750){
      // Se a posição individual de cada sensor estiver nestes limiares
      // pode ser que o robot tenha encontrado um fin de curso,
      // ou esteja a beira de uma mesa prestes a cair,
      // neste caso devemos fazer algo,.. quem sabe parar os motores, dar ré
      // girar 180 graus, e assim continuar.
      set_motors(0,0); // para motores
      set_motors(-50,-50); // vai para tras
      return;
    }
    */
    // Verificamos a posição da linha e ajustamos as velocidades dos motores
    // e direção de acordo com a necessidade
    // A posição da linha é determinado da seguinte forma;
    // 0*valor0+1000*valor1+2000*valor2+.....
    // Quando totalmente a esquerda o valor será 0,
    // quando exatamente no meio (sensor2) o valor é 2000... e assim por diante
    if(position < 500){ // Linha totalmente a esquerda do robot
      set_motors(0,100); // Giro a esquerda acentuado para corrigir
    }
    // Linha entre o sensor 0 e 1
    else if(position >= 500 && position < 1500){
      set_motors(50,100); // Giro a esquerda suave para corrigir
    }
    // Linha entre o sensor 1 e 3, incluindo o 2
    // quase meio
    else if(position >= 1500 && position < 2500){
      set_motors(100,100); // Segue em frente velocidade máxima
    }
    // Linha ente o sensor 3 e 4
    else if(position >= 2500 && position < 3500){
      set_motors(100,50); // Giro a direita suave para corrigir
    }
    // Linha totalmente a direta do robot
    else{
      set_motors(100,0); // Giro a direita acentuado para corrigir
    }
  }
  // Esta parte do código, nunca deve ser alcançado. O robot nunca deve
  // chegar ao fim do programa, ou comprotamentos imprevisíveis 
  // podem ocorrer e partes aleatórias do código podem ser executadas.
  // Se você realmente deseja parar todas as ações neste ponto, 
  // configure os motores para 0,0 e rode um comando while como loop infinito
  // caso não queira, comente as linhas abaixo
  set_motors(0,0);
  while(1);
}




// Acionamento dos motores
void set_motors(int left_speed, int right_speed){
  if(right_speed >= 0 && left_speed >= 0){
    analogWrite(inA1, 0);
    analogWrite(inA2, right_speed);
    analogWrite(inB1, 0);
    analogWrite(inB2, left_speed);
  }
  if(right_speed >= 0 && left_speed < 0){
    left_speed = -left_speed;
    analogWrite(inA1, 0);
    analogWrite(inA2, right_speed);
    analogWrite(inB1, left_speed);
    analogWrite(inB2, 0);
  }
  if(right_speed < 0 && left_speed >= 0){
    right_speed = -right_speed;
    analogWrite(inA1, right_speed);
    analogWrite(inA2, 0);
    analogWrite(inB1, 0);
    analogWrite(inB2, left_speed);
  } 
}


// Verifica tensão da bateria
unsigned int read_batery(){
  // Tirando a média
  for (int k = 0; k < NUMREADINGS; k++){ // Zerando a matriz
    readings[k] = 0;
  }
  total -= readings[index]; // Inicializa total
  readings[index] = analogRead(pinBatery); // Faz a leitura do sensor
  total += readings[index]; // Soma valores da matriz
  index = (index + 1); // Próximo valor
  if (index >= NUMREADINGS){ // Verifica se chegou ao fim das leituras
    index = 0; // Se sim zera o índice
  }
  average = total / NUMREADINGS; // Média simples
  //Serial.println(average); // Para debug, se necessário
  // Agora calculamos a tensão baseado na leitura analógica média
  volts = average * 5000 * 3 / 2 / 1023 * 10;
  return (volts); // Retorna com o valor
}

// Toca música e sons
void playMusic(){
  // Definição de frequência das notas
  #define NOTE_B0  31
  #define NOTE_C1  33
  #define NOTE_CS1 35
  #define NOTE_D1  37
  #define NOTE_DS1 39
  #define NOTE_E1  41
  #define NOTE_F1  44
  #define NOTE_FS1 46
  #define NOTE_G1  49
  #define NOTE_GS1 52
  #define NOTE_A1  55
  #define NOTE_AS1 58
  #define NOTE_B1  62
  #define NOTE_C2  65
  #define NOTE_CS2 69
  #define NOTE_D2  73
  #define NOTE_DS2 78
  #define NOTE_E2  82
  #define NOTE_F2  87
  #define NOTE_FS2 93
  #define NOTE_G2  98
  #define NOTE_GS2 104
  #define NOTE_A2  110
  #define NOTE_AS2 117
  #define NOTE_B2  123
  #define NOTE_C3  131
  #define NOTE_CS3 139
  #define NOTE_D3  147
  #define NOTE_DS3 156
  #define NOTE_E3  165
  #define NOTE_F3  175
  #define NOTE_FS3 185
  #define NOTE_G3  196
  #define NOTE_GS3 208
  #define NOTE_A3  220
  #define NOTE_AS3 233
  #define NOTE_B3  247
  #define NOTE_C4  262
  #define NOTE_CS4 277
  #define NOTE_D4  294
  #define NOTE_DS4 311
  #define NOTE_E4  330
  #define NOTE_F4  349
  #define NOTE_FS4 370
  #define NOTE_G4  392
  #define NOTE_GS4 415
  #define NOTE_A4  440
  #define NOTE_AS4 466
  #define NOTE_B4  494
  #define NOTE_C5  523
  #define NOTE_CS5 554
  #define NOTE_D5  587
  #define NOTE_DS5 622
  #define NOTE_E5  659
  #define NOTE_F5  698
  #define NOTE_FS5 740
  #define NOTE_G5  784
  #define NOTE_GS5 831
  #define NOTE_A5  880
  #define NOTE_AS5 932
  #define NOTE_B5  988
  #define NOTE_C6  1047
  #define NOTE_CS6 1109
  #define NOTE_D6  1175
  #define NOTE_DS6 1245
  #define NOTE_E6  1319
  #define NOTE_F6  1397
  #define NOTE_FS6 1480
  #define NOTE_G6  1568
  #define NOTE_GS6 1661
  #define NOTE_A6  1760
  #define NOTE_AS6 1865
  #define NOTE_B6  1976
  #define NOTE_C7  2093
  #define NOTE_CS7 2217
  #define NOTE_D7  2349
  #define NOTE_DS7 2489
  #define NOTE_E7  2637
  #define NOTE_F7  2794
  #define NOTE_FS7 2960
  #define NOTE_G7  3136
  #define NOTE_GS7 3322
  #define NOTE_A7  3520
  #define NOTE_AS7 3729
  #define NOTE_B7  3951
  #define NOTE_C8  4186
  #define NOTE_CS8 4435
  #define NOTE_D8  4699
  #define NOTE_DS8 4978

  // Definição de músicas
  int note[] = {NOTE_C4, NOTE_C4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5};
  int duration[] = {100, 100, 100, 300, 100, 300};

  int starttune[] = {NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_F4, NOTE_G4};
  int duration2[] = {100, 200, 100, 200, 100, 400, 100, 100, 100, 100, 200, 100, 500};

  int error[] = {NOTE_G3, NOTE_C3, NOTE_G3, NOTE_C3, NOTE_G3, NOTE_C3, NOTE_G3, NOTE_C3};
  int duration3[] = {100, 200, 100, 200, 100, 200, 100, 200};
  
  // Loop com a quandiade de notas a serem tocadas
  // Se necessário, mude o valor interno do loop "for"
  for(int i=0;i<6;i++){
     tone(pinAudio, note[i], duration[i]); // Emite o som nota/duração
     delay(duration[i]);                   // Pausa do som
     noTone(pinAudio);                     // Finaliza som
  }
}

