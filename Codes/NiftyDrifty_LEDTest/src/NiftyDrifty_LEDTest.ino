#include <Arduino.h>

#include <FastLED.h>
#define NUM_LEDS 12

CRGB leds[NUM_LEDS];

void setup()
{
    FastLED.addLeds<APA102>(leds, NUM_LEDS);
    //FastLED.addLeds<APA102, 11, 13, BGR, DATA_RATE_MHZ(1)>(leds, NUM_LEDS);
}

void loop()
{
  FastLED.clear();
  FastLED.clear();

/*
  for(int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = CRGB::Red;
    FastLED.show();
    // clear this led for the next time around the loop
    leds[dot] = CRGB::Black;
    //delay(10);
  }
  */

  for(int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = CRGB::Red;
  }

  FastLED.show();
  FastLED.show();

}
