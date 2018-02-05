/*
   StatusMonitor

   Displays formatted, dynamic, real-time Arduino status information
   using BasicTerm.

   It responds to the following input commands:
   ' '  (space):  Toggle LED
   '\f' (Ctrl-L): Redraw screen
*/

#include <BasicTerm.h>

#define LED 13

BasicTerm term(&Serial);

uint32_t now;
uint32_t last;

uint32_t lastEventTime;
uint32_t currentEventTime;
uint32_t elapsedEventTime;
uint32_t lastPrintUpdate;

uint8_t pinStatus;

uint16_t currentRPM;

uint16_t key;

void setup(void)
{
  //Init serial port
  Serial.begin(115200);

  //Init pins
  pinMode(LED, OUTPUT);

  uint8_t i;

  for (i = A0; i <= A5; i++) {
    pinMode(i, INPUT);
  }

  for (i = 2; i <= 12; i++) {
    pinMode(i, INPUT);
  }

  //Init LEDs
  digitalWrite(LED, LOW);


  //Init terminal
  term.init();
  term.cls();
  term.show_cursor(false);

  term.position(0, 0);
  term.set_attribute(BT_NORMAL);
  term.set_attribute(BT_BOLD);
  term.print(F("NiftyDrifty LED Controller v0.1"));
  
  //Init timestamp
  now = millis();

  //Set up pin change interrupt for motor hall effect
  pciSetup(0);

}

void loop(void)
{
  
  //Show time since program start
  now  = millis();

  if(now - lastPrintUpdate >= 20)
  {
    term.position(0, 40);
    term.set_attribute(BT_NORMAL);
    term.set_color(BT_RED, BT_BLACK);
  
    term.print(now);

    //Get and show current rpm
    currentRPM = getCurrentRPM();
  
    term.position(1, 0);
    term.set_attribute(BT_NORMAL);
    term.set_color(BT_GREEN, BT_BLACK);
    term.print(F("0000"));

    term.position(1, 0);
    term.print(now - lastEventTime);   

    //Check for key input
    key = term.get_key();
    switch (key) {
      case ' ':
      /* Spacebar: toggle LED */
      digitalWrite(LED, !digitalRead(LED));
      break;
    case '\f':
      /* Ctrl-L: redraw screen */
      term.cls();
      break;
    }
  }
}

//Function for calculating RPM from time between last hall effect events
uint16_t getCurrentRPM()
{
  return 1337;
}

//Set up pin change interrupts
void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

//Pin change interrupt for hall effect sensors on motor
ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7 here
{
     
     pinStatus = digitalRead(0);
     
     digitalWrite(LED,pinStatus);

     if(pinStatus == 0)
     {
      lastEventTime = millis();
     }
}  

