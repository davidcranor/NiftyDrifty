/*
   StatusMonitor

   Displays formatted, dynamic, real-time Arduino status information
   using BasicTerm.

   It responds to the following input commands:
   ' '  (space):  Toggle LED
   '\f' (Ctrl-L): Redraw screen
*/

#define LED 13
#define SENSOR_PIN 0

uint32_t now;
uint32_t last;

uint32_t lastEventTime;
uint32_t currentEventTime;
uint32_t elapsedEventTime;
uint32_t lastPrintUpdate;

float currentRPM;
float currentHz;

uint8_t pinStatus;

uint16_t key;

void setup(void)
{
  //Init serial port
  Serial.begin(115200);

  //Init pins
  pinMode(LED, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  
  //Init LEDs
  digitalWrite(LED, LOW);
  
  //Init timestamp
  now = millis();

  //Set up pin change interrupt for motor hall effect
  pciSetup(0);

}

void loop(void)
{
  now = millis();

  if(now - lastPrintUpdate >= 20)
  {
    
    Serial.print(now);
    Serial.print(": ");
    Serial.print(now - currentEventTime);

    updateCurrentHz();

    Serial.print("     ");
    Serial.println(currentHz);
    

    lastPrintUpdate = now;
  }
}

void updateCurrentHz()
{
  currentHz = (float)(1/((float)elapsedEventTime/1000));
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
     
     pinStatus = digitalRead(SENSOR_PIN);
     
     digitalWrite(LED,pinStatus);

     if(pinStatus == 1)
     {
      lastEventTime = currentEventTime;
      currentEventTime = millis();
      elapsedEventTime = currentEventTime - lastEventTime;
     }
}  
