// Public global paramters related to this module

// related to custom serial read, buffer & parsing
// http://forum.arduino.cc/index.php?topic=288234.0
const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;

char messageFromPC[buffSize] = {0};

// END Public global paramters related to this module


//==============================


// Custom Serial data receive code, to prevent String objects which allocate data dynammically and possible fill up the stack
// http://forum.arduino.cc/index.php?topic=288234.0

void getDataFromPC() {

  int antiSerialReadPortLock;


  antiSerialReadPortLock = 0;

  // receive data from PC and save it into inputBuffer

  // read all available data, which might block further operations, when data keeps coming fast.
  // But to not read more then 'buffSize' chars in one go to prevent possible Serial DOS attack
  while ( (Serial.available() > 0) && (antiSerialReadPortLock++ < buffSize) ) {

    char x = Serial.read();

    // the order of these IF clauses is significant

    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      //parseData();
      processDataFromPC();
    }

    if (readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;    // ugly, last char gets overwritten next time when buffer is full... and gets finally overwritten by a'\0.
      }
    }

    if (x == startMarker) {
      bytesRecvd = 0;
      readInProgress = true;
    }
  }
}



// Show all configuireables (and mybe more)
// called from /showconfig
void print_running_configtoPC(void) {

  //HIER

  Serial.print(F("g_debug: "));
  Serial.println(g_debug);

  Serial.print(F("g_operating_mode: "));
  if ( g_operating_mode == C_OPERATING_MODE_AUTOMATIC) Serial.println(F("AUTOMATIC"));
  if ( g_operating_mode == C_OPERATING_MODE_SIMULATED) Serial.println(F("SIMULATED"));
  if ( g_operating_mode == C_OPERATING_MODE_MANUAL) Serial.println(F("MANUAL"));

  Serial.print(F("g_simulate_values[0]: "));
  Serial.println(g_simulate_values[0]);

  Serial.print(F("g_simulate_values[1]: "));
  Serial.println(g_simulate_values[1]);

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

  Serial.print(F("g_dallas_conversion_time(ms): "));
  Serial.println(g_dallas_conversion_time);

  Serial.print(F("g_Dallas_Resulution: "));
  Serial.println(g_Dallas_Resolution);

  Serial.print(F("C_Dallas_Resulution: "));
  Serial.println(C_Dallas_Resolution);


  Serial.print(F("actual global library resolution: "));
  Serial.println(sensors.getResolution());

  Serial.print(F("Device[0] resolution: "));
  Serial.println(sensors.getResolution(DallasDevices[MotorTemp_IDX]));

  Serial.print(F("Device[1] resolution: "));
  Serial.println(sensors.getResolution(DallasDevices[RadiatorTemp_IDX]));

  Serial.print(F("g_Dallas_Synchronous: "));
  Serial.println(g_Dallas_Synchronous);

  Serial.print("dutycycle_desired: ");
  Serial.println(dutycycle_desired);

  Serial.print(F("C_PWM_Period_us (PWM) (uS): "));
  Serial.println(C_PWM_Period_us);

  Serial.print(F("C_PWM_Period_us (PWM) (Hz): "));
  Serial.println( (1.0 / C_PWM_Period_us) * 1000000);

  Serial.print(F("g_PWM_Period (PWM) (uS): "));
  Serial.println(g_PWM_Period);

  Serial.print(F("g_PWM_Period (PWM) (Hz): "));
  Serial.println( (1.0 / g_PWM_Period) * 1000000);

  Serial.print(F("g_completely_manual_motor_control_pwm_dutycycle: "));
  Serial.println(g_completely_manual_motor_control_pwm_dutycycle);

  Serial.print(F("g_completely_manual_motor_control_pwm_period_us: "));
  Serial.println(g_completely_manual_motor_control_pwm_period_us);


  Serial.print(F("C_FAN_SLOWSTART_TIME (S): "));
  Serial.println(C_FAN_SLOWSTART_TIME);

  Serial.print(F("g_FAN_SLOWSTART_TIME (S): "));
  Serial.println(g_FAN_SLOWSTART_TIME);

  // derived values, no constant defined directly
  //Serial.print(F("C_pwm_soft_change_period_us (uS): "));
  //Serial.println(C_pwm_soft_change_period_us);

  // derived values, no constant defined directly
  //Serial.print(F("C_pwm_soft_change_period_us (Hz): "));
  //Serial.println( (1.0/C_pwm_soft_change_period_us)*1000000);


  Serial.print(F("g_pwm_control_timer_period_us (uS): "));
  Serial.println(g_pwm_control_timer_period_us);

  Serial.print(F("g_pwm_control_timer_period_us (Hz): "));
  Serial.println( (1.0 / g_pwm_control_timer_period_us) * 1000000);


  Serial.print(F("g_pwm_soft_step: "));
  Serial.println(g_pwm_soft_step);

  Serial.print(F("#pwm_soft_step: "));
  Serial.println((PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE ) / g_pwm_soft_step);

  Serial.print(F("g_CoolingWaterTemp_Motor_OUT_InC: "));
  Serial.println(g_CoolingWaterTemp_Motor_OUT_InC);

  Serial.print(F("g_CoolingWaterTemp_Motor_IN_InC: "));
  Serial.println(g_CoolingWaterTemp_Motor_IN_InC);

  Serial.print(F("C_Temperature_PWM_Policy: "));
  Serial.println(C_Temperature_PWM_Policy);

  Serial.print(F("g_Temperature_PWM_Policy: "));
  Serial.println(g_Temperature_PWM_Policy);

  Serial.print(F("C_Cooling_Policy3__MotorOut_FanONTemp: "));
  Serial.println(C_Cooling_Policy3__MotorOut_FanONTemp);

  Serial.print(F("C_Cooling_Policy3__MotorIN_FanONTemp: "));
  Serial.println(C_Cooling_Policy3__MotorIN_FanONTemp);

  Serial.print(F("C_Cooling_Policy3__MotorIN_FanOffTemp: "));
  Serial.println(C_Cooling_Policy3__MotorIN_FanOffTemp);


  Serial.print(F("g_Cooling_Policy3__MotorOut_FanONTemp: "));
  Serial.println(g_Cooling_Policy3__MotorOut_FanONTemp);

  Serial.print(F("g_Cooling_Policy3__MotorIN_FanONTemp: "));
  Serial.println(g_Cooling_Policy3__MotorIN_FanONTemp);

  Serial.print(F("g_Cooling_Policy3__MotorIN_FanOffTemp: "));
  Serial.println(g_Cooling_Policy3__MotorIN_FanOffTemp);


  Serial.print(F("C_EEPROM_SCHEMA in code: "));
  Serial.println(C_EEPROM_SCHEMA);

#ifdef __AVM__
  Serial.print(F("C_EEPROM_SCHEMA on EEPROM: "));
  Serial.println(EEPROM.read(C_EEPROM_SCHEMA_IDX));
#endif
#ifdef ESP32
  preferences.begin(C_Preferences_namespace, true);
  Serial.print(F("C_EEPROM_SCHEMA from Preferences: "));
  Serial.println(preferences.getUInt(Preferences_keys[C_EEPROM_SCHEMA_IDX], NO_VALUE));
  preferences.end();
#endif

  Serial.print(F("C_PROGVERSION in code: "));
  Serial.println(C_PROGVERSION);

#ifdef __AVR__
  Serial.print(F("C_PROGVERSION on EEPROM: "));
  Serial.println(EEPROM.read(C_EEPROM_PROGVER_IDX));
#endif
#ifdef ESP32
  preferences.begin(C_Preferences_namespace, true);
  Serial.print(F("C_PROGVERSION on EEPROM: "));
  Serial.println(preferences.getUInt(Preferences_keys[C_EEPROM_PROGVER_IDX], NO_VALUE));
  preferences.end();
#endif

  Serial.print(F("g_display_update_time (ms): "));
  Serial.println(g_display_update_time);

  Serial.print(F("g_Dallas_Default_Ordering (true/false): "));
  Serial.println(g_Dallas_Default_Ordering);

  //Serial.print(F("EEPROM g_display_update_time (ms): "));
  //Serial.println(EEPROM[C_display_update_time_IDX]);
  // EEPROM[] reads just a byte, and for display update in ms which I like to have at 1000, it just doesn't fit.

  //Serial.print(F("EEPROM address size: "));
  //Serial.println(EEPROM.length());

  Serial.print("g_calculated_rpm : ");
  Serial.println(g_calculated_rpm);

  Serial.print("DallasMissedCounters[MotorTemp_IDX] : ");
  Serial.println(DallasMissedCounters[MotorTemp_IDX]);

  Serial.print("DallasMissedCounters[RadiatorTemp_IDX] : ");
  Serial.println(DallasMissedCounters[RadiatorTemp_IDX]);


}


