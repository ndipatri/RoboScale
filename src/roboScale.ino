#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>
#include <Arduino.h>

NAU7802 myScale; //Create instance of the NAU7802 class

// These values are scale-specific and need to be derived!
double SCALE_FACTOR = 0.000666;   
double SCALE_OFFSET = 2.91;  


//EEPROM locations to store 4-byte variables
#define EEPROM_SIZE 100 //Allocate 100 bytes of EEPROM
#define LOCATION_CALIBRATION_FACTOR 0 //Float, requires 4 bytes of EEPROM
#define LOCATION_ZERO_OFFSET 20 //Must be more than 4 away from previous spot. 24-bit signed, stored as int32_t. Requires 4 bytes of EEPROM

bool settingsDetected = false; //Used to prompt user to calibrate their scale

//Average the weight over this many samples. This helps smooth out jitter.
const uint8_t avgWeights = 20; //At 40 SPS, this will produce readings at 2Hz

//Allow negative weights. Why not?
const bool allowNegative = true;

void setup()
{

  // This instructs the core to not connect to the
  // Particle cloud until explicitly instructed to
  // do so from within our service loop ... This is so the device
  // has the ability to operate offline.
  SYSTEM_MODE(MANUAL);

  Serial.begin(115200);
  Serial.println(F("Qwiic Scale Example"));

  Wire.begin();
  Wire.setClock(400000); //Qwiic Scale is capable of running at 400kHz if desired

  if (myScale.begin() == false) //begin performs an internal calibration. We will overwrite this with readSystemSettings
  {
    Serial.println(F("Scale not detected. Please check wiring. Freezing..."));
    while (1);
  }
  Serial.println(F("Scale detected!"));

  myScale.setSampleRate(NAU7802_SPS_40); //Set sample rate: 10, 20, 40, 80 or 320
  myScale.setGain(NAU7802_GAIN_16); //Gain can be set to 1, 2, 4, 8, 16, 32, 64, or 128.
  myScale.setLDO(NAU7802_LDO_3V0); //Set LDO (AVDD) voltage. 3.0V is the best choice for Qwiic

  readSystemSettings(); //Load NAU7802 offset and calibrationFactor from EEPROM

  Serial.print(F("NAU7802 offset register: "));
  Serial.println(myScale.getChannel1Offset());
  Serial.print(F("Library calibration factor: "));
  Serial.println(myScale.getCalibrationFactor());

  Serial.println(F("\r\nPress 'c' to calibrate the scale\r\n"));
}

void loop()
{
  if (myScale.available() == true)
  {
    float currentWeight = myScale.getWeight(allowNegative, avgWeights);

    Serial.print(F("Weight: "));
    Serial.print(currentWeight, 2); //Print 2 decimal places

    if(settingsDetected == false)
    {
      Serial.print(F("\tScale not calibrated. Press 'c'."));
    }

    Serial.println();
  }

  if (Serial.available())
  {
    byte incoming = Serial.read();

    if (incoming == 'c') //Calibrate
      calibrateScale();
  }

  delay(50);
}


//Gives user the ability to set a known weight on the scale and calculate a calibration factor
void calibrateScale(void)
{
  Serial.println();
  Serial.println();
  Serial.println(F("Scale calibration"));

  Serial.println(F("Setup scale with no weight on it. Press a key when ready."));
  while (Serial.available()) Serial.read(); //Clear anything in RX buffer
  while (Serial.available() == 0) delay(10); //Wait for user to press key

  //Perform an external offset - this sets the NAU7802's internal offset register
  myScale.calibrateAFE(NAU7802_CALMOD_OFFSET); //Calibrate using external offset
  
  Serial.print(F("New NAU7802 offset register: "));
  Serial.println(myScale.getChannel1Offset());

  Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));
  while (Serial.available()) Serial.read(); //Clear anything in RX buffer
  while (Serial.available() == 0) delay(10); //Wait for user to press key

  Serial.print(F("Please enter the weight, without units, currently sitting on the scale (for example '4.25'): "));
  while (Serial.available()) Serial.read(); //Clear anything in RX buffer
  while (Serial.available() == 0) delay(10); //Wait for user to press key

  //Read user input
  float weightOnScale = Serial.parseFloat();
  Serial.println();

  //Tell the library how much weight is currently on it
  //We are sampling slowly, so we need to increase the timeout too
  myScale.calculateCalibrationFactor(54.8, 64, 3000); //64 samples at 40SPS. Use a timeout of 3 seconds
  Serial.print(F("Weight on scale: "));
  Serial.println(54.8, 2);
  Serial.print(F("New library calibration factor: "));
  Serial.println(myScale.getCalibrationFactor(), 2);

  recordSystemSettings(); //Commit these values to EEPROM

  settingsDetected = true; //Mark the settings as detected
}

//Record the current system settings to EEPROM
void recordSystemSettings(void)
{
  //Get various values from the library and commit them to NVM
  EEPROM.put(LOCATION_CALIBRATION_FACTOR, myScale.getCalibrationFactor());
  EEPROM.put(LOCATION_ZERO_OFFSET, myScale.getChannel1Offset());
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void readSystemSettings(void)
{
  float settingCalibrationFactor; //Value used to convert the load cell reading to lbs or kg
  int32_t offsetRegister; //Zero value that is found when scale is tared

  //Look up the calibration factor
  //If the EEPROM has been erased, default to 1.0
  EEPROM.get(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
  if (settingCalibrationFactor == 0xFFFFFFFF)
  {
    settingCalibrationFactor = 1.0; //Default to 1.0
    EEPROM.put(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
  }

  //Look up the offset register
  //If the EEPROM has been erased, default to 0
  EEPROM.get(LOCATION_ZERO_OFFSET, offsetRegister);
  if (offsetRegister == 0xFFFFFFFF)
  {
    offsetRegister = 0; //Default to 0 - i.e. no offset
    EEPROM.put(LOCATION_ZERO_OFFSET, offsetRegister);
  }

  //Pass these values to the library and NAU7802
  myScale.setCalibrationFactor(settingCalibrationFactor);
  myScale.setChannel1Offset(offsetRegister);

  settingsDetected = true; //Assume for the moment that there are good cal values
  if (settingCalibrationFactor == 1.0 || offsetRegister == 0)
    settingsDetected = false; //Defaults detected. Prompt user to cal scale.
}