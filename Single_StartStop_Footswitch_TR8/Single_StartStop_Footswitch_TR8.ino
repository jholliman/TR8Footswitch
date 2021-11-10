/* midi switch
 * sends midi command based on switch input
 */
#include <Wire.h>
#include "ACROBOTIC_SSD1306.h"

unsigned long lastInterupt = 0;

int LEDPin = 13;       // LED output pin
int stateLED = LOW; //current state of LED

int triggerPin = 2;       // switch input pin 
int ES1 = 3;              //pin for incrementing midi note # (or midi status byte)
int ES2 = 4;              //pin for decrementing midi note # (or midi status byte)

int readES1;            //reading for ES1 IO pin
int readES2;            //reading for ES2 IO pin

int pReadES1 = HIGH;      //previous reading for ES1 pin
int pReadES2 = HIGH;      //previous reading for ES1 pin

int BPM = 120;
char ch1Msg[30];
char *msgPtr = new char[12]; //message displayed on screen


long time = 0;         
long debounce = 100;   //millisecond debounce
float timeDelay = 20.95; //millisecond version of 1/24th of a quarter note at 120 BPM
int intTimeDelay = timeDelay * 100;
bool play = false;

void setup()
{
  Wire.begin();  
  oled.init(); 
  
  pinMode(LEDPin, OUTPUT);
  pinMode(triggerPin, INPUT);
  pinMode(ES1, INPUT);
  pinMode(ES2, INPUT);
  attachInterrupt(digitalPinToInterrupt(triggerPin), footswitchInterupt, RISING);
  
  oled.clearDisplay();              
  oled.setTextXY(0,0);             
  oled.putString("initializing");
  delay(1500);
  Serial.begin(31250);// Set MIDI baud rate:
  oled.setTextXY(1,0);             
  oled.putString("init midi port");
  delay(1500);
  oled.clearDisplay();
}

void loop()
{

  sprintf(ch1Msg,"TimeDelay: %d",intTimeDelay);
  strcpy(msgPtr, ch1Msg);
  //strcat(msgPtr,noteStr);

  
  oled.setTextXY(0,0);
  oled.putString(msgPtr);  
    
  readES1 = digitalRead(ES1);
  readES2 = digitalRead(ES2);
  
 while(play==true){
    sendStart();
    sendTimeClock();
    delay(timeDelay);
  }
  
 
   //if ES1 (edit switch 1) engaged
   // decrement midi note one half step and display screen accordingly
   ///////////////////////////////////////////////////////////////////////////
  if (readES2 == HIGH && pReadES2 == LOW && millis() - time > debounce) {
    BPM--;
    timeDelay = updateMidiTimeDelay(BPM);
    time = millis(); //update current time variable
  }

  
   // if ES0 (edit switch 0) engaged
   // increment midi note one half step and display screen accordingly
   ////////////////////////////////////////////////////////////////////////////
  if(readES1 == HIGH && pReadES1 == LOW && millis() - time > debounce) {
    BPM++;
    timeDelay = updateMidiTimeDelay(BPM);
    time = millis(); //update current time variable
  }

  

  pReadES1 = readES1;
  pReadES2 = readES2;
  
}

float updateMidiTimeDelay(int BPM){
  float newTime = BPM/6;
  intTimeDelay = newTime * 100;
   return newTime;
}

void sendStart(){
  Serial.write(251);
}
void sendStop(){
  Serial.write(252);
}
void sendTimeClock(){
  Serial.write(248);
}
void footswitchInterupt(){

  if (millis() - lastInterupt > 400) {
   
    play = !play;
    lastInterupt = millis(); //update current time variable
  }

}
