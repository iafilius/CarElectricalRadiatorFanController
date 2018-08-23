// Public global paramters related to this module


// END Public global paramters related to this module


int cooling_policy_func_nr_of_runs=0;

// prototype of all cooling functions
typedef unsigned long int (*CF_ptr_t)(float, float);

#define MAXCF_descriptionlength 40
// static/fixed  size
typedef struct {
  CF_ptr_t CF_ptr;
  char description[MAXCF_descriptionlength];
} CoolingFunc_t;

// Array of actual advertized/selectable functions with its description
const CoolingFunc_t CoolingFunc_arr[]={
  {Cooling_Policy0,"CP: Permanent OFF"},                          // 0
  {Cooling_Policy1,"CP: Permanent ON"},                           // 1
  {Cooling_Policy3,"CP: Dynamic"},                                // 2
  {Cooling_Policy_90,"CP: Dynamic, FAN on above 90°C"},           // 3
  {Cooling_Policy_85,"CP: Dynamic, FAN on above 85°C"},           // 4
  {Cooling_Policy_update_failure_test,"DEBUG: desired pwm update failure test"}, // 5
  {Cooling_Policy_REBOOTME,"DEBUG: restart ESP32"}                   // 6
  };

// replaced by function to make it availabe in other modules
//#define NR_CFunctions  sizeof(CoolingFunc_arr)/sizeof(CoolingFunc_t)
unsigned int NR_CFunctions(void) {
  return sizeof(CoolingFunc_arr)/sizeof(CoolingFunc_t);
}

const char* getPolicyDescription(int PWM_Policy) {

    // check if #elements matches index
    if(  !(PWM_Policy < NR_CFunctions() ) ){
      // for some reason got out of range...
      Serial.println("PWM_Policy too large for #elements, setting to zero!!!");
      return NULL;
    }

return CoolingFunc_arr[PWM_Policy].description;
}


//
unsigned long Cooling_Policy0(float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  cooling_policy_func_nr_of_runs++;
  return PWM_MIN_DUTYCYCLE;
}

unsigned long Cooling_Policy1(float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  cooling_policy_func_nr_of_runs++;
  return PWM_MAX_DUTYCYCLE;
  //return 0;
}


unsigned long Cooling_Policy2 (float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC )
{
  signed long new_dutycycle_desired;
  cooling_policy_func_nr_of_runs++;

  // https://en.wikipedia.org/wiki/PID_controller
  //int Ki_slow = 5;  // Itegration constant
  //int Ki_fast = 100;  // Itegration constant
  //int error_upper;    // upper sensor delta
  //int error_lower;    // lower error delta
  //int error_delta;    // desired a minimum of 30

  new_dutycycle_desired = dutycycle_desired;


  // PID regulator rules

  // integrator RULES
  if (CoolingWaterTemp_Motor_OUT_InC > 100) {
    new_dutycycle_desired += 5;
  }
  if (CoolingWaterTemp_Motor_IN_InC > 70) {
    new_dutycycle_desired += 5;
  }

  // delta temperature minimim, but only when on operating temperature (80 degrees)
  if ( (CoolingWaterTemp_Motor_OUT_InC > 30) && ((CoolingWaterTemp_Motor_OUT_InC  - CoolingWaterTemp_Motor_IN_InC) < 20 )) {
    new_dutycycle_desired += 100;
  } else {
    new_dutycycle_desired -= 50;
  }

  /*
    if( CoolingWaterTemp_Motor_OUT_InC > 80) {   // begin operating range
      error_upper=CoolingWaterTemp_Motor_OUT_InC -80;
      error_delta=(CoolingWaterTemp_Motor_OUT_InC-CoolingWaterTemp_Motor_IN_InC) -30;   // desired 30 degrees delta minimum

      // desired state is to have 30 degrees difference with 90 degrees motor out temperature but may be 0 at 80 degrees
      // Output temperature needs to be dominant over (negative) delta always
      error=error_upper + error_upper*error_delta;

    }
  */


  // Range check/correction
  if (new_dutycycle_desired >= PWM_MAX_DUTYCYCLE) {
    new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
    // Alarm!!! end of range, overheating???
  }

  if (new_dutycycle_desired < 0 ) {
    new_dutycycle_desired = 0;
    // Alarm!!! end of range, normal operation for cool motor, no cooling. NO FAN required
  }

  return new_dutycycle_desired;
}



//-------------------------------------------------------------------------------
unsigned long Cooling_Policy3 (float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  static signed long new_dutycycle_desired=0;   // remember state internally, do not use dutycycle_desired for that.
  // ON/OFF RULES
  cooling_policy_func_nr_of_runs++;

// ignore errornous value(s) which might happen during (PWM noise) interference (-127.00)  and hold/return previous value (best guess)
if ( 
    (CoolingWaterTemp_Motor_OUT_InC == DEVICE_DISCONNECTED_C) || 
    (CoolingWaterTemp_Motor_IN_InC == DEVICE_DISCONNECTED_C)
   ) return(new_dutycycle_desired);


  // Non linear on/off controll , with too high escalation.

  // Higer temperature sensor overrule lower sensor logic if motor temperature get's higher then desired
  if (CoolingWaterTemp_Motor_OUT_InC > g_Cooling_Policy3__MotorOut_FanONTemp) {
    new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
  } else {

    // lower sensors
    if (CoolingWaterTemp_Motor_IN_InC > g_Cooling_Policy3__MotorIN_FanONTemp) {
      new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
    }
    if (CoolingWaterTemp_Motor_IN_InC < g_Cooling_Policy3__MotorIN_FanOffTemp) {
          new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
      }
  }

  // return new set value, or if not set, previous set value.
  return(new_dutycycle_desired);
}

