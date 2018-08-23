
//Include libraries
// https://github.com/espressif/arduino-esp32/issues/755


// Public global paramters related to this module


bool TempSensorFound = false;

//#define MotorTemp_IDX     0
//#define RadiatorTemp_IDX  1


void setup_dallas(void) {
  // sensors

  sensors.begin();// IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

  //delay(500);
  // locate devices on the bus
  Serial.print(F("Found "));
  Serial.print(sensors.getDeviceCount(), DEC);      // Strange, this always seems to return zero.. why is that?
  Serial.println(F(" oneWire devices."));

  // search for devices on the bus and assign based on an index.
  // change order if g_Dallas_Default_Ordering == false
  if (!sensors.getAddress(DallasDevices[MotorTemp_IDX],   g_Dallas_Default_Ordering ? MotorTemp_IDX : RadiatorTemp_IDX)) Serial.println(F("Unable to find address for Device 0"));
//  Serial.print(F("Found "));
//  Serial.print(sensors.getDeviceCount(), DEC);      // Strange, this always seems to return zero.. why is that?
//  Serial.println(F(" oneWire devices."));

  if (!sensors.getAddress(DallasDevices[RadiatorTemp_IDX], g_Dallas_Default_Ordering ? RadiatorTemp_IDX : MotorTemp_IDX)) Serial.println(F("Unable to find address for Device 1"));
//  Serial.print(F("Found "));
//  Serial.print(sensors.getDeviceCount(), DEC);      // Strange, this always seems to return zero.. why is that?
//  Serial.println(F(" oneWire devices."));

  if (g_Dallas_Default_Ordering == false) {
    Serial.println(F("Reversed default order of dallas Device"));
  }

  // Libary CRC check of adress
  if ( !sensors.validAddress(DallasDevices[MotorTemp_IDX]) ) {
    Serial.println(F("Invalid adress (CRC check failed)"));
  } else {
    Serial.println(F("CRC OK"));
  }
  if ( !sensors.validAddress(DallasDevices[RadiatorTemp_IDX]) ) {
    Serial.println(F("Invalid adress (CRC check failed)"));
  } else {
    Serial.println(F("CRC OK"));
  }

  if ( sensors.readPowerSupply(DallasDevices[MotorTemp_IDX])) {
    Serial.println(F("Device runs has no power (undesired parasitic mode)!"));
  } else {
    Serial.println(F("Power OK"));
  }
  if ( sensors.readPowerSupply(DallasDevices[RadiatorTemp_IDX]) ) {
    Serial.println(F("Device runs has no power (undesired parasitic mode)!"));
  } else {
    Serial.println(F("Power OK"));
  }

  //sensors.setResolution(DallasDevices[MotorTemp_IDX],g_Dallas_Resolution);
  //sensors.setResolution(DallasDevices[RadiatorTemp_IDX],g_Dallas_Resolution);
  set_dallas_resolution(g_Dallas_Resolution);

  // Set (a)sync mode
  sensors.setWaitForConversion(g_Dallas_Synchronous);

  // how long we should "wait" (just not requesting the value, after starting conversion.
  // 750 is from datasheet for 12 bits
  //g_dallas_conversion_time = 750 / ( 1 << ( 12 - g_Dallas_Resolution ) );  // res in 9,10,11,12



#define SHOW_DALLAS_ERROR 1
  {
    DeviceAddress temp_device;
    int i;
    for (i = 0; i < 2; i++) {
      sensors.getAddress(temp_device, i);
      switch ( temp_device[0] )
      {
        case DS18S20MODEL:
          Serial.println(F("Device[i]  Chip = DS18(S)20"));  // or old DS1820
          //type_s = 1;
          break;
        case DS18B20MODEL:
          Serial.println(F("Device[i]  Chip = DS18B20"));
          //type_s = 0;
          TempSensorFound = true;     // only accept this type
          break;
        case DS1822MODEL :
          Serial.println(F("Device[i]  Chip = DS1822"));
          //type_s = 0;
          break;
        case DS1825MODEL :
          Serial.println(F("Device[i]  Chip = DS1825"));
          //type_s = 0;
          break;
        case DS28EA00MODEL :
          Serial.println(F("Device[i]  Chip = DS28EA00"));
          //type_s = 0;
          break;

        default:
#ifdef SHOW_DALLAS_ERROR
          Serial.println(F("Device is not a DS18x20 family device."));
#endif
          return;
      }
    }
  }


  // show the addresses we found on the bus
  Serial.print(F("Device 0[DallasDevices[MotorTemp_IDX]] Address: "));
  printAddress(DallasDevices[MotorTemp_IDX]);
  Serial.println();

  Serial.print(F("Device 0[DallasDevices[MotorTemp_IDX]] Alarms: "));
  printAlarms(DallasDevices[MotorTemp_IDX]);
  Serial.println();

  Serial.print(F("Device 1[DallasDevices[RadiatorTemp_IDX]] Address: "));
  printAddress(DallasDevices[RadiatorTemp_IDX]);
  Serial.println();

  Serial.print(F("Device 1[DallasDevices[RadiatorTemp_IDX]] Alarms: "));
  printAlarms(DallasDevices[RadiatorTemp_IDX]);
  Serial.println();

/*
  Serial.println(F("Setting alarm temps..."));

  // alarm when temp is higher than 30C
  sensors.setHighAlarmTemp(DallasDevices[MotorTemp_IDX], 30);

  // alarm when temp is lower than -10C
  sensors.setLowAlarmTemp(DallasDevices[MotorTemp_IDX], -10);

  // alarm when temp is higher than 31C
  sensors.setHighAlarmTemp(DallasDevices[RadiatorTemp_IDX], 31);

  // alarn when temp is lower than 27C
  sensors.setLowAlarmTemp(DallasDevices[RadiatorTemp_IDX], -10);

  Serial.print(F("New Device 0 Alarms: "));
  printAlarms(DallasDevices[MotorTemp_IDX]);
  Serial.println();

  Serial.print(F("New Device 1 Alarms: "));
  printAlarms(DallasDevices[RadiatorTemp_IDX]);
  Serial.println();
*/
}





// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print(F("0"));
    Serial.print(deviceAddress[i], HEX);
  }
}





// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print(F("Temp C: "));
  Serial.print(tempC);
  Serial.print(F(" Temp F: "));
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}




void printAlarms(uint8_t deviceAddress[])
{
  char temp;
  temp = sensors.getHighAlarmTemp(deviceAddress);
  Serial.print(F("High Alarm: "));
  Serial.print(temp, DEC);
  Serial.print(F("C/"));
  Serial.print(DallasTemperature::toFahrenheit(temp));
  Serial.print(F("F | Low Alarm: "));
  temp = sensors.getLowAlarmTemp(deviceAddress);
  Serial.print(temp, DEC);
  Serial.print(F("C/"));
  Serial.print(DallasTemperature::toFahrenheit(temp));
  Serial.print(F("F"));
}




// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
  Serial.print(F("Device Address: "));
  printAddress(deviceAddress);
  Serial.print(F(" "));
  printTemperature(deviceAddress);
  Serial.println();
}

void checkAlarm(DeviceAddress deviceAddress)
{
  if (sensors.hasAlarm(deviceAddress))
  {
    Serial.print(F("ALARM: "));
    printData(deviceAddress);
  }
}





// old version , serialized, spend huge time, just on waiting (blocking)
void get_Dallas_Temperatures_sync(void) {

  uint32_t t_start, t_stop;

  if (TempSensorFound == false) return; // refuse to do anything when sensor not found

  if (g_debug) Serial.print("Requesting temperatures...(synch)");

  t_start = millis();  // measuring time function call
  sensors.requestTemperatures(); // request to all devices on the bus
  t_stop = millis();  // measuring time function call
  t_dallas_sensors = t_stop - t_start;

  if (g_debug) Serial.print(F("requestTemperature Time:"));
  if (g_debug) Serial.println(t_dallas_sensors);

  if ( g_operating_mode == C_OPERATING_MODE_SIMULATED ) {
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    //sensors.getTempCByIndex(0);   // request value, and do not assign it.
    //sensors.getTempCByIndex(1);   // request value, and do not assign it.
    sensors.getTempC(DallasDevices[MotorTemp_IDX]); // request value, and do not assign it.
    sensors.getTempC(DallasDevices[RadiatorTemp_IDX]);// request value, and do not assign it.

    g_CoolingWaterTemp_Motor_OUT_InC = g_simulate_values[MotorTemp_IDX]; // Inject Simulated values
    g_CoolingWaterTemp_Motor_IN_InC = g_simulate_values[RadiatorTemp_IDX];  // Inject Simulated value
  } else {
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    //g_CoolingWaterTemp_Motor_OUT_InC = sensors.getTempCByIndex(0);
    //g_CoolingWaterTemp_Motor_IN_InC = sensors.getTempCByIndex(1);
    g_CoolingWaterTemp_Motor_OUT_InC = sensors.getTempC(DallasDevices[MotorTemp_IDX]);
    g_CoolingWaterTemp_Motor_IN_InC = sensors.getTempC(DallasDevices[RadiatorTemp_IDX]);

  }



  if (g_CoolingWaterTemp_Motor_OUT_InC == DEVICE_DISCONNECTED_C ) {
    Serial.println(F("ERROR: Device[DallasDevices[MotorTemp_IDX]] returns disconnected (getTempC())"));
    DallasMissedCounters[MotorTemp_IDX]++;
  }
  if (g_CoolingWaterTemp_Motor_IN_InC == DEVICE_DISCONNECTED_C ) {
    Serial.println(F("ERROR: Device[DallasDevices[RadiatorTemp_IDX]] returns disconnected (getTempC())"));
    DallasMissedCounters[RadiatorTemp_IDX]++;
  }


}

