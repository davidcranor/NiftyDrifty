#include <Arduino.h>

#include <BasicTerm.h>
#include <FastLED.h>

#define DEBUG_LED A5

//NUM_LEDS should equal the sum of the LEDs in each section
#define NUM_LEDS 158
CRGB leds[NUM_LEDS];

#define NUM_TAIL_TOP_LEDS 39
#define NUM_TAIL_BOTTOM_LEDS 39//Added 39 tail2 LEDs
#define NUM_SEAT_LEDS 66
#define NUM_HEAD_LEDS 14

CRGB tailLightTopLEDs[NUM_TAIL_TOP_LEDS];
CRGB tailLightBottomLEDs[NUM_TAIL_BOTTOM_LEDS];
CRGB seatLightLEDs[NUM_SEAT_LEDS];
CRGB headLightLEDs[NUM_HEAD_LEDS];

//UI update timing, in microseconds (makes it less confusing to update currentTime everywhere)
//#define UI_UPDATE_RATE 50000
#define UI_UPDATE_RATE 10000
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
  FastLED.addLeds<APA102, 11, 13, BGR>(leds, NUM_LEDS);


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

    //Update terminal readout, commented out to save cycles for animation
    //updateTerminal();

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
    } else {
      ticksBufferPointer = 0;
      ticksBuffer[ticksBufferPointer] = elapsedTickTime;
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

//Assuming that these are connected in a string, look in individual functions for where they end up, i'm in a hurry here

//Update the top tail lights
updateTopTailLight();

//Update the bottom tail lights
updateBottomTailLight();

//Update the seat light
updateSeatLight();

//Update the headlight
updateHeadLight();

//Clear the LED string
//FastLED.clear();

//Slap 'em all into the main LED framebuffer, MAKE SURE WE DONT OVERRUN HERE
memcpy(&leds[0], &tailLightTopLEDs[0], NUM_TAIL_TOP_LEDS * sizeof(CRGB));
memcpy(&leds[NUM_TAIL_TOP_LEDS], &tailLightBottomLEDs[0], NUM_TAIL_BOTTOM_LEDS * sizeof(CRGB));
memcpy(&leds[NUM_TAIL_BOTTOM_LEDS + NUM_TAIL_TOP_LEDS], &seatLightLEDs[0], NUM_SEAT_LEDS * sizeof(CRGB));
memcpy(&leds[NUM_TAIL_TOP_LEDS + NUM_TAIL_BOTTOM_LEDS + NUM_SEAT_LEDS], &headLightLEDs[0], NUM_HEAD_LEDS * sizeof(CRGB));

//Show the LED string
FastLED.show();

}

void updateTopTailLight()
{

  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( tailLightTopLEDs, NUM_TAIL_TOP_LEDS, 50);
  int pos = beatsin16(15,0,NUM_TAIL_TOP_LEDS);
  tailLightTopLEDs[pos] += CHSV( CRGB::Red, 255, 192);

}

void updateBottomTailLight()
{

  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( tailLightBottomLEDs, NUM_TAIL_BOTTOM_LEDS, 50);
  int pos = beatsin16(15,0,NUM_TAIL_BOTTOM_LEDS);
  tailLightBottomLEDs[pos] += CHSV( CRGB::Red, 255, 192);

}

void updateSeatLight()
{
  uint8_t mappedGraphValue;

  mappedGraphValue = getNormalizedSpeed(255);

/*
  for(int i = 0; i < NUM_SEAT_LEDS; i++)
  {
    if(i < mappedGraphValue)
    {
      seatLightLEDs[i] = CRGB::Black;
    } else {
      seatLightLEDs[i] = CRGB::Blue;
    }
  }
  */

  for(int i = 0; i < NUM_SEAT_LEDS; i++)
  {
      //seatLightLEDs[i] = CHSV(mappedGraphValue, 255, 255);
      seatLightLEDs[i] = CHSV(beatsin8(4), 255, 255);
  }

}

void updateHeadLight()
{
  /*
  for(int i = 0; i < NUM_HEAD_LEDS; i++)
  {
    headLightLEDs[i] = CRGB::White;
  }
  */

  fadeToBlackBy( headLightLEDs, NUM_HEAD_LEDS, 50);
  int pos = beatsin16(25,0,NUM_HEAD_LEDS);
  headLightLEDs[pos] += CHSV( CRGB::Red, 255, 192);

  //fadeLightBy(tailLightLEDs, NUM_TAIL_LEDS, 20);
}

//Changed from 32 to 8 here, HACK
uint8_t getNormalizedSpeed(uint8_t MAX_RANGE)
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
