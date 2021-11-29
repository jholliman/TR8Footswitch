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


double BPM = 120.00;
char ch1Msg[30];
char ch2Msg[20];
char *msgPtr = new char[12]; //message displayed on screen

int currentStateCLK;
int lastStateCLK;

int currentStateSW;
int lastStateSW;

long unsigned lastUpdateOled;
long lastRotate;
float clockMsgDelay = 20.95; //millisecond version of 1/24th of a quarter note at 120 BPM
int intTimeDelay = clockMsgDelay * 100;
bool play = false;
bool fineEdit = false;
double BPMdecimal;//this is a hack to display doubles on screen
void setup()
{
  Wire.begin();  
  oled.init(); 
  
  pinMode(LEDPin, OUTPUT);
  pinMode(triggerPin, INPUT);

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
  //Serial.begin(9600);// for serial printing debug stuff
  oled.setTextXY(1,0);             
  oled.putString("init midi port");
  delay(500);
  oled.clearDisplay();
}

void loop()
{

  
 while(play==true){
 
     sendStart();
     sendTimeClock();
     delay(clockMsgDelay);
  }

  //read push button
  //////////////////
  currentStateSW = digitalRead(SW);
  if(currentStateSW != lastStateSW && currentStateSW == 1){
    fineEdit = !fineEdit;//toggle the fine/coarse editing mode
  }
  lastStateSW = currentStateSW;

  
  //read rotary encoder
  ////////////////////////////////
    // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);
  
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1 && (millis()-lastRotate) > 40 ){
    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      
      // Encoder is rotating CW so increment
        if(fineEdit == true)
        {
          BPM = BPM + 0.1;
        }
        else
        {
          BPM ++;
        }
      lastRotate = millis();
    } 
    else {
      
      if(fineEdit == true)
      {
        BPM = BPM - 0.1;
      }
      else
      {
        BPM --;
      }
      lastRotate = millis();

    }
  }
  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  
  if (millis() - lastUpdateOled > 200){

    //can't find a way to print floats/doubles to the SSD1306, 
    //so everything to the right of the decimal place is another int
    
    BPMdecimal = (BPM - floor(BPM))*100;//values to right of decimal place, times 100 to make integer looking

    sprintf(ch1Msg,"BPM: %d.%d", static_cast<int>(BPM),static_cast<int>(BPMdecimal));

    
    if (fineEdit == true){
      sprintf(ch2Msg, "fine adj mode  ");
    }
    else{
      sprintf(ch2Msg, "coarse adj mode");
    }
    oled.setTextXY(1,0);
    oled.putString("MIDI CLOCK OUT");  
    oled.setTextXY(2,0);
    oled.putString((const char *)&ch1Msg);  
    oled.setTextXY(4,0);
    oled.putString((const char *)&ch2Msg);  

    updateMidiTimeDelay();
    lastUpdateOled = millis();
  }

  delay(2);

  
}
void updateMidiTimeDelay(){
  
  clockMsgDelay = ((1/(static_cast<double>(BPM)/60))/24)*1000;//length of one beat in Seconds. (length of one quarter note)

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
