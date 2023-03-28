/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/ndipatri/development/roboGaggia/roboScale/RoboScale/src/roboScale.ino"
#include <Qwiic_Scale_NAU7802_Arduino_Library.h>
#include <Arduino.h>

void setup();
void loop();
void readScaleState();
#line 4 "/Users/ndipatri/development/roboGaggia/roboScale/RoboScale/src/roboScale.ino"
NAU7802 myScale; //Create instance of the NAU7802 class

// These values are scale-specific and need to be derived!
double SCALE_FACTOR = 0.000660;   
double SCALE_OFFSET = 3.21;  

#define SCALE_SAMPLE_SIZE 8 

double avgReadings[SCALE_SAMPLE_SIZE];
byte avgReadingIndex = 0;

double measuredWeight = 0.0;
long avgReading = 0.0;


// setup() runs once, when the device is first turned on.
void setup() {
  
  // I2C Setup
  Wire.begin();

  if (myScale.begin() == false)
  {
    Log.error("Scale not detected!");
  }

  myScale.setGain(NAU7802_GAIN_64); //Gain can be set to 1, 2, 4, 8, 16, 32, 64, or 128. default is 16
  myScale.setSampleRate(NAU7802_SPS_80); //Sample rate can be set to 10, 20, 40, 80, or 320Hz. default is 10
  myScale.calibrateAFE(); //Does an internal calibration. Recommended after power up, gain changes, sample rate changes, or channel changes.

  waitFor(Serial.isConnected, 3000);
}

void loop() {
  
  readScaleState();
  
  delay(200);
}

void readScaleState() {
  measuredWeight = 0.0;

  if (myScale.available() == true) {
    long scaleReading = myScale.getReading();

    // This is a rotating buffer
    avgReadings[avgReadingIndex++] = scaleReading;
    if(avgReadingIndex == SCALE_SAMPLE_SIZE) {
      
      measuredWeight = (float)avgReading * SCALE_FACTOR + SCALE_OFFSET;
      Particle.publish("roboScale", String(avgReading) + "," + String(measuredWeight), 60, PUBLIC);
      
      avgReadingIndex = 0;
    }

    avgReading = 0;
    for (int index = 0 ; index < SCALE_SAMPLE_SIZE ; index++)
      avgReading += avgReadings[index];
    avgReading /= SCALE_SAMPLE_SIZE;
  }
}