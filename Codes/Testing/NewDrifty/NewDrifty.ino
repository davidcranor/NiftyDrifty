//UI update timing, in microseconds (makes it less confusing to update currentTime everywhere)
#define UI_UPDATE_RATE 50000
#define SENSOR_TICKS_SAMPLE_SIZE 20

volatile uint32_t sensorTicks;
volatile unsigned long RPMTimeoutCounter;

uint32_t currentRPM;

unsigned long lastRPMUpdateTime;
unsigned long lastUIUpdateTime;

unsigned long currentTime;


void setup()
{
  //Begin serial comm
  Serial.begin(115200);

  //Set up pin change interrupt for hall effect
  //https://www.arduino.cc/en/Reference/attachInterrupt
  attachInterrupt(digitalPinToInterrupt(2), updateTicks, RISING);

  //Initalize variables
  sensorTicks = 0;
  currentRPM = 0;
  lastRPMUpdateTime = 0;
  lastUIUpdateTime = 0;
}

void loop()
{
  currentTime = micros();

  //Update readout and LEDs
  if (currentTime - lastUIUpdateTime >= UI_UPDATE_RATE)
  {
    Serial.print("Current RPM: ");
    Serial.println(currentRPM);

    //Reset the clock
    lastUIUpdateTime = currentTime;

    //If it's been more than a second since the sensor triggered, the wheel is stopped
    if (RPMTimeoutCounter > 1000000 / UI_UPDATE_RATE)
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

    //Serial.print("Sensor ticks: ");
    //Serial.print(sensorTicks);

    //Serial.print("Time: ");
    //Serial.println(currentTime - lastRPMUpdateTime);
    
    //Update RPM, assuming 1 hall effect event per revolution
    currentRPM = 60 * 1000000 / (currentTime - lastRPMUpdateTime) * sensorTicks;

    //Reset the clock and ticks
    lastRPMUpdateTime = micros();
    sensorTicks = 0;

    //Restart interrupts
    interrupts();
    
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