//-------------------------------------------------------------------------------
unsigned long Cooling_Policy4 ( float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  signed long new_dutycycle_desired;
  // ON/OFF RULES
  cooling_policy_func_nr_of_runs++;

  // Cooling agressiveness for Motor in temperature depending on Motor Out temperature.
  // Non linear on/off controll with multiple temperature steps.

  if ( 100 <= CoolingWaterTemp_Motor_OUT_InC ) {
    new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
  }

  if ( (90 <= CoolingWaterTemp_Motor_OUT_InC) && (CoolingWaterTemp_Motor_OUT_InC < 100) ) {
    if (CoolingWaterTemp_Motor_IN_InC > 60) {
      new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
    }
    if (CoolingWaterTemp_Motor_IN_InC < 55) {
      new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
    }
  }
  if ( (85 <= CoolingWaterTemp_Motor_OUT_InC) && (CoolingWaterTemp_Motor_OUT_InC < 90) ) {
    if (CoolingWaterTemp_Motor_IN_InC > 65) {
      new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
    }
    if (CoolingWaterTemp_Motor_IN_InC < 60) {
      new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
    }
  }
  if ( (80 <= CoolingWaterTemp_Motor_OUT_InC) && (CoolingWaterTemp_Motor_OUT_InC < 85) ) {
    if (CoolingWaterTemp_Motor_IN_InC > 70) {
      new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
    }
    if (CoolingWaterTemp_Motor_IN_InC < 65) {
      new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
    }
  }
  if (  CoolingWaterTemp_Motor_OUT_InC < 80) {
    if (CoolingWaterTemp_Motor_IN_InC > 75) {
      new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
    }
    if (CoolingWaterTemp_Motor_IN_InC < 70) {
      new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
    }
  }


  return new_dutycycle_desired;
}




//-------------------------------------------------------------------------------
// trigger on high  temperature only
unsigned long Cooling_Policy_90 (float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  static signed long new_dutycycle_desired=0;   // remember state internally, do not use dutycycle_desired for that.
  // ON/OFF RULES
  cooling_policy_func_nr_of_runs++;

// ignore errornous value(s) which might happen during (PWM noise) interference (-127.00)  and hold/return previous value (best guess)
if ( 
    (CoolingWaterTemp_Motor_OUT_InC == DEVICE_DISCONNECTED_C) || 
    (CoolingWaterTemp_Motor_IN_InC == DEVICE_DISCONNECTED_C)
   ) return(new_dutycycle_desired);


  // Non linear on/off controll , with too high escalation.

  // Higer temperature sensor overrule lower sensor logic if motor temperature get's higher then desired
  if (CoolingWaterTemp_Motor_OUT_InC > 90) {
    new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
  } else {
    new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
  }

  // return new set value, or if not set, previous set value.
  return(new_dutycycle_desired);
}


//-------------------------------------------------------------------------------
// trigger on high  temperature only
unsigned long Cooling_Policy_85 (float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  static signed long new_dutycycle_desired=0;   // remember state internally, do not use dutycycle_desired for that.
  // ON/OFF RULES
  cooling_policy_func_nr_of_runs++;

// ignore errornous value(s) which might happen during (PWM noise) interference (-127.00)  and hold/return previous value (best guess)
if ( 
    (CoolingWaterTemp_Motor_OUT_InC == DEVICE_DISCONNECTED_C) || 
    (CoolingWaterTemp_Motor_IN_InC == DEVICE_DISCONNECTED_C)
   ) return(new_dutycycle_desired);


  // Non linear on/off controll , with too high escalation.

  // Higer temperature sensor overrule lower sensor logic if motor temperature get's higher then desired
  if (CoolingWaterTemp_Motor_OUT_InC > 85) {
    new_dutycycle_desired = PWM_MAX_DUTYCYCLE;
  } else {
    new_dutycycle_desired = PWM_MIN_DUTYCYCLE;
  }

  // return new set value, or if not set, previous set value.
  return(new_dutycycle_desired);
}



// Checks if a cooling policy is called (triggered by incident on 2018/08/08 where PWM value didn't get changed (when it should) for unknown reasons)
// return 
// true if OK
// false if not OK
bool Check_runs_Cooling_Policy(void) {

if(  cooling_policy_func_nr_of_runs == 0 ) {
  if ( g_debug > 0) {
    Serial.println("ERROR cooling_policy_func_nr_of_runs ==0 !!!!");
  }
  return false;
}

cooling_policy_func_nr_of_runs=0;  
return true;
}


unsigned long Cooling_Policy_update_failure_test(float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  // cooling_policy_func_nr_of_runs++;  // do not detect it has run...
  return dutycycle_desired; // no change
}

unsigned long Cooling_Policy_REBOOTME(float CoolingWaterTemp_Motor_OUT_InC , float CoolingWaterTemp_Motor_IN_InC)
{
  // cooling_policy_func_nr_of_runs++;  // do not detect it has run...
  //esp_restart_noos();
  ESP.restart();
  return dutycycle_desired; // no change
}

