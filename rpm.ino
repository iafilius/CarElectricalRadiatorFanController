/* This module computes the RPM,  
 * based on sampeling a single rotation on demand.
 * When a new measument is desired, a new /single measurement is started
 * 
 */

// Public global parameters related to this module

#define C_MAX_RPM_Sample_Time 2000
#define C_MIN_RPM_Sample_Time 250

volatile unsigned long int g_revolutions_measured;                  // used for calculating rpm using counting rotations
volatile uint32_t g_measured_rpm_period_us_t;                       // used for rpm computing based on period time measurement

//uint32_t g_rpm_measurement_time_previous=0; // time since last measurement

// END Public global paramters related to this module


// count measurement
void IRAM_ATTR rpm_fun_count(void) {
  //void rpm_fun(void) {
  g_revolutions_measured++;
  // Each interrupt/revolution this function is run once

#ifdef SHOW_LED_IN_rpm_fun
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  digitalWrite(led_PIN, ledState);    // toggle internal led just for visual control, the PWM control is executed (but it might be too fast to see blinking)
#endif

}


// time measurement
void IRAM_ATTR rpm_fun_time(void) {
  //void rpm_fun(void) {
  static uint32_t prev_t=micros();
  uint32_t current_t=micros();
  uint32_t period_t = current_t - prev_t;   
  
  
  // Each interrupt/revolution this function is run once


  g_measured_rpm_period_us_t=period_t;
  prev_t=current_t;

#ifdef SHOW_LED_IN_rpm_fun
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  digitalWrite(led_PIN, ledState);    // toggle internal led just for visual control, the PWM control is executed (but it might be too fast to see blinking)
#endif

}


void rpm_measurement_setup(void) {
  if (digitalPinToInterrupt(Tacho_PIN) < 0 ) {
    Serial.println(F("ERROR: you selected a no valid pinnumber for Tacho_PIN"));
    delay(5000);  // make user notice
  } else {

    //pinMode (Tacho_PIN, INPUT);
    //pinMode(Tacho_PIN, INPUT_PULLUP);
    pinMode(Tacho_PIN, INPUT);

    //digitalPinToInterrupt(Tacho_PIN);
    attachInterrupt(digitalPinToInterrupt(Tacho_PIN), rpm_fun_time, RISING); // Interrupt 0 = PIN2 ??
    //attachInterrupt(Tacho_PIN,rpm_fun, RISING); // Interrupt 0 = PIN2 ??
    Serial.print(F("rpm_fun() attached to interrupt: "));
    Serial.println(digitalPinToInterrupt(Tacho_PIN));

  }


}



// measure interrupts as revolutions => RPM
//

/*
  void handle_rpm_events(void) {
  static uint32_t g_rpm_measurement_time_previous;
  uint32_t current_time;

  //uint32_t g_revolutions_measured_frozen;

  // Keep critical calues close to prevent interrupt introduce jitter
  current_time=millis();


  if( (current_time - g_rpm_measurement_time_previous) > 1000 ) {    // only once in some condition to update counter. time in this case, or number of revolutions??

    // keep thise values as a pair
    //current_time=millis();
    current_time=millis();    // better accuracy??
    noInterrupts();
    g_revolutions_measured_frozen=g_revolutions_measured;   // take value from ISR
    g_revolutions_measured=0;                               // Reset value of ISR to start over again.
    interrupts();

    if(g_debug>DEBUGLEVEL1)Serial.println(F("handle_rpm_events handled"));

    // force UL (Unsignedd Long to prevent overflow
    int rounding = (current_time - g_rpm_measurement_time_previous)/2;  // add half period as it chops value, and does not round.
    g_calculated_rpm=(1000UL*g_revolutions_measured_frozen + rounding)/(current_time - g_rpm_measurement_time_previous); // convert  millis first
    // TODO milis to micros conversion...
    //g_rpm_measurement_time_previous=current_time;
    g_rpm_measurement_time_previous=millis();     // forget current_time, we would like to skip calculation time and start meauring from now on again.


  // https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
  // http://gammon.com.au/interrupts
  //attachInterrupt(0,rpm_fun, RISING);
  }
  }
*/



