/* midi switch
 * sends midi command based on switch input
 */
#include <Wire.h>
#include "ACROBOTIC_SSD1306.h"

#define CLK 9
#define DT 10
#define SW 11
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

int currentStateCLK;
int lastStateCLK;

//unsigned long lastMidiMsg = 0;
long unsigned lastUpdateOled;
long time = 0;         
long debounce = 100;   //millisecond debounce
float clockMsgDelay = 20.95; //millisecond version of 1/24th of a quarter note at 120 BPM
int intTimeDelay = clockMsgDelay * 100;
bool play = false;

void setup()
{
  Wire.begin();  
  oled.init(); 
  
  pinMode(LEDPin, OUTPUT);
  pinMode(triggerPin, INPUT);
  //pinMode(ES1, INPUT);
  //pinMode(ES2, INPUT);
  attachInterrupt(digitalPinToInterrupt(triggerPin), footswitchInterupt, RISING);

  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);//push button


  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
  
  oled.clearDisplay();              
  oled.setTextXY(0,0);             
  oled.putString("initializing");
  delay(500);
  Serial.begin(31250);// Set MIDI baud rate:
  oled.setTextXY(1,0);             
  oled.putString("init midi port");
  delay(500);
  oled.clearDisplay();
}

void loop()
{

  //sprintf(ch1Msg,"TimeDelay: %d",BPM);
  //strcpy(msgPtr, ch1Msg);
  //strcat(msgPtr,noteStr);

  
  //oled.setTextXY(0,0);
  //oled.putString(msgPtr);  
/*    
  readES1 = digitalRead(ES1);
  readES2 = digitalRead(ES2);
*/  
 while(play==true){
  //if(millis() - lastMidiMsg > clockMsgDelay){
      
      //lastMidiMsg = millis();
 // }   
  sendStart();
     sendTimeClock();
     delay(clockMsgDelay);
  }


  //read rotary encoder
  ////////////////////////////////
    // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1){
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      BPM --;
    } else {
      // Encoder is rotating CW so increment
      BPM ++;
    }
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  
  if (millis() - lastUpdateOled > 200){
    oled.setTextXY(0,0);
    sprintf(ch1Msg,"BPM: %d  ", BPM);
    oled.putString((const char *)&ch1Msg);  
    updateMidiTimeDelay();
    lastUpdateOled = millis();
  }

  delay(2);

  
}
void updateMidiTimeDelay(){
  
  clockMsgDelay = ((1/(static_cast<float>(BPM)/60))/24)*1000;//length of one beat in Seconds. (length of one quarter note)

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
