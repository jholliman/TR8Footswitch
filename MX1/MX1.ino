/* midi switch
 * sends midi command based on switch input
 */
#include <Wire.h>
#include "ACROBOTIC_SSD1306.h"

#define CLK 9
#define DT 10
#define SW 11
#define LINE_LENGTH 30
#define MENU_LENGTH 3
unsigned long lastInterupt = 0;

int LEDPin = 13;       // LED output pin
int stateLED = LOW; //current state of LED

int triggerPin = 2;       // stomp switch input pin 

int lastButtonPress;
char ln1Msg[LINE_LENGTH]; //line 0 mainly for debugging
char ln2Msg[LINE_LENGTH];  //character array for line 1 msg 
char ln3Msg[LINE_LENGTH];  //character array for line 2 msg 
char ln4Msg[LINE_LENGTH];  //character array for line 3 msg 

int currentStateCLK;
int lastStateCLK;

long selectionCounter = 300;//value edited by rotary encoder
int currentSelection = 0; //selectionCounter modulus 3 (because there are 3 menu choices)
int currentMenu = 0; //index of what menu we are currently in
bool menuChanged = false; //used to know if we need to clear screen in response to menu changed
int menuLens[4] = {3,2,2,2}; //used to know length of menu for placing cursor

long unsigned lastUpdateOled;//when screen was update last
long unsigned lastRotate=0;//time in millis of last rotate, for debouncing rotary encoder
int BPM = 120;
float clockMsgDelay = 20.95; //millisecond version of 1/24th of a quarter note at 120 BPM
int intTimeDelay = clockMsgDelay * 100;
bool play = false;

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
  oled.setTextXY(1,0);             
  oled.putString("init midi port");
  delay(500);
  oled.clearDisplay();
}

void loop()
{
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
  if(millis()-lastRotate > 40){
    // Read the current state of CLK
    currentStateCLK = digitalRead(CLK);
  
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1 ){
      if (digitalRead(DT) != currentStateCLK) {// If the DT state is different than the CLK state then
        selectionCounter ++;// the encoder is rotating CCW so increment
        lastRotate = millis();
      } else {
        selectionCounter --;// Encoder is rotating CW so decrement
        lastRotate = millis();
      }
    }
  lastStateCLK = currentStateCLK;  // Remember last CLK state
  currentSelection = (selectionCounter % menuLens[currentMenu]) + 1;//modulus by the length of the menu we are currently in
  }

  // Read the button state
  ////////////////////////////////
  int btnState = digitalRead(SW);
  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    
    if (millis() - lastButtonPress > 50) {
      
      //if user is not at main menu and picks 1 (back), return to main menu
      if (currentMenu > 0 && currentSelection == 1){
        currentMenu = 0;
      }

      else{
        currentMenu = currentSelection;
      }
      menuChanged = true;
    }
    // Remember last button press event
    lastButtonPress = millis();
    
  }

 //here is where we update the screen, dependent on menu choice 
 /////////////////////////////////////////////////////////////////
  if (millis() - lastUpdateOled > 200){//only update screen every 200 ms
    
    //some debug stuff
    //oled.setTextXY(1,0);
    //sprintf(ln1Msg,"select: %d", currentSelection);
    //oled.putString((const char *)&ln1Msg);

    //main menu
    ///////////
    if (currentMenu == 0)
    {
      if(menuChanged == true){
        oled.clearDisplay();
        menuChanged = !menuChanged;
      }
      
      
      oled.setTextXY(2,0);
      sprintf(ln2Msg,"Drum machine   ");
      oled.putString((const char *)&ln2Msg);  
  
      oled.setTextXY(3,0);
      sprintf(ln3Msg,"Hold pitch      ");
      oled.putString((const char *)&ln3Msg);  

        
      oled.setTextXY(4,0);
      sprintf(ln4Msg,"about          ");
      oled.putString((const char *)&ln4Msg);  

      oled.setTextXY((currentSelection+1),13);
      oled.putString("-");
      lastUpdateOled = millis();
    }

    
    //menu for drum machine mode
    //////////
    if(currentMenu == 1){
      if(menuChanged == true){
        oled.clearDisplay();
        menuChanged = !menuChanged;
      }
      
      oled.setTextXY(2,0); 
      sprintf(ln2Msg,"back           ");
      oled.putString((const char *)&ln2Msg);  
      
      oled.setTextXY(3,0);
      sprintf(ln3Msg,"BPM: %d       ", BPM);
      oled.putString((const char *)&ln3Msg);  
       
      oled.setTextXY((currentSelection+1),13);
      oled.putString("-");
      lastUpdateOled = millis();
    }


    //menu for pitch hold mode
    //////////
    if(currentMenu == 2){
      if(menuChanged == true){
        oled.clearDisplay();
        menuChanged = !menuChanged;
      }
      oled.setTextXY(2,0);
      sprintf(ln2Msg,"back            ");
      oled.putString((const char *)&ln2Msg);  
      
      oled.setTextXY(3,0);
      sprintf(ln3Msg,"hold pitch mode      ");
      oled.putString((const char *)&ln3Msg);  
      
      oled.setTextXY((currentSelection+1),13);
      oled.putString("-");
      lastUpdateOled = millis();
    }

    //menu for about (kind of just an excuse for another menu and a junkyard)
    ////////
    if(currentMenu == 3){
      if(menuChanged == true){
        oled.clearDisplay();
        menuChanged = !menuChanged;
      }
      oled.setTextXY(2,0);
      sprintf(ln2Msg,"back           ");
      oled.putString((const char *)&ln2Msg);  
      
      oled.setTextXY(3,0);
      sprintf(ln3Msg,"firmware v1.0  ");
      oled.putString((const char *)&ln3Msg);  

      oled.setTextXY((currentSelection+1),13);
      oled.putString("-");
      lastUpdateOled = millis();
    }
  }


  
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