// dynamic sample time adjustment, for making low RPM stable and more acurate
void handle_rpm_events_count(void) {
  static uint32_t g_rpm_measurement_time_previous;
  static int sample_time = C_MIN_RPM_Sample_Time;

  uint32_t current_time;

  //uint32_t g_revolutions_measured_frozen;

  // Keep critical calues close to prevent interrupt introduce jitter
  current_time = millis();


  if ( (current_time - g_rpm_measurement_time_previous) > sample_time ) {   // only once in some condition to update counter. time in this case, or number of revolutions??

    // keep thise values as a pair
    //current_time=millis();
    //current_time=millis();    // better accuracy??
    noInterrupts();
    current_time = millis();  // better accuracy??
    g_revolutions_measured_frozen = g_revolutions_measured; // take value from ISR
    g_revolutions_measured = 0;                             // Reset value of ISR to start over again.
    interrupts();

    if (g_debug > DEBUGLEVEL1)Serial.println(F("handle_rpm_events handled"));

    // force UL (Unsignedd Long to prevent overflow
    int rounding = (current_time - g_rpm_measurement_time_previous) / 2; // add half period as it chops value, and does not round.
    g_calculated_rpm = (1000UL * g_revolutions_measured_frozen + rounding) / (current_time - g_rpm_measurement_time_previous); // convert  millis first
    // TODO milis to micros conversion...
    //g_rpm_measurement_time_previous=current_time;
    g_rpm_measurement_time_previous = current_time;
    // forget current_time, we would like to skip calculation time and start meauring from now on again.


    // target for getting 100 counts per measurement
    if ( (1 < g_revolutions_measured_frozen) ) {  // 1 to avoid dive by zero
      sample_time = sample_time * 300 / g_revolutions_measured_frozen;
    } else {
      sample_time = C_MAX_RPM_Sample_Time;
    }
    if ( sample_time > C_MAX_RPM_Sample_Time) {
      sample_time = C_MAX_RPM_Sample_Time; //max
    }
    if ( sample_time < C_MIN_RPM_Sample_Time ) {
      sample_time = C_MIN_RPM_Sample_Time; // min
    }
    //Serial.println(sample_time);
    // https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
    // http://gammon.com.au/interrupts
    //attachInterrupt(0,rpm_fun, RISING);


  }
}





// take last measured period (and ignore potential previous meaured periods)
// perhaps more acurate is measure time over longer time (as often this function is called) 
// micros() = 2^32 -1 = 4.294.967.296-1
//     1 RPM  =>           60.000.000     uS (per revolution),   +/- 1uS => +/- 1/60.000.000 *1 RPM
//   600 RPM  =>              100.000     uS (per revolution),   +/- 1uS => +/- 1/100.000 * 600 RPM
//  4500 RPM  =>               13.333,333 uS (per revolution),   +/- 1uS => +/- 1/13.333,333 * 4500 RPM

void handle_rpm_events_time(void) {

// 60.000.000 => 
//    1.000.000 => from uS to seconds 
//    60 => from 1s 60 seconds

  if(g_measured_rpm_period_us_t != 0) {
    g_calculated_rpm = 60000000.0/g_measured_rpm_period_us_t;   // for 1 revolution
  } else {
    g_calculated_rpm=0;     // no measurement yet, set it to 0
  }
  if (g_debug >= DEBUGLEVEL2 ) {
    Serial.print("g_measured_rpm_period_us_t: ");
    Serial.println(g_measured_rpm_period_us_t);
  }

// TODO middle over multiple revolutions:
//g_calculated_rpm = 60000000.0/g_measured_rpm_period_us_t (*#revolutions;   // for #revolution
 
}


