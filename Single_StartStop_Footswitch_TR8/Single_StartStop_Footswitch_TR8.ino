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

int readTrigger;           // value for ch1 stomp switch
int pReadTrigger = HIGH;  // previous reading from stomp switch

int readES1;            //reading for ES1 IO pin
int readES2;            //reading for ES2 IO pin

int pReadES1 = HIGH;      //previous reading for ES1 pin
int pReadES2 = HIGH;      //previous reading for ES1 pin

int BPM = 120;
char noteStr[3];    //character array of note int, used for printing
char ch1Msg[] = "BPM:"; //prefix for screen message indicating which note
char *msgPtr = new char[12]; //message displayed on screen

long time = 0;         
long debounce = 100;   //millisecond debounce
double timeDelay = 20.83; //millisecond version of 1/24th of a quarter note at 120 BPM
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
  sprintf(noteStr,"%d",BPM);
  strcpy(msgPtr, ch1Msg);
  strcat(msgPtr,noteStr);

  
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
    time = millis(); //update current time variable
  }

  
   // if ES0 (edit switch 0) engaged
   // increment midi note one half step and display screen accordingly
   ////////////////////////////////////////////////////////////////////////////
  if(readES1 == HIGH && pReadES1 == LOW && millis() - time > debounce) {
    BPM++;
    time = millis(); //update current time variable
  }

  

  pReadES1 = readES1;
  pReadES2 = readES2;
  
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
