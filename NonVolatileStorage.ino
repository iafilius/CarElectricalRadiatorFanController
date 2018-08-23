
#ifdef __AVR__

// EEprom includes for reboot persistant values
#include <EEPROM.h>

#endif


#ifdef ESP32

#include <Preferences.h>    // Leave the primitive EEPROM behind ....and use something more highlevel
Preferences preferences;

#define C_Preferences_namespace "pwm-fancontrol"

#endif






//////////////////////////////////////////////////////////////////////////////////////
#ifdef __AVR__
//
void EEPROMWritelong(int address, long value) {
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}



// http://electronics.scriblab.de/write-an-integer-to-the-arduino-eeprom/
void EEPROMWriteInt(int address, int value) {
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.write(address, two);
  EEPROM.write(address + 1, one);
}

int EEPROMReadInt(int address) {
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);

  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}


// Write (currently) the program defaults to EEPROM
void EEPROM_Initialize(void) {
  // Reinitialize the EEPROM, in case the SCHEMA changes too much, or you want to get the hardcoded initial values back.
  //return;
  //EEPROM[C_EEPROM_SCHEMA_IDX] = C_EEPROM_SCHEMA;
  EEPROM.write(C_EEPROM_SCHEMA_IDX, C_EEPROM_SCHEMA);
  //return;

  //EEPROM[C_EEPROM_PROGVER_IDX] = C_PROGVERSION;
  EEPROM.write(C_EEPROM_PROGVER_IDX, C_PROGVERSION);

  //EEPROM[C_Temperature_PWM_Policy_IDX] = C_Temperature_PWM_Policy;
  EEPROM.write(C_Temperature_PWM_Policy_IDX, C_Temperature_PWM_Policy);

  //EEPROM[C_FAN_SLOWSTART_TIME_IDX] = C_FAN_SLOWSTART_TIME;
  EEPROM.write(C_FAN_SLOWSTART_TIME_IDX, C_FAN_SLOWSTART_TIME);

  //EEPROM[C_Cooling_Policy3__MotorOut_FanONTemp_IDX]=C_Cooling_Policy3__MotorOut_FanONTemp;
  EEPROM.write(C_Cooling_Policy3__MotorOut_FanONTemp_IDX, C_Cooling_Policy3__MotorOut_FanONTemp);

  //EEPROM[C_Cooling_Policy3__MotorIN_FanONTemp_IDX]=C_Cooling_Policy3__MotorIN_FanONTemp;
  EEPROM.write(C_Cooling_Policy3__MotorIN_FanONTemp_IDX, C_Cooling_Policy3__MotorIN_FanONTemp);

  //EEPROM[C_Cooling_Policy3__MotorIN_FanOffTemp_IDX]=C_Cooling_Policy3__MotorIN_FanOffTemp;
  EEPROM.write(C_Cooling_Policy3__MotorIN_FanOffTemp_IDX, C_Cooling_Policy3__MotorIN_FanOffTemp);

  //EEPROM[C_Dallas_Resolution_IDX]=C_Dallas_Resolution;
  EEPROM.write(C_Dallas_Resolution_IDX, C_Dallas_Resolution);

  //EEPROM[C_Dallas_Synchronous_IDX]=C_Dallas_Synchronous;
  EEPROM.write(C_Dallas_Synchronous_IDX, C_Dallas_Synchronous);

  //EEPROM[C_PWM_Period_IDX]=C_PWM_Period_us;
  EEPROM.write(C_PWM_Period_IDX, C_PWM_Period_us);


  //EEPROM[C_display_update_time_IDX]=C_display_update_time;
  EEPROM.write(C_display_update_time_IDX, C_display_update_time);

  EEPROM.write(C_Dallas_Default_Ordering_IDX, C_Dallas_Default_Ordering);

  //eeprom_write_block((const void*)&C_display_update_time,(void*)C_PWM_Period_IDX, sizeof(g_display_update_time));   // this variabele most likely sceeds the standard byte value...

#ifdef ESP32
  EEPROM.commit();    // seems to be propritiary for ESP EEPROM implementation, not used anyway (preferences framework)
#endif
}

