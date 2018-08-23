// TODO Manual perio setting!!!!!!

// Public global parameters related to this module

// END Public global parameters related to this module


#define C_Control_Timer_Period_us_no_slowstart  100000  // 10 times a second to check if fan needs to go on or off seems reasonable to me, adjust to your taste. (yes this is a bit hidden);


//=========================
void update_PWM_dutycycle_desired(void) {

  unsigned long new_dutycycle_desired = 0;  // set to 0 to avoid possible uninitialized value..

  if (g_operating_mode != C_OPERATING_MODE_MANUAL) {

    // check if #elements matches index
    if( (sizeof(CoolingFunc_arr)/sizeof(CoolingFunc_t) ) < (g_Temperature_PWM_Policy+1) ) {
      // for some reason got out of range...
      Serial.println("g_Temperature_PWM_Policy too large for #elements, setting to zero!!!");
      g_Temperature_PWM_Policy=0;
    }

    new_dutycycle_desired = CoolingFunc_arr[g_Temperature_PWM_Policy].CF_ptr(g_CoolingWaterTemp_Motor_OUT_InC, g_CoolingWaterTemp_Motor_IN_InC);
    /*
    switch (g_Temperature_PWM_Policy) {
      case 0:
        // Always OFF policy
        new_dutycycle_desired = Cooling_Policy0(g_CoolingWaterTemp_Motor_OUT_InC, g_CoolingWaterTemp_Motor_IN_InC);
        break;
      case 1:
        // Always ON policy
        new_dutycycle_desired = Cooling_Policy1(g_CoolingWaterTemp_Motor_OUT_InC, g_CoolingWaterTemp_Motor_IN_InC);
        break;
      case 2:
        break;
      case 3:
        // Controlling Radiator Out Temperature with Motor Out temperature to high escalation
        new_dutycycle_desired = Cooling_Policy3(g_CoolingWaterTemp_Motor_OUT_InC, g_CoolingWaterTemp_Motor_IN_InC);
        break;
      case 4:
        new_dutycycle_desired = Cooling_Policy4(g_CoolingWaterTemp_Motor_OUT_InC, g_CoolingWaterTemp_Motor_IN_InC);
        break;
      case 5:
        // Future Policy
        break;
      default:
        break;
    }
    */
  } else {
    new_dutycycle_desired = g_completely_manual_motor_control_pwm_dutycycle;  // overrule with manual value
  }


  // make sure new value does not get read just while updating value
  if (dutycycle_desired != new_dutycycle_desired) {
#ifdef __AVR__
    noInterrupts();
#endif
#ifdef ESP32
    portENTER_CRITICAL(&dutycycle_desired_mux);
#endif
#ifdef ESP32
    dutycycle_desired = new_dutycycle_desired;
#endif
    portEXIT_CRITICAL(&dutycycle_desired_mux);
#ifdef __AVR__    
    interrupts();
#endif

  }
}






// _period in micro seconds
// Recommended is around 15kHz (66uS), depending on fan, switching electronics, and possible radio interference
// TODO: needs to be able to set mananual override   g_completely_manual_motor_control_pwm_period_us/g_operating_mode=MANUAL/
void set_PWM_Period_us(int _period) {

    ESP_LOGE("TAG","Debug...");

  // Check no value found using Preferences libarary
  if ( _period == NO_VALUE ) {
    Serial.println("period= < 0  no  action to perform");
    return;
  }

  if ( _period == 0 ) {
    Serial.println("ERROR period=0 cowardly refusing a device by zero");
    return;
  }

#ifdef __AVR__
  Timer1.initialize(_period);  // Timer1 for PWM @15KHz
  // need to re-assign PWM, else no output after re-initialize
  // Accepted risk :  this command isn't executed right after ending one period, so one period might be completely wrong.
  Timer1.pwm( PWM_PIN_OUT, __dutycycle_actual);   // restore to current dutycyle and pin assignment for seemless transition as possible.
#endif
#ifdef ESP32
  int _freq = 1000000 / (_period); // global varabele...
  //g_period=_period;
  //Serial.print("Setting PWM frequency to : ");
  //Serial.println(_freq);
  ledcSetup(g_ledChannel, _freq, PWM_resolution);   // TOCHECK, not sure if re-configureatin works..
  //ledcAttachPin(PWM_PIN_OUT, g_ledChannel);
  ledcWrite(g_ledChannel, __dutycycle_actual);         // and re-apply g_dutyCycle (or else no output)

  //Serial.println("DEBUG 0100");

#endif

  /* Function is agnostic for maual/automatic as paramters might changed without operating in that mode.
    if(g_operating_mode!=C_OPERATING_MODE_MANUAL) {
       g_completely_manual_motor_control_pwm_period_us=_period; // remember value (manual) for future display
    } else {
      g_PWM_Period=_period;    // remember value for future display and persist storage
    }
  */
  // Possible Timer3 values and stepsize needs adjustment after adjusting timer1 (PWM) period.
  set_pwm_control_timer(g_FAN_SLOWSTART_TIME);

}