//=========================
void parseData() {

  // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(inputBuffer, ",");     // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  //newFlashInterval = atoi(strtokIndx);     // convert this part to an integer

  strtokIndx = strtok(NULL, ",");
  //servoFraction = atof(strtokIndx);     // convert this part to a float

}


void processDataFromPC() {

  // debug
  Serial.print('\n');
  Serial.print(inputBuffer);
  Serial.print('\n');

  if (strcmp("debug on", inputBuffer) == 0) g_debug = true;
  if (strcmp("debug off", inputBuffer) == 0) g_debug = false;
  if (strcmp("reload", inputBuffer) == 0) software_Reset();    // doesn't work, results in a requirement to reflash code... (TODO)
  if (strcmp("watchdog test", inputBuffer) == 0) while (1);   // Wait for watchdog to kick in. not, needs a reflash after this ... (TODO)

  {
    const char* cmd = "resolution ";  // const, not in progmem 9,10,11,12
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int resolution = atoi(separator);
        set_dallas_resolution(resolution);
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }

  if (strcmp("async on", inputBuffer) == 0) g_Dallas_Synchronous = false;
  if (strcmp("async off", inputBuffer) == 0) g_Dallas_Synchronous = true;


  {
    const char* cmd = "period ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int period = atoi(separator);
        if (g_operating_mode != C_OPERATING_MODE_MANUAL) set_PWM_Period_us(period);
        g_PWM_Period = period;   // store setting for future re-use
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }

  {
    const char* cmd = "fanstarttime ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int fantime_sec = atoi(separator);
        set_pwm_control_timer(fantime_sec);
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }


  if (strcmp("simulate on", inputBuffer) == 0) g_operating_mode = C_OPERATING_MODE_SIMULATED;
  if (strcmp("simulate off", inputBuffer) == 0) g_operating_mode = C_OPERATING_MODE_AUTOMATIC; // Automatic is always the default to prevent "issues"

  {
    const char* cmd = "motortemp ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int temperature = atoi(separator);
        g_simulate_values[MotorTemp_IDX] = temperature;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }
  {
    const char* cmd = "radiatortemp ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int temperature = atoi(separator);
        g_simulate_values[RadiatorTemp_IDX] = temperature;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }

  //g_Cooling_Policy3__MotorOut_FanONTemp
  //g_Cooling_Policy3__MotorIN_FanONTemp
  //g_Cooling_Policy3__MotorIN_FanOffTemp

  {
    const char* cmd = "cp3_motortemp_fanontemp ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int temperature = atoi(separator);
        g_Cooling_Policy3__MotorOut_FanONTemp = temperature;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }

  {
    const char* cmd = "cp3_radiatortemp_fanontemp ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int temperature = atoi(separator);
        g_Cooling_Policy3__MotorIN_FanONTemp = temperature;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }

  {
    const char* cmd = "cp3_radiatortemp_fanofftemp ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int temperature = atoi(separator);
        g_Cooling_Policy3__MotorIN_FanOffTemp = temperature;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }

  {
    const char* cmd = "displayupdate ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int d_temp = atoi(separator);
        g_display_update_time = d_temp;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }


  // 1 = true
  // 0 = false
  {
    const char* cmd = "dallasdefaultorder ";  // const, not in progmem
    if (strncmp(cmd, inputBuffer, strlen(cmd)) == 0) {
      char* separator = strchr(inputBuffer, ' ');
      if (separator != 0) {
        // Actually split the string in 2: replace ':' with 0
        //*separator = 0;
        ++separator;
        int d_temp = atoi(separator);
        g_Dallas_Default_Ordering = d_temp;
      } else {
        Serial.println(F("Expected option, but not found"));
      }
    }
  }


  if (strcmp("show run", inputBuffer) == 0) print_running_configtoPC();

  if (strcmp("clear run", inputBuffer) == 0) read_saved_values_from_persist_storage();
  if (strcmp("clear start", inputBuffer) == 0) read_default_values_from_Program();

  if (strcmp("erase startup", inputBuffer) == 0) persistent_variabeles_initialize(); // write program defaults to persistent storage
  if (strcmp("write mem", inputBuffer) == 0) save_running_config();

}

//====================
void showParsedData() {
  Serial.print("Message ");
  Serial.println(messageFromPC);
  Serial.print("Integer ");
  //Serial.println(integerFromPC);
  Serial.print("Float ");
  // Serial.println(floatFromPC);
}



//=========================

void delay_until_Serial_available(int m_seconds) {
  int i;

  for (i = 0; (i < m_seconds) && (Serial.available() == 0) ; i++) {
    delay(1);
  }

}

//=========================