void EEPROM_save_running_config(void) {

  //EEPROM[C_EEPROM_SCHEMA_IDX] = C_EEPROM_SCHEMA;
  //EEPROM[C_EEPROM_PROGVER_IDX] = C_PROGVERSION;
  //EEPROM[C_Temperature_PWM_Policy_IDX] = g_Temperature_PWM_Policy;
  EEPROM.write(C_Temperature_PWM_Policy_IDX, g_Temperature_PWM_Policy);

  //EEPROM[C_FAN_SLOWSTART_TIME_IDX] = g_FAN_SLOWSTART_TIME;
  EEPROM.write(C_FAN_SLOWSTART_TIME_IDX, g_FAN_SLOWSTART_TIME);

  //EEPROM[C_Cooling_Policy3__MotorOut_FanONTemp_IDX]=g_Cooling_Policy3__MotorOut_FanONTemp;
  EEPROM.write(C_Cooling_Policy3__MotorOut_FanONTemp_IDX, g_Cooling_Policy3__MotorOut_FanONTemp);

  //EEPROM[C_Cooling_Policy3__MotorIN_FanONTemp_IDX]=g_Cooling_Policy3__MotorIN_FanONTemp;
  EEPROM.write(C_Cooling_Policy3__MotorIN_FanONTemp_IDX, g_Cooling_Policy3__MotorIN_FanONTemp);

  //EEPROM[C_Cooling_Policy3__MotorIN_FanOffTemp_IDX]=g_Cooling_Policy3__MotorIN_FanOffTemp;
  EEPROM.write(C_Cooling_Policy3__MotorIN_FanOffTemp_IDX, g_Cooling_Policy3__MotorIN_FanOffTemp);

  //EEPROM[C_Dallas_Resolution_IDX]=g_Dallas_Resolution;
  EEPROM.write(C_Dallas_Resolution_IDX, g_Dallas_Resolution);

  //EEPROM[C_Dallas_Synchronous_IDX]=g_Dallas_Synchronous;
  EEPROM.write(C_Dallas_Synchronous_IDX, g_Dallas_Synchronous);

  //EEPROM[C_PWM_Period_IDX]=g_PWM_Period;
  EEPROM.write(C_PWM_Period_IDX, g_PWM_Period);

  //EEPROM[C_display_update_time_IDX]=g_display_update_time;
  EEPROM.write(C_display_update_time_IDX, g_display_update_time);

  EEPROM.write(C_Dallas_Default_Ordering_IDX, g_Dallas_Default_Ordering);

  // TODO eeprom_write_block((const void*)&g_display_update_time,(void*)C_PWM_Period_IDX, sizeof(g_display_update_time));   // this variabele most likely sceeds the standard byte value...


}




void read_saved_values_from_EEPROM(void) {


  // EEPROM_SCHEMA
  // C_PROGVERSION
  g_Temperature_PWM_Policy = EEPROM.read(C_Temperature_PWM_Policy_IDX);

  //C_FAN_SLOWSTART_TIME
  g_FAN_SLOWSTART_TIME = EEPROM.read(C_FAN_SLOWSTART_TIME_IDX);
  // Hard Derived values
  //g_pwm_control_timer_period_us = g_FAN_SLOWSTART_TIME * 1000000 / PWM_MAX_DUTYCYCLE;
  //g_pwm_soft_step = 1;
  //set_pwm_control_timer(g_FAN_SLOWSTART_TIME);    TODO ESP32


  g_Cooling_Policy3__MotorOut_FanONTemp = EEPROM.read(C_Cooling_Policy3__MotorOut_FanONTemp_IDX);
  g_Cooling_Policy3__MotorIN_FanONTemp = EEPROM.read(C_Cooling_Policy3__MotorIN_FanONTemp_IDX);
  g_Cooling_Policy3__MotorIN_FanOffTemp = EEPROM.read(C_Cooling_Policy3__MotorIN_FanOffTemp_IDX);
  g_Dallas_Resolution = EEPROM.read(C_Dallas_Resolution_IDX);

  g_Dallas_Synchronous = EEPROM.read(C_Dallas_Synchronous_IDX);
  g_PWM_Period = EEPROM.read(C_PWM_Period_IDX);

  g_display_update_time = EEPROM.read(C_display_update_time_IDX);

  g_Dallas_Default_Ordering = EEPROM.read(C_Dallas_Default_Ordering_IDX);

  // TODO eeprom_read_block((void*)&g_display_update_time,(void*)C_PWM_Period_IDX, sizeof(g_display_update_time));   // this variabele most likely sceeds the standard byte value...


}


// checks if EEPROM is available
// initializes it with the default if no compatible SCHEMA is found
// reads the values in which has been stored previously
void My_EEPROM_setup(void) {

  // really needed??? untested...
  // https://github.com/espressif/arduino-esp32/issues/569
  // https://github.com/espressif/arduino-esp32/blob/master/tools/partitions/default.csv
#ifdef ESP32
  //no longer used in favour of Preferences
#define EEPROM_SIZE 0x1000
  //int len=EEPROM.length(EEPROM_SIZE);
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
#endif


  // Test EEPROM
  if ( EEPROM.read(C_EEPROM_SCHEMA_IDX) != C_EEPROM_SCHEMA) {
    EEPROM_Initialize();  // write program defaults to EEPROM
  }

  read_saved_values_from_EEPROM();
}