// Timer 3 PWM step change frequency
// _slowstart in seconds.
// Standard (PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE) steps are taken (stepsize=1)
// So Timer3-Period should be > Timer1-Period * #steps (PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE)
// with parameters 1 and higher this should cause no problems unless Timer1-Period is set to unwise low frequency (< 1024Hz)
// when set to 0, when no protection is Timer3 ISR consumes all resources, watchdog kicks in and MCU resets's.
// when period set to 0, I assume you want to disable soft change characteristics.
//   - Assumed is , no softstart is desired (perhaps for minimizing radio interference)
//   - stepsize is set to PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE
//   - softchange becomes hard-change
void  set_pwm_control_timer(int _slowstart) {

    ESP_LOGE("TAG","Debug...");

  //return;

  if (_slowstart == 0) {  // so fan motor is not given any time to start, motor needs to start at once (like a relay)
    // Excption 0
    // Always print a warning
    Serial.println(F("WARNING: setting slowstart to '0', disabeling softstart with this value"));
    g_FAN_SLOWSTART_TIME = _slowstart;
    g_pwm_control_timer_period_us = C_Control_Timer_Period_us_no_slowstart ;
    g_pwm_soft_step = PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE;

  } else {
    if (  (_slowstart * 1000000 / (PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE))  <  g_PWM_Period ) {
      // Exeption: not enough time/steps
      Serial.println("Not enough time/steps....");

      // standard #steps PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE with stepsize of 1 does not fit requirements (useless/undesired to change PWM each (or multiple times per) period
      // given _slowstart ... adjusting  stepsize, and therefore #steps required
      //g_pwm_soft_step
      // #steps= (_slowstart*1000000) / g_PWM_Period;
      // #stepsize = PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE / #steps
      // period = g_PWM_Period * #steps

      int _nr_steps = (_slowstart * 1000000) / g_PWM_Period;
      g_pwm_soft_step = (PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE) / _nr_steps ;
      g_pwm_control_timer_period_us = g_PWM_Period * _nr_steps;


      Serial.print("Number of steps: ");
      Serial.println(_nr_steps);
    } else {
      // Normal/Desired operation.... (1/1024 th of Timer1 period)
      //Serial.println("Normal operation....");
      g_FAN_SLOWSTART_TIME = _slowstart;
      g_pwm_control_timer_period_us = g_FAN_SLOWSTART_TIME * 1000000 / (PWM_MAX_DUTYCYCLE - PWM_MIN_DUTYCYCLE);
      g_pwm_soft_step = 1;
    }
  }


#ifdef __AVR__
  Timer3.initialize(g_pwm_control_timer_period_us);
#endif
#ifdef ESP32
  /*
     Debug....
    Serial.print("g_pwm_control_timer_period_us :");
    Serial.println(g_pwm_control_timer_period_us);

    Serial.print("_slowstart :");
    Serial.println(_slowstart);

    Serial.print("g_PWM_Period :");
    Serial.println(g_PWM_Period);

    Serial.print("g_FAN_SLOWSTART_TIME :");
    Serial.println(g_FAN_SLOWSTART_TIME);

    Serial.print("PWM_MAX_DUTYCYCLE :");
    Serial.println(PWM_MAX_DUTYCYCLE);

    Serial.print("PWM_MIN_DUTYCYCLE :");
    Serial.println(PWM_MIN_DUTYCYCLE);
  */

  timerAlarmWrite(pwm_control_timer, g_pwm_control_timer_period_us, true); //set time in us, 3th paramter true=repeat!!
  //  Serial.println("DEBUG");
  //  delay(5000);
#endif
  // No pin nor PWM is used
  // Timer3.pwm....
}



