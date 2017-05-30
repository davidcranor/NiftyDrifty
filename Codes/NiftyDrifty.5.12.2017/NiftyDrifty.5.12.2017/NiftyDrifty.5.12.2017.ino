#include <Arduino.h>

#include <BasicTerm.h>
#include <FastLED.h>

#define DEBUG_LED A5

//FastLED Stuff
#define NUM_LEDS 270
//#define NUM_LEDS 12
CRGB leds[NUM_LEDS];

//UI update timing, in microseconds (makes it less confusing to update currentTime everywhere)
#define UI_UPDATE_RATE 50000
#define SENSOR_TICKS_SAMPLE_SIZE 20
#define RPM_TIMEOUT 1000000

#define DANGER_RPM 1500

#define TICKS_BUFFER_SIZE 8

#define MIN_TICK_TIME 4000
#define MAX_TICK_TIME 35000

BasicTerm term(&Serial);

volatile uint32_t sensorTicks;
volatile unsigned long RPMTimeoutCounter;

volatile unsigned long ticksBuffer[TICKS_BUFFER_SIZE];
volatile uint8_t ticksBufferPointer;

uint32_t currentRPM;
uint32_t totalRevs;

unsigned long lastRPMUpdateTime;
unsigned long lastUIUpdateTime;

volatile unsigned long currentTickTime;
volatile unsigned long lastTickTime;
volatile unsigned long averageTickTime;

unsigned long currentTime;


void setup()
{
  pinMode(DEBUG_LED, OUTPUT);
  digitalWrite(DEBUG_LED, LOW);

  //Begin serial comm
  Serial.begin(115200);

  //Set up pin change interrupt for hall effect
  //https://www.arduino.cc/en/Reference/attachInterrupt
  attachInterrupt(digitalPinToInterrupt(2), updateTicks, CHANGE);

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

  //Init ticks buffer
  ticksBufferPointer = 0;
  for(int i = 0; i < TICKS_BUFFER_SIZE; i++)
  {
    ticksBuffer[i] = 0;
  }

  currentTickTime = 0;
  lastTickTime = 0;

  //FastLED Stuff
  FastLED.addLeds<APA102>(leds, NUM_LEDS);
  //FastLED.addLeds<APA102, 11, 13, BGR, DATA_RATE_MHZ(1)>(leds, NUM_LEDS);

}

void loop()
{
  currentTime = micros();

  //Update readout and LEDs
  if (currentTime - lastUIUpdateTime >= UI_UPDATE_RATE)
  {
    //Do whatever we gonna do with the tick times buffer
    processTickTimes();

    //Update LEDs
    updateLEDs();

    //Update terminal readout
    updateTerminal();

    //Reset the clock
    lastUIUpdateTime = currentTime;

  }
}

void updateTerminal()
{

  term.set_attribute(BT_NORMAL);
  for(int i = 0; i < TICKS_BUFFER_SIZE; i++)
  {
    Serial.print(ticksBuffer[i]);
    Serial.print(" ");
  }

  term.set_attribute(BT_BOLD);
  term.set_color(BT_GREEN, BT_BLACK);
  Serial.print(averageTickTime);

  Serial.print(" ");

  term.set_attribute(BT_BOLD);
  term.set_color(BT_RED, BT_BLACK);
  Serial.println(getNormalizedSpeed(NUM_LEDS));

}

void updateTicks()
{

  //Toggle debug pin
  digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));

  //Init the variables and get ready to find new tick time
  unsigned long elapsedTickTime;

  //Take current time reading
  currentTickTime = micros();

  //Calculate elapsed time since last event
  elapsedTickTime = currentTickTime - lastTickTime;

  if(elapsedTickTime > 5000)
  {

    //Add elapsed time since last event to circular buffer
    if(ticksBufferPointer < TICKS_BUFFER_SIZE)
    {
      ticksBuffer[ticksBufferPointer] = elapsedTickTime;
      //ticksBuffer[ticksBufferPointer] = currentTickTime;
    } else {
      ticksBufferPointer = 0;
      ticksBuffer[ticksBufferPointer] = elapsedTickTime;
      //ticksBuffer[ticksBufferPointer] = currentTickTime;
    }

    ticksBufferPointer++;

    //Reset timeout
    RPMTimeoutCounter = 0;

    //Set up for next time measurement
    lastTickTime = currentTickTime;
  }

  //Toggle debug pin
  //digitalWrite(DEBUG_LED, !digitalRead(DEBUG_LED));

}

void processTickTimes()
{
  //This part is still under construction.  Right now, it just averages the
  //rolling buffer and updates the variable averageTickTime

  unsigned long tickTimesAccumulator = 0;

  for(int i = 0; i < TICKS_BUFFER_SIZE; i++)
  {
    tickTimesAccumulator += ticksBuffer[i];
  }

  averageTickTime = (tickTimesAccumulator / TICKS_BUFFER_SIZE);
}

void updateLEDs()
{
  uint32_t mappedGraphValue;

  mappedGraphValue = getNormalizedSpeed(NUM_LEDS);

  //mappedGraphValue = map(rawGraphValue, 6000, 31000, 0, NUM_LEDS);

  FastLED.clear();

  for(int i = 0; i < NUM_LEDS; i++)
  {
    if(i < mappedGraphValue)
    {
      leds[i] = CRGB::Black;
    } else {
      leds[i] = CRGB::Red;
    }
  }

  FastLED.show();
}

uint32_t getNormalizedSpeed(uint32_t MAX_RANGE)
{
  if(averageTickTime < MIN_TICK_TIME)
  {

    return map(MIN_TICK_TIME, MIN_TICK_TIME, MAX_TICK_TIME, 0, MAX_RANGE);

  } else if(averageTickTime > MAX_TICK_TIME)
  {

    return map(MAX_TICK_TIME, MIN_TICK_TIME, MAX_TICK_TIME, 0, MAX_RANGE);

  } else {

    return map(averageTickTime, MIN_TICK_TIME, MAX_TICK_TIME, 0, MAX_RANGE);

  }

}