#endif



#ifdef ESP32

void Preferences_setup(void) {

#ifdef ESP32
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.

  preferences.begin(C_Preferences_namespace, true);
  unsigned int schema = preferences.getUInt("schema", NO_VALUE);
  preferences.end();


  if (schema != C_EEPROM_SCHEMA) {
    Preferences_initialize();   // write program defaults into preferences (NVRAM)
  }

  // if not previous defined read them back in, from NVRAM/EEPROM
  // normal operation: read previous stored values after having initialized once and set by user.
  read_saved_values_from_Preferences();

#endif
}

// NVRAM
void  read_saved_values_from_Preferences(void) {

  preferences.begin(C_Preferences_namespace, true);

  //preferences.getUInt(Preferences_keys[C_EEPROM_SCHEMA_IDX],C_EEPROM_SCHEMA);
  //preferences.getUInt(Preferences_keys[C_EEPROM_PROGVER_IDX], C_PROGVERSION);
  g_Temperature_PWM_Policy = preferences.getUInt(Preferences_keys[C_Temperature_PWM_Policy_IDX], C_Temperature_PWM_Policy);
  g_FAN_SLOWSTART_TIME = preferences.getUInt(Preferences_keys[C_FAN_SLOWSTART_TIME_IDX], C_FAN_SLOWSTART_TIME);
  g_Cooling_Policy3__MotorOut_FanONTemp = preferences.getUInt(Preferences_keys[C_Cooling_Policy3__MotorOut_FanONTemp_IDX], C_Cooling_Policy3__MotorOut_FanONTemp);
  g_Cooling_Policy3__MotorIN_FanONTemp = preferences.getUInt(Preferences_keys[C_Cooling_Policy3__MotorIN_FanONTemp_IDX], C_Cooling_Policy3__MotorIN_FanONTemp);
  g_Cooling_Policy3__MotorIN_FanOffTemp = preferences.getUInt(Preferences_keys[C_Cooling_Policy3__MotorIN_FanOffTemp_IDX], C_Cooling_Policy3__MotorIN_FanOffTemp);
  g_Dallas_Resolution = preferences.getUInt(Preferences_keys[C_Dallas_Resolution_IDX], C_Dallas_Resolution);
  g_Dallas_Synchronous = preferences.getUInt(Preferences_keys[C_Dallas_Synchronous_IDX], C_Dallas_Synchronous);
  g_PWM_Period = preferences.getUInt(Preferences_keys[C_PWM_Period_IDX], C_PWM_Period_us);
  g_display_update_time = preferences.getUInt(Preferences_keys[C_display_update_time_IDX], C_display_update_time);
  g_Dallas_Default_Ordering = preferences.getBool(Preferences_keys[C_Dallas_Default_Ordering_IDX], C_Dallas_Default_Ordering);

  preferences.end();

}


void Preferences_initialize(void) {

  preferences.begin(C_Preferences_namespace, false);

  // Reinitialize the NVRAM, in case the SCHEMA changes too much, or you want to get the hardcoded initial values back.
  preferences.putUInt(Preferences_keys[C_EEPROM_SCHEMA_IDX], C_EEPROM_SCHEMA);
  preferences.putUInt(Preferences_keys[C_EEPROM_PROGVER_IDX], C_PROGVERSION);
  preferences.putUInt(Preferences_keys[C_Temperature_PWM_Policy_IDX], C_Temperature_PWM_Policy);
  preferences.putUInt(Preferences_keys[C_FAN_SLOWSTART_TIME_IDX], C_FAN_SLOWSTART_TIME);
  preferences.putUInt(Preferences_keys[C_Cooling_Policy3__MotorOut_FanONTemp_IDX], C_Cooling_Policy3__MotorOut_FanONTemp);
  preferences.putUInt(Preferences_keys[C_Cooling_Policy3__MotorIN_FanONTemp_IDX], C_Cooling_Policy3__MotorIN_FanONTemp);
  preferences.putUInt(Preferences_keys[C_Cooling_Policy3__MotorIN_FanOffTemp_IDX], C_Cooling_Policy3__MotorIN_FanOffTemp);
  preferences.putUInt(Preferences_keys[C_Dallas_Resolution_IDX], C_Dallas_Resolution);
  preferences.putUInt(Preferences_keys[C_Dallas_Synchronous_IDX], C_Dallas_Synchronous);
  preferences.putUInt(Preferences_keys[C_PWM_Period_IDX], C_PWM_Period_us);
  preferences.putUInt(Preferences_keys[C_display_update_time_IDX], C_display_update_time);
  preferences.putBool(Preferences_keys[C_Dallas_Default_Ordering_IDX], C_Dallas_Default_Ordering);

  preferences.end();

}

