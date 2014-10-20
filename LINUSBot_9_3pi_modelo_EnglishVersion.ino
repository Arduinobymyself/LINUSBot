
// Libraries include
#include <Button.h>
#include <QTRSensors.h>
#include <Wire.h>
#include <LiquidCrystal.h>


// Constants and variables definition
#define NUMREADINGS 10 // Sample numbers to take the median batery voltage


// Objects definition
Button button = Button(2,BUTTON_PULLUP_INTERNAL); // pin 2 - button
// the 5 sensor are in the analogic pins
// used as digital pins named 14, 15, 16, 17 and 18
// time-out 2000 and without LED pin
QTRSensorsRC qtr((unsigned char[]) {14, 15, 16, 17, 18}, 5, 2000, QTR_NO_EMITTER_PIN);
// display in the pins: R/W - 13, Enable - 12, data - 9, 8, 7 e 4
LiquidCrystal lcd(13, 12, 9, 8, 7, 4);

// Variables definition
unsigned int sensors[5]; // Array to store the sensors values


int inA1 = 10; // Dual H-Bridge pins
int inA2 = 11;
int inB1 = 5;
int inB2 = 6;

//int pinButton = 2; // Button pin
int pinBatery = 5;   // batery sensor pin
// |GND|---/\/\/\/------/\/\/\/-----|VCC|
//           10K     |    5K
//                   |
int pinAudio = 3;  // Buzzer/Speaker pin


int voltage;
int readings[NUMREADINGS];
float volts = 0;
int total = 0;
float average = 0;
int index = 0;

// performed at the Arduino initialization
void setup(){
  lcd.begin(16, 2);
  Serial.begin(9600);        // Initialize the serial communication
  pinMode(pinAudio, OUTPUT); // Define the Audio pin as an output
  set_motors(0,0); // while wait, motors are stopped
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    LINUSBot");
  lcd.setCursor(0, 1);
  lcd.print(" Line Follower ");
  delay(2000);  
  voltage = read_batery(); // take the batery voltage measure
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
  delay(500); // Delay to allow time to take your finger off the button
              // Always wait for a button pressed before your robot
              // get in movement 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Auto-calibracao");
  //Serial.println("Auto-calibracao");
  // Autocalibration: turns to right and after to left and returns to the start position
  // sensors' calibration
  unsigned int counter; // used as a simple counter
  for(counter=0; counter<80; counter++){
    if(counter < 20 || counter >= 60){
      set_motors(50,-50); // turn to right
    }
    else{
      set_motors(-50,50); // turn to left
    }
    // This function stores a set of reads of the sensors and 
    // keep information about the maximum and minimum values found.
    qtr.calibrate();
    // Since w count till 80, the whole time of calibration will be: 80 * 10 = 800 ms
    delay(10);
  }
  set_motors(0,0); // ensures that the motors are stopped after calibration proccess
  
  lcd.setCursor(0, 1);
  lcd.print("Calibrado!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pressione botao");
  
  // While button is not pressed shows the position of the line relative to the sensors; used to debug via serial console.
  /*
  while(!button.isPressed()){
    unsigned int position = qtr.readLine(sensors);
    Serial.println(position);
  }
  */
}


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
  // this is the main loop - it will run forever
  while(1){
    // Get the position of the line.  Note that we *must* provide
    // the "sensors" argument to read_line() here, even though we
    // are not interested in the individual sensor readings.
    unsigned int position = qtr.readLine(sensors);
    
    /*
    if (sensors[0] > 750 && sensors[1] > 750 && sensors[2] > 750 && sensors[3] > 750 && sensors[4] > 750){
      // If the individual position of each sensor is in these thresholds
      // May be that the robot has found a end course,
      // Or is on the verge of a table about to fall,
      // In this case we should do something .. maybe stop the engines, reverse or
      // Rotate 180 degrees, and so on.
      set_motors(0,0); // motors stopped
      set_motors(-50,-50); // reverse
      return;
    }
    */
    
    // We check the line position and adjusting the motors speeds
    // And direction, according to the need
    // The position of the line is determined as follows;
    // 0*valor0+1000*valor1+2000*valor2+.....
    // When fully left the value is 0,
    // When exactly in the middle (sensor2) value is 2000 ... and so on
    if(position < 500){ // The line is totaly at the left of the robot
      set_motors(0,100); // turns to left (sharp) to correct
    }
    // The line is between sensors 0 and 1
    else if(position >= 500 && position < 1500){
      set_motors(50,100); // turns to left (soft) to correct
    }
    // The line is between sensors 1 and 3, including sensor 2
    // almost the middle way
    else if(position >= 1500 && position < 2500){
      set_motors(100,100); // walk forward with maximum speed
    }
    // The line is between sensors 3 and 4
    else if(position >= 2500 && position < 3500){
      set_motors(100,50); // turns to right (soft) to correct
    }
    // The line is totaly at the right of the robot
    else{
      set_motors(100,0); // turns to right (sharp) to correct
    }
  }
  // This part of the code is never reached.  A robot should
    // never reach the end of its program, or unpredictable behavior
    // will result as random code starts getting executed.  If you
    // really want to stop all actions at some point, set your motors
    // to 0,0 and run the following command to loop forever:
  set_motors(0,0);
  while(1);
}




// Motors actuation
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


// Batery check
unsigned int read_batery(){
  // Median
  for (int k = 0; k < NUMREADINGS; k++){ // zeroing the array of values
    readings[k] = 0;
  }
  total -= readings[index]; // Initialize the total amount
  readings[index] = analogRead(pinBatery); // reads the sensor
  total += readings[index]; // sum values
  index = (index + 1); // next value
  if (index >= NUMREADINGS){ // check if is the end of reads
    index = 0; // if yes, zeroing the index
  }
  average = total / NUMREADINGS; // simple median
  //Serial.println(average); // just for debug via serial console
  // Now we workout the voltage value based in the analog read and median
  volts = average * 5000 * 3 / 2 / 1023 * 10;
  return (volts); // Returns with the value
}

// Plays sounds and music
void playMusic(){
  // Notes frequency definition
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

  // Musics definition
  int note[] = {NOTE_C4, NOTE_C4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5};
  int duration[] = {100, 100, 100, 300, 100, 300};

  int starttune[] = {NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_C4, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_F4, NOTE_G4};
  int duration2[] = {100, 200, 100, 200, 100, 400, 100, 100, 100, 100, 200, 100, 500};

  int error[] = {NOTE_G3, NOTE_C3, NOTE_G3, NOTE_C3, NOTE_G3, NOTE_C3, NOTE_G3, NOTE_C3};
  int duration3[] = {100, 200, 100, 200, 100, 200, 100, 200};
  
  // Loop with the amount of notes to be played
  // If needed, change the internal FOR loop value
  for(int i=0;i<6;i++){
     tone(pinAudio, note[i], duration[i]); // emits the sound note/duration
     delay(duration[i]);                   // pause
     noTone(pinAudio);                     // ending 
  }
}