//-------------------------------------------------------

// updated version, asynchronous version, starts conversion, and reads next time, when conversion time has been expired.

void get_Dallas_Temperatures_async(void) {

  uint32_t t_start, t_stop;
  static uint32_t lastTempRequest;    // keep value over multiple funciton call's

  static bool running = false; // initialize for not in action, true=> waiting for an answer

  if (TempSensorFound == false) return; // refuse to do anything when sensor not found

  if (running == true) { // running (no else, because running is changing inside loop)

    if ( (millis() - lastTempRequest) >= g_dallas_conversion_time) {

      if (g_debug) {
        Serial.print(F("Reading temperatures... after (ms): "));
        Serial.print(millis() - lastTempRequest);
        Serial.print(F("    g_dallas_conversion_time (ms): "));
        Serial.println(g_dallas_conversion_time);
      }


      if ( sensors.isConversionComplete() ) {
        running = false;

        if ( g_operating_mode == C_OPERATING_MODE_SIMULATED) {
          // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
          //sensors.getTempCByIndex(0);   // request value, and do not assign it.
          //sensors.getTempCByIndex(1);   // request value, and do not assign it.
          sensors.getTempC(DallasDevices[MotorTemp_IDX]); // request value, and do not assign it.
          sensors.getTempC(DallasDevices[RadiatorTemp_IDX]);// request value, and do not assign it.

          g_CoolingWaterTemp_Motor_OUT_InC = g_simulate_values[MotorTemp_IDX]; // Simulated values
          g_CoolingWaterTemp_Motor_IN_InC = g_simulate_values[RadiatorTemp_IDX];  // Simulated value
        } else {
          // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
          //g_CoolingWaterTemp_Motor_OUT_InC = sensors.getTempCByIndex(0);
          //g_CoolingWaterTemp_Motor_IN_InC = sensors.getTempCByIndex(1);
          g_CoolingWaterTemp_Motor_OUT_InC = sensors.getTempC(DallasDevices[MotorTemp_IDX]);
          g_CoolingWaterTemp_Motor_IN_InC = sensors.getTempC(DallasDevices[RadiatorTemp_IDX]);
        }

        // ERROR handling
        if (g_CoolingWaterTemp_Motor_OUT_InC == DEVICE_DISCONNECTED_C ) {
          Serial.println(F("ERROR: Device[DallasDevices[MotorTemp_IDX]] returns disconnected (getTempC())"));
          DallasMissedCounters[MotorTemp_IDX]++;
        }
        if (g_CoolingWaterTemp_Motor_IN_InC == DEVICE_DISCONNECTED_C ) {
          Serial.println(F("ERROR: Device[DallasDevices[RadiatorTemp_IDX]] returns disconnected (getTempC())"));
          DallasMissedCounters[RadiatorTemp_IDX]++;
        }



      } else {
        Serial.println(F("ERROR: Sensor conversion time expired, but converson was not complete!!!!"));
        // Keep testing next time, perhaps a max retry/escalation/error recovery????

        if ( (millis() - lastTempRequest) >= (g_dallas_conversion_time * 4)) { // Escalation when expired time is over 4 times expected time
          Serial.println(F("ERROR: Sensor 4*g_dallas_conversion_time expired, restarting "));
          //sensors.reset??();  // does this work as expected?
          running = false;

          // Up the errors
          DallasMissedCounters[MotorTemp_IDX]++;
          DallasMissedCounters[RadiatorTemp_IDX]++;

        }
      }


      // Immediate restart requesting a new temperature requst, like first time.

    } else {
      if (g_debug > DEBUGLEVEL2) {
        Serial.print(F("dallas_conversion time not yet exceeded, time since last request (ms) : "));
        Serial.print(millis() - lastTempRequest);
        Serial.print(F("    g_dallas_conversion_time (ms): "));
        Serial.println(g_dallas_conversion_time);
      }

    }
  }



  if (running == false) { // Initial AND new measurements
    running = true;

    if (g_debug) Serial.println(F("Requesting temperatures... (async)"));

    t_start = millis();  // measuring time function call
    sensors.requestTemperatures(); // request to all devices on the bus
    lastTempRequest = t_stop = millis(); // measuring time function call

    if (g_debug) {
      t_dallas_sensors = t_stop - t_start;
      //if (g_debug) Serial.print(F("requestTemperature Request Time:"));
      //if (g_debug) Serial.println(t_dallas_sensors);
    }
  }
}