void Preferences_save_running_config(void) {

  preferences.begin(C_Preferences_namespace, false);

  //EEPROM[C_EEPROM_SCHEMA_IDX] = C_EEPROM_SCHEMA;    // do not user initiated overwrite SCHEMA nor PROGVERSION
  //EEPROM[C_EEPROM_PROGVER_IDX] = C_PROGVERSION;
  // Store user changeable parameters for reuse later
  preferences.putUInt(Preferences_keys[C_Temperature_PWM_Policy_IDX], g_Temperature_PWM_Policy);
  preferences.putUInt(Preferences_keys[C_FAN_SLOWSTART_TIME_IDX], g_FAN_SLOWSTART_TIME);
  preferences.putUInt(Preferences_keys[C_Cooling_Policy3__MotorOut_FanONTemp_IDX], g_Cooling_Policy3__MotorOut_FanONTemp);
  preferences.putUInt(Preferences_keys[C_Cooling_Policy3__MotorIN_FanONTemp_IDX], g_Cooling_Policy3__MotorIN_FanONTemp);
  preferences.putUInt(Preferences_keys[C_Cooling_Policy3__MotorIN_FanOffTemp_IDX], g_Cooling_Policy3__MotorIN_FanOffTemp);
  preferences.putUInt(Preferences_keys[C_Dallas_Resolution_IDX], g_Dallas_Resolution);
  preferences.putUInt(Preferences_keys[C_Dallas_Synchronous_IDX], g_Dallas_Synchronous);
  preferences.putUInt(Preferences_keys[C_PWM_Period_IDX], g_PWM_Period);
  preferences.putUInt(Preferences_keys[C_display_update_time_IDX], g_display_update_time);
  preferences.putBool(Preferences_keys[C_Dallas_Default_Ordering_IDX], g_Dallas_Default_Ordering);

  preferences.end();
}
#endif




////////////////////////////////////////////
// Compile time is a form of non persistent storage
// this functions belongs to similar functions in this file.

// reads the default values defined in code/program
void read_default_values_from_Program(void) {

  // EEPROM_SCHEMA
  // C_PROGVERSION
  g_Temperature_PWM_Policy = C_Temperature_PWM_Policy;
  // C_FAN_SLOWSTART_TIME
  g_FAN_SLOWSTART_TIME = C_FAN_SLOWSTART_TIME;
  // Hard dereived values
  // g_pwm_control_timer_period_us= g_FAN_SLOWSTART_TIME* 1000000 / PWM_MAX_DUTYCYCLE;
  // g_pwm_soft_step = 1;
  set_pwm_control_timer(g_FAN_SLOWSTART_TIME);

  g_Cooling_Policy3__MotorOut_FanONTemp = C_Cooling_Policy3__MotorOut_FanONTemp;
  g_Cooling_Policy3__MotorIN_FanONTemp = C_Cooling_Policy3__MotorIN_FanONTemp;
  g_Cooling_Policy3__MotorIN_FanOffTemp = C_Cooling_Policy3__MotorIN_FanOffTemp;
  g_Dallas_Resolution = C_Dallas_Resolution;

  g_Dallas_Synchronous =  C_Dallas_Synchronous;
  g_PWM_Period = C_PWM_Period_us;
  g_display_update_time = C_display_update_time;
  g_Dallas_Default_Ordering = C_Dallas_Default_Ordering;

}







// Init storage if SCHEMA has changed , write program defaults
// Needs to be defined as last as My_EEPROM_setup() isn't recognized (duh...) (leonardo)
void persistent_variabeles_setup(void) {

#ifdef __AVR__
  My_EEPROM_setup();
#endif

#ifdef ESP32
  // using nvram
  Preferences_setup();
#endif

}

// Needs to be defined as last as My_EEPROM_setup() isn't recognized (duh...) (leonardo)
void persistent_variabeles_initialize(void) {
#ifdef __AVR__
  EEPROM_Initialize();
#endif

#ifdef ESP32
  Preferences_initialize();
#endif

}


// Needs to be defined as last as My_EEPROM_setup() isn't recognized (duh...) (leonardo)
void read_saved_values_from_persist_storage(void) {

#ifdef __AVR__
  read_saved_values_from_EEPROM();
#endif

#ifdef ESP32
  read_saved_values_from_Preferences();
#endif

}


// Needs to be defined as last as My_EEPROM_setup() isn't recognized (duh...) (leonardo)
void save_running_config (void) {
#ifdef __AVR__
  EEPROM_save_running_config();
#endif

#ifdef ESP32
  Preferences_save_running_config();
#endif

}


