#include <BasicTerm.h>

//UI update timing, in microseconds (makes it less confusing to update currentTime everywhere)
#define UI_UPDATE_RATE 50000
#define SENSOR_TICKS_SAMPLE_SIZE 5
#define RPM_TIMEOUT 1000000

BasicTerm term(&Serial);

volatile uint32_t sensorTicks;
volatile unsigned long RPMTimeoutCounter;

uint32_t currentRPM;
uint32_t totalRevs;

unsigned long lastRPMUpdateTime;
unsigned long lastUIUpdateTime;

unsigned long currentTime;


void setup()
{
  //Begin serial comm
  Serial.begin(115200);

  //Set up pin change interrupt for hall effect
  //https://www.arduino.cc/en/Reference/attachInterrupt
  attachInterrupt(digitalPinToInterrupt(2), updateTicks, FALLING);

  //Initalize variables
  sensorTicks = 0;
  currentRPM = 0;
  lastRPMUpdateTime = 0;
  lastUIUpdateTime = 0;

  totalRevs = 0;

  //Init terminal
  term.init();
  term.cls();
  term.show_cursor(false);

}

void loop()
{
  currentTime = micros();

  //Update readout and LEDs
  if (currentTime - lastUIUpdateTime >= UI_UPDATE_RATE)
  {

    //Update terminal readout
    updateTerminal();

    //Reset the clock
    lastUIUpdateTime = currentTime;

    //If it's been more than a second since the sensor triggered, the wheel is stopped
    if (RPMTimeoutCounter > RPM_TIMEOUT / UI_UPDATE_RATE)
    {
      currentRPM = 0;
    }

    RPMTimeoutCounter++;
  }

  //Update RPM measurement
  if (sensorTicks >= SENSOR_TICKS_SAMPLE_SIZE)
  {
    //Stop interrupts
    noInterrupts();

    //Update current time
    currentTime = micros();

    //Update RPM, assuming 20 hall effect event per revolution (is it 21?)
    //currentRPM = 3 * 1000000 / (currentTime - lastRPMUpdateTime) * sensorTicks;
    currentRPM = sensorTicks * 60000000 / ((currentTime - lastRPMUpdateTime) * 20);

    //term.position(1, 0);
    //term.clear_line_right();
    //term.set_attribute(BT_BOLD);
    //term.set_color(BT_RED, BT_BLACK);
    //term.print(sensorTicks);



    //Reset the clock and ticks
    lastRPMUpdateTime = micros();
    sensorTicks = 0;

    totalRevs++;

    //Restart interrupts
    interrupts();

  }
}

void updateTerminal()
{
  term.position(0, 0);
  term.set_attribute(BT_NORMAL);
  term.print(F("Current RPM: "));

  term.clear_line_right();
  term.set_attribute(BT_BOLD);
  term.set_color(BT_GREEN, BT_BLACK);
  term.print(currentRPM);

    //term.position(1, 0);
    //term.clear_line_right();
    //term.set_attribute(BT_BOLD);
    //term.set_color(BT_RED, BT_BLACK);
    //term.print(totalRevs);


    term.position(1, 0);
    term.clear_line_right();


    if(currentRPM > 350)
    {
    term.position(1, 0);
    term.clear_line_right();
    term.set_attribute(BT_BOLD);
    term.set_color(BT_RED, BT_BLACK);
    term.print("DANGER ZONE!!!");
    }

}

void updateTicks()
{
  //Update sensor ticks
  sensorTicks++;
  //Serial.println(sensorTicks);

  //Reset timeout
  RPMTimeoutCounter = 0;

}