// Changes resultions and related conversion time to wait
// This function does not handle current started measurements
// So you may wait too long, or too short in transition measurement.
void set_dallas_resolution(int _resolution) {

  sensors.setResolution(DallasDevices[MotorTemp_IDX], _resolution);
  sensors.setResolution(DallasDevices[RadiatorTemp_IDX], _resolution);

  Serial.print(F("Set dallas sensor resolution[9,10,11,12] to :"));
  Serial.println(_resolution);

  g_Dallas_Resolution = _resolution; // update global value
  //g_dallas_conversion_time = 750 / ( 1 << ( 12 - g_Dallas_Resolution ) );  // res in 9,10,11,12
  g_dallas_conversion_time = sensors.millisToWaitForConversion(g_Dallas_Resolution); // same as above, but then using buildin library
}


// 


float getChipTemperatureInC(void) {

#ifdef __AVR__
// https://playground.arduino.cc/Main/InternalTemperatureSensor
#endif

#ifdef ESP32
 return getESP32ChipTemperatureInC();
#endif  
}


#ifdef ESP32
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


float getESP32ChipTemperatureInC(void) {
  #define MAGIC_CORRETIONFACTOR 46    // the thread at blynk forum
// Convert raw temperature in F to Celsius degrees
// Note: this results are WAY off, reading about 70 degrees celsius just at start..
// https://community.blynk.cc/t/esp32-internal-sensors/23041/37 on "Isn’t 61.7 ºC a bit high"

// and another thread mentions 32 as correction on raw value:
// 
uint8_t raw_temp_f= temprature_sens_read();
/*
Serial.print("Raw temp in F: ");
Serial.println(raw_temp_f);
Serial.print("Raw temp in F: ");
Serial.println(raw_temp_f,BIN);
*/

//uint8_t bugCorrection = 83;   // magical 46*1.8...=82,8
uint8_t bugCorrection = 42;   // manual adjusted to some reasonable value
raw_temp_f -= bugCorrection;
return  (( raw_temp_f - 32) / 1.8 - 0) ;
}

#endif

