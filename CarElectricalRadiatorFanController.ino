/*
 *  PWM car radiator fan control, as addon or repacement for thermostat swich.
 *  
 *  Introduction:
 *   Triggered by the thread on dubbellucht.nl to replace the mechanical radator fan on an oldtimer van for an electical one to gain some power/save some fuel and produce less noice.
 *   But instead fixed location/fixed temperature - thermostat switch with on/off characteristics, i started working on a project using multiple temperature sensors, and microsprocessor controlled PWM Fan motor control.
 *   Started with an arduino leonardo 32U4 micro board, wich worked fine, but soon ran into available memory issue which limited development (even after memory tuning with strings to EEPROM etc).
 *   Switched to the ESP32 platform, with "lots" of memory and lots of integrated options, basically a few generations newer platform.
 *   Alltough it still compiles for leonardo/32U4, development has stopped due lack of space, and lost interrest.
 *   I use the The ESP-WROOM-32 development board , available for about $5 with shipment.
 *   Idea was using both Bluetooth AND WiFi, but abandonned/(put on hold) bluetooth for 3 reasons:
 *    - it eats a real _HUGE_ amount of flash, and needed to repsrtition the 4MB flash
 *    - it eats a quite part of available memory as well (which led to frequent crashes)
 *    - Need to write a native app, and IOS development still can't be doen without investing (money and time)
 *   Shifted focus from serial console to WIFI/webbased, and most importand item are available .. on you phone...
 *   So these days WiFi/webapp is the main focus, and in Cockpit view you can monitor the behavious realtime of the temperatures, and fan status, and you can change the policy/function on the fly from a webbased menu.
 *   After some several firmware updates, I enabled firmware updates over wifi, as the USB port is hidden/water proofed inside the motor room, and my laptop contains policies which make native OTA imposible.
 *  
 *  History:
 *  1   First operational and installed version (ESP32) 2018/07/15
 *   2018/08/17 recompiled with async tcp commit 5453ec2e3fdd9beb92b8423878008b57be7805e6 for some tcp_close related stability issues
 *   2018/08/20  Fixed crashes with ledcWrite (from timerinterrupt) by switching to  ledc_set_duty_and_update() is is documented thread safe
 *  
 *  
 *  Features:
 *  multi platform (Arduino AVR (ex Leonardo), Arduino ESP32 (ex Doit Devkit V1), features depend on platform and available memory
 *  Configureable Fan Softstart (time) (AVR/ESP32)
 *  Support Dallas DS18B20 temperature sensors
 *    - One designed to be in upper radator hose usinng a well
 *    - One designed to be in lower radator hose
 *    - Selectable resolution 
 *  Oled display (AVR/ESP32)
 *  Persistent parameters (store to NVRAM/EEPROM)
 *  USB/Serial console access
 *    -logging
 *    -dump actual/startup configuration to screen
 *    -setup/configure parameters
 *   Multiple cooling policies/functions
 *    -Realtime change cooling function/policy (either USB/serial or via WIFI (ESP32 only))
 *    -Easy to add you own cooling function compile time.
 *   WiFI (ESP32 only)
 *    webserver:
 *      -Cockpit view, with realtime data using SSE
 *      -Setup page
 *      -debug page
 *      -firmware update page
 *      -does not advertise a default route, and does not disable 3/4G access
 *   RPM measurement
 *    -basic RPM measurement support, for future integration in cooling functions/policies
 *   Firmware upgrade options
 *    -USB (AVR & ESP32) through Arduino IDE
 *    -OTA (WIFI) (ESP32) through Arduino IDE, native OTA but needs rights on client for opening up ports, may not work on corporate workstation, even with elevated rights.
 *    -Webserver (WIFI) (ESP32), use file upload from any browser and choose bin file from <profiledir>\AppData\Local\Temp\arduino_build_296723\*.bin. no specefic rights on laptop required.
 *   Watchdog
 *    -
 *   Service announcement through Bonjour/MDNS on WiFi (ESP32)
 *    - use Bonjoursearch (IOS)
 *    
 *    Hardware list:
 *    -Biggest/strongest electrical radiator fan you can fit/buy
 *    -Fet Module designed to switch PWM/inductive loads (ex DS16 PWM Motor Speed Controll controller))
 *    -Fet Driver IC (ex MIC4422YN)
 *    -a mini switching power supply for microcontroller module (generating 7 V (or 5V or 3,3 at your choice), accepts 8-24V)
 *    -Arduino module (eg Leonaro), or ESP wroom-32 Dev board, or just the ESP wroom-32 module itself
 *    -DS18B20 temperature sensors with stainless protection tube
 *    -Thermowell for inserting the DS18B20 sensors
 *    -thermal grease for inserting the sensors into the thermowell
 *    -radiator hose temperature sensor adapter (get biggest diameter you can fit, eg 42mm) (to insert the termowell)
 *    -2 short pieces radiator hoses, (when you don't want to cur the original hoses)
 *    
 *    Schema: ToDo in KiCad
 *    
 *    Software Module dependency:
 *    -ESP32 core for arduino (ESP32)
 *    -TimerOne/TimerThree (AVR)
 *    -SSD1306Ascii (AVR)
 *    -SSD1306 (ESP32)
 *    -SSD1306AsciiAvrI2c
 *    -OneWire
 *    -DallasTemperature
 *    -AsyncTCP (ESP32/WiFI/webserver) (need to download from github)
 *    -ESPAsyncWebServer (need to download from github)
 *    -WiFiManager ESP32 (planned ESP32)
 *    
 *    Buildin policies:
 *    -fan permanent off
 *    -fan permanent on
 *    -policy trying to keep some thermal capacity reserve
 *    -policy to turn fan on above a certain temperature (like 90 degrees celsius)
 *    <put your smarter funcitons here>
 *    
 *    
 *    Issues:
 *    -It looks like the ESP32 platform (or any platform??) has difficulties with String + operations.
 *     in the main pages  String.reservation is used, which prevents most common type of crash. 
 *    -Common type of crash? yes, unfortunately the tcp stack seems to crash too often on tcp_close(), as quite many people seem to have similar issues, this hasn't been adressed yet.
 *    -CSS menu is blasing fast, and it is difficult NOT to press a menu item at menu popup time.
 *    -CSS menu's pop-up perfectly fine location on IOS, but firefox/chrome (windows) the you can't reach the menu with the mouse (which i do not use in the car anyway...)
 *    -The presented temperatures in browser leave some reservation for capitals and underline characters, otherwise stated, the numbers do not fill the complete cell, and eat some space which could be used for an other purpose 
 *    -RPM isn't currently used, basically due to lack of acuracy. 
 *      when measuring pusels a second:1 pulse/second => 60RPM accurary is always +/ 60RPM
 *      When measuring the interval time based on interrupts, a huge jitter is observed.
 *      Ideal: RPM is calculated directly form the time betwen to (or more) pulses 
 *    -2018/08/08 experienced the PWM counter value dit not seem to turn on, not in SSE and did not hear the fan. manual turning on dit als not work.
 *      Need a online/wifi based syslog solution......
 *    -2018/08/21 While using "Cockpit" in simulation mode and having a non-perfect WiFi connectin (distance/metal motor compartment), the watchdog didn't feed the dog in  tim e(8000ms)
 *      -So when stalling the main loop... the functionality is heavily under attack!! needs to be fixed.
 *      -As workaround a fix for thottling SSE updates is implemented and constant C_SSE_Update_throttle_time_ms, and works fine under normal conditions. But a simulated WIFI out of reach test still blocks and hangs the system!!!
 */
// For leonardo/beetle or ESP32
// 2018/07/17 started to rework String class into C strings due to frequent crashes on AsyncHttpServer

#ifdef ESP32
#include <Arduino.h>      // defines ESP32 specifics, worked perfectly without
//#include "adc.h"

#include <sys/time.h>
#include <sstream>
#include <string>
//#include <C:\Users\m99i941\Arduino\hardware\espressif\esp32\tools\sdk\include\lwip\apps\dhcpserver_options.h>
#endif
//#define C_STR_BUFSIZE 20    // short buffer for display, to be used witrh snprintf etc.
#define C_STR_BUFSIZE 100    // short buffer for display, to be used witrh snprintf etc.

// watchdog includes (can't be moved to watchdog module
#ifdef __AVR__
#include <avr/wdt.h>
#endif


#define LEDCWRITE_SUSPECTEDBUG_WORKAROUND   


#define NO_VALUE  -1  // value set when paramter is given but had no value set

#define C_DEBUG   0     // 0, 1, 2 ,3 4 ...
#define DEBUGLEVEL1   1
#define DEBUGLEVEL2   2


//#define SHOW_LED_IN_rpm_fun     // uncomment to attach buildin led to rpm_fun, defaults to  PWM_control_Timer_callback
#ifndef SHOW_LED_IN_rpm_fun
#define SHOW_LED_IN_PWM_control_Timer_callback
#endif
/* Using  CJMCU BArduino Beetle (clone) for PWM controlled Radiator Fan
    Measuring temperature sensort on Upper radiator input and lower radiator output to calculate the desired Radiator PWM dutycycle.
    Set PWM frequenecy to fixed 15KHz
    Calculate DS18S20 time consumed reading bus.
    Using 1306 Oled display
    Reading voltage Lambda sensors
    set PWM dutycycle from 0-1024 depending on temperature(s) or manual override (before/while mountain climb)

    Using MIC4422YN as powerFET Driver
    Using Modified 60A PWM FET 60A board (disabled onboard PWM, and connected output of MIC4422YN to FET gates.
    DS18S20 based  waterproofed/shielded sensors.
    Radiator hose temperature sensor adapter. (42mm, the maximum) used 11,4mm(or 11,8) drill and G1/4 tap to make the thread.
    20mm length hotpot (G1/4 thread) in sensor adapter
    Teflon tape to seal G1/4 thread from hotpot into sensor adapter.



  TimerOne
  0 ==  Low
  1 == LOW (possibel due to rounding errors)
  1023 != high !!!!
  1024 == High !!!!
  512== 50% exact


*/



// END EEPROM stored variable definitions

// Where to story what
// http://forum.arduino.cc/index.php?topic=4697.0
#define C_EEPROM_SCHEMA_IDX                                 0
#define C_EEPROM_PROGVER_IDX                                1
#define C_Temperature_PWM_Policy_IDX                        2
#define C_FAN_SLOWSTART_TIME_IDX                            3

#define C_Cooling_Policy3__MotorOut_FanONTemp_IDX           4
#define C_Cooling_Policy3__MotorIN_FanONTemp_IDX            5
#define C_Cooling_Policy3__MotorIN_FanOffTemp_IDX           6
#define C_Dallas_Resolution_IDX                             7

#define C_Dallas_Synchronous_IDX                            8
#define C_PWM_Period_IDX                                    9
//const byte  C_PWM_Period_IDX=9;
#define C_display_update_time_IDX                           10
#define C_Dallas_Default_Ordering_IDX                       11

#ifdef ESP32
#define C_preference_key_len_max                  15    // property of preferences library   
char Preferences_keys[][C_preference_key_len_max + 1 ] = {
  "schema",           // C_EEPROM_SCHEMA_IDX
  "progversion",      // C_EEPROM_PROGVER_IDX
  "pwm_policy",       // C_Temperature_PWM_Policy_IDX
  "fan_slowstart_t",  // C_FAN_SLOWSTART_TIME_IDX
  "cp3mo_fanon_t",    // C_Cooling_Policy3__MotorOut_FanONTemp_IDX
  "cp3mi_fanon_t",    // C_Cooling_Policy3__MotorIN_FanONTemp_IDX
  "cp3mi_fanoff_t",   // C_Cooling_Policy3__MotorIN_FanOffTemp_IDX
  "dallas_res",       // C_Dallas_Resolution_IDX
  "dallas_synchr",    // C_Dallas_Synchronous_IDX
  "pwm_period",       // C_PWM_Period_IDX
  "displ_upd_t",      // C_display_update_time_IDX
  "dallas_order"      //  Default Order == true, false = reversed
};
#endif

// Initial/program values  (note 1 byte only! (0-255 0-0xFF)
#define C_EEPROM_SCHEMA           0x42         // change Schema when EEPROM structure changes to much it would no longer be compatible.
#define C_PROGVERSION             0X04
#define C_Temperature_PWM_Policy  3            // (See case statement using different cooling policies
#define C_FAN_SLOWSTART_TIME      10          // Desired Soft Start FAN in Seconds from zero to max, adjust to your needs/desires

#define C_Cooling_Policy3__MotorOut_FanONTemp  90
#define C_Cooling_Policy3__MotorIN_FanONTemp   80
#define C_Cooling_Policy3__MotorIN_FanOffTemp  79
#define C_Dallas_Resolution  12   // program default

#define C_Dallas_Synchronous  false // synchrone or asynchrone
#define C_PWM_Period_us   66    // 66 uS => 15.151KHz
#define C_display_update_time 250    // every xx ms an update when called.
#define C_Dallas_Default_Ordering   true  // default dallas order

// Schema
// progversion
int g_Temperature_PWM_Policy;   // needs to be read from EEPROM or program default.
int g_FAN_SLOWSTART_TIME;
// derived values
unsigned long int  g_pwm_control_timer_period_us ;  // derived from C_FAN_SLOWSTART_TIME
unsigned int g_pwm_soft_step;                     // set to 1, and change_period_us is set such, that it takes PWM_MAX_DUTYCYCLE steps to reach the max value.

int g_Cooling_Policy3__MotorOut_FanONTemp;
int g_Cooling_Policy3__MotorIN_FanONTemp;
int g_Cooling_Policy3__MotorIN_FanOffTemp;
int g_Dallas_Resolution;

bool g_Dallas_Synchronous;        // synchrone or asynchrone.
unsigned int g_PWM_Period;  // Default, but might change when desired.
unsigned int g_display_update_time;    // every xx ms an update when called.
bool  g_Dallas_Default_Ordering;      // true = default order, false = revere

// END EEPROM/NonVoilatible memory stored variable definitions
// the actual/running values
// schema
// progversion


// Non persistent variables
int g_debug = C_DEBUG;                              // global debug


const int C_OPERATING_MODE_AUTOMATIC  = 1;          // used conmstant instead of define to work in strings....
const int C_OPERATING_MODE_SIMULATED  = 2;
const int C_OPERATING_MODE_MANUAL     = 3;
int   g_operating_mode  = C_OPERATING_MODE_AUTOMATIC;     // default automatic, after each boot (non persistent by design)

unsigned int g_completely_manual_motor_control_pwm_dutycycle = 0;    // Set dutycycle (0-1024)
unsigned int g_completely_manual_motor_control_pwm_period_us = C_PWM_Period_us; // set periof of PWM in uS (66 = 15kHz)
//Others related simulated???

//#define SIMULATED_MOTORTEMP 0     // to be used as  index for g_simulate_values
//#define SIMULATED_RADATORTEMP 1   // to be used as  index for g_simulate_values
#define MotorTemp_IDX     0       // used in device array and device error array
#define RadiatorTemp_IDX  1       // used in device array and device error array
float g_simulate_values[2];               // values which are "read" when g_simulate_temperature_sensors == true


// End initial values



#ifdef __AVR__
// https://www.pjrc.com/teensy/td_libs_TimerOne.html

// Timer1 = PWM
// Timer3 = Soft update PWM (Softstart/soft-change PWM)

#include <TimerOne.h>
#include <TimerThree.h>
#endif

#ifdef ESP32
#include <driver/ledc.h>    // ledc_set_duty_and_update()
int g_ledChannel = 2;   // PWM
hw_timer_t * pwm_control_timer = NULL;
int C_PWM_CONTROL_HW_TIMER_NR = 1;
int C_WDT_HW_TIMER_NR = 0;  // and ledchannel is  connected to hw timer xxx???

// LEDC uses folowing mapping to timers:
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-ledc.c

/*
 * LEDC Chan to Group/Channel/Timer Mapping
** ledc: 0  => Group: 0, Channel: 0, Timer: 0
** ledc: 1  => Group: 0, Channel: 1, Timer: 0
** ledc: 2  => Group: 0, Channel: 2, Timer: 1
** ledc: 3  => Group: 0, Channel: 3, Timer: 1
** ledc: 4  => Group: 0, Channel: 4, Timer: 2
** ledc: 5  => Group: 0, Channel: 5, Timer: 2
** ledc: 6  => Group: 0, Channel: 6, Timer: 3
** ledc: 7  => Group: 0, Channel: 7, Timer: 3
** ledc: 8  => Group: 1, Channel: 0, Timer: 0
** ledc: 9  => Group: 1, Channel: 1, Timer: 0
** ledc: 10 => Group: 1, Channel: 2, Timer: 1
** ledc: 11 => Group: 1, Channel: 3, Timer: 1
** ledc: 12 => Group: 1, Channel: 4, Timer: 2
** ledc: 13 => Group: 1, Channel: 5, Timer: 2
** ledc: 14 => Group: 1, Channel: 6, Timer: 3
** ledc: 15 => Group: 1, Channel: 7, Timer: 3
*/

#endif



#ifdef ESP32

//#define USE_BLE_SERVER    // BLE is HUGE (900K!!) needs most likely repartitioning to create more space
#define USE_HTTP_SERVER    // comment to use switch between WIFi or BLE (prevent too much memory consumtion)
#endif


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

//#endif


// From docu:
//Board   TimerOne TimerThree
//        PWM Pins PWM Pins
//        9,10,11   5



// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
int ledState = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables

//#include <stdio.h> // for function sprintf

// Data wire is plugged into pin D10 on the Arduino


// PWM motor/FAN controller output on D9
//
#define  analogInput_PIN    A0     // A0
// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
#ifdef __AVR__
#define Tacho_PIN           0    // Last free labelled port D11 isn't supported for interrupts,  'RX' maps to digital PIN0
#endif
#ifdef ESP32
#define Tacho_PIN           4     // GPIO 4 easy to find on (2 in BUILDIN_LED!!) 
#endif

#ifdef __AVR__
#define PWM_PIN_OUT         9     // D09 Timer1 output (PWM)
#endif
#ifdef ESP32
// D05/GPIO5 has pullup resistor in DOIT DEVKIT v1 !!!, not completely suitable for PWM when not initialized.....
#define PWM_PIN_OUT         18     // GPIO18 with pull down resistor monted!!!!, D05  output (PWM)  (D09 is not external connected on dev board)
#endif

#ifdef __AVR__
#define ONE_WIRE_BUS_PIN    10    // D 10 
#endif
#ifdef ESP32
#define ONE_WIRE_BUS_PIN    15    // GPIO 15 was easy to find...
#endif


#ifdef ESP32
#define SDA1_PIN            21
#define SCL1_PIN            22
#endif
const int led_PIN = LED_BUILTIN;  // the pin with a LED (D02??)



//unsigned long time;
byte seconds ;
byte minutes ;
byte hours ;

// BEGIN BLE related definitions
#ifdef ESP32

#ifdef USE_HTTP_SERVER
#undef USE_BLE_SERVER     // don't use BLE when HTTP/WiFI is configured)
#endif

#endif
// END BLE related definitions

// BEGIN WEB related definitions
#ifdef ESP32
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// https://github.com/me-no-dev/ESPAsyncWebServer#async-event-source-plugin
AsyncEventSource events("/events");     // html5 server stream events ... needs to be global as it might called from several locations
#endif

// END WEB related definitions

// BEGIN WiFi related definitions
#ifdef ESP32
#include <WiFi.h>
#include <SPI.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>


//#include <WiFiManager.h>  
#include <PersonalIncludes.h>   // Located in libaries/

#ifndef WIFI_PASS
#define WIFI_PASS "0123456789"
#endif
#ifndef WIFI_SID
#define WIFI_SSID  "WiFISIDasClient"
#define WIFI_Server_SSID  "MyESP"
#endif

bool shouldReboot=false;    // set by /update in async http server

static const int spiClk = 1000000; // 1 MHz
//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;
uint16_t toTransfer;

#endif
// END WiFi related definitions



// BEGIN RPM related definitions
unsigned long int g_calculated_rpm = 0;           // calculated rpm, curently revolutions/second....
unsigned long int g_revolutions_measured_frozen;  //

// END RPM related definitions

// BEGIN volt measurement related defintions
float vout = 0.0;
float vin = 0.0;
float R1 = 100000.0; // resistance of R1 (100K) -see text!
float R2 = 10000.0; // resistance of R2 (10K) - see text!
int value = 0;
// END volt measurement related defintions



// BEGIN display related definitions
//https://techtutorialsx.com/2017/12/02/esp32-arduino-interacting-with-a-ssd1306-oled-display/

#include <Wire.h>

//#include <SPI.h>

// 1306 OLED
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

#ifdef __AVR__
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

SSD1306AsciiAvrI2c oled;

#endif


#ifdef ESP32
// https://techtutorialsx.com/2017/12/02/esp32-arduino-interacting-with-a-ssd1306-oled-display/
#include "SSD1306.h"

// SSD1306 is actually typedeffed to:  SSD1306Wire  (#include "SSD1306Wire.h")
SSD1306  display(I2C_ADDRESS, SDA1_PIN, SCL1_PIN);

#endif

// END display related defintions

// BEGIN Dallas temperature sensor related definitions

#include <OneWire.h>
#include <DallasTemperature.h>

// Using Dallas DS18S20 1-Wire Digital Thermometer
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS_PIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold DS18S20 Digital Thermometer device addresses
//DeviceAddress MotorOutThermometer, RadiatorOutThermometer;
DeviceAddress DallasDevices[2];   //  [MotorTemp_IDX, RadiatorTemp_IDX  ]
int DallasMissedCounters[2]={0,0};      // count how many times a -127 degrees shows up.


volatile float g_CoolingWaterTemp_Motor_OUT_InC = -999;   // out or range, make sure it gets noticed when not updated
volatile float g_CoolingWaterTemp_Motor_IN_InC  = -999;   // out or range, make sure it gets noticed when not updated

// default for now.
uint32_t g_dallas_conversion_time = 750;     // how long we should wait to ask for an answer, calculated in setup or when dallas_resultion changes
bool  g_ReadTemperature = false;        // at start no value is read

// END Public global paramters related to this module

uint32_t  t_dallas_sensors;       // mesasures the time taken for each temperature  prope


// END Dallas temperature sensor related definitions


// PWM related definitions

// Resolution is used by Cooling Policy as well..
#define PWM_resolution    10    // Dynamic with ledC/EPS32, fixed with Timer1 2^10=1024 is 100%
#define PWM_MAX_DUTYCYCLE (1<<PWM_resolution)  //  2^10   // Timer1,3 PWM maximum for 100% @1024
#define PWM_MIN_DUTYCYCLE 0     // 0% PWM

portMUX_TYPE  dutycycle_desired_mux = portMUX_INITIALIZER_UNLOCKED; // to protect dutycycle_desired
volatile unsigned long dutycycle_desired = 0; // The desired state , set by Temperature/Cooling Policy
volatile unsigned long __dutycycle_actual = 0; // Internal state of the actual (soft-change) PWM state, might be read for informaitonal purposes


// Arduino does not accept to move This function to a separate file or to below....
// so we do it here then.

#ifdef ESP32
void IRAM_ATTR PWM_control_Timer_callback(void)
#endif
#ifdef __AVR__
void PWM_control_Timer_callback(void)
#endif
{
  //g_revolutions_measured++;   // DEBUG
portENTER_CRITICAL_ISR(&dutycycle_desired_mux);  
unsigned long dutycycle_desired_copy=dutycycle_desired ;
portEXIT_CRITICAL_ISR(&dutycycle_desired_mux);
static bool HasRun=false;    // used to print time it took to actual do some work.

if(g_debug > 0 ) {
  if(HasRun==false) {
    HasRun=true;
    // Serial printing in interrupt routine should NOT be done ... (it crashes when writing though an interrupted Serial.print....)
    //Serial.print("PWM_control_Timer_callback activated first time after : ");
    //Serial.print(millis());
    //Serial.println(" ms");
  }
}
#ifdef SHOW_LED_IN_PWM_control_Timer_callback
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  digitalWrite(led_PIN, ledState);    // toggle internal led just for visual control, the PWM control is executed (but it might be too fast to see blinking)
#endif

  // Soft Update PWM dutycycle and LED indicator only when required.
  if (__dutycycle_actual != dutycycle_desired_copy) {

    if ( abs((signed)__dutycycle_actual - (signed)dutycycle_desired_copy) < g_pwm_soft_step ) {
      // (signed) is no typo, abs (unsigned, unsigned) returns unpredicatble results...) see http://forum.arduino.cc/index.php?topic=96185.0
      // softstep to target at once due to too small step.
      __dutycycle_actual = dutycycle_desired_copy;
    } else {
      if ( __dutycycle_actual < dutycycle_desired_copy ) {
        // to low, need go higher(faster) in steps
        __dutycycle_actual += g_pwm_soft_step;
      } else {
        // to high, need to go lower (slower) in steps
        __dutycycle_actual -= g_pwm_soft_step;
      }
    }
#ifdef __AVR__
    Timer1.setPwmDuty(PWM_PIN_OUT, __dutycycle_actual);
#endif
#ifdef ESP32
    //ledc_timer_config_t ledc_timer=
    //ledc_channel_t ledc_channel=
    //int hpoint = ledc_get_hpoint(LEDC_HIGH_SPEED_MODE,(ledc_channel_t)g_ledChannel);
    // just read HPOINT, what is it exactly?
    // 
    //Serial.print("HPOINT : ");
    //Serial.println(hpoint);

 #ifndef LEDCWRITE_SUSPECTEDBUG_WORKAROUND
    ledcWrite(g_ledChannel, __dutycycle_actual);    // ledcWrite seems not thread safe, and crashes quite frequently
 #else
    int hpoint = ledc_get_hpoint(LEDC_HIGH_SPEED_MODE,(ledc_channel_t)g_ledChannel);
    //ledc_mode_t ledc_mode=LEDC_HIGH_SPEED_MODE;
    //                       MODE                 Channel                   Dutycycle         hpoint
    ledc_set_duty_and_update(LEDC_HIGH_SPEED_MODE,(ledc_channel_t)g_ledChannel,__dutycycle_actual, hpoint);
 #endif
#endif
  } // (dutycycyle_actual != dutycycle_desired_copy)

}

// END PWM related definitions







void setup(void)
{
#ifdef __AVR__
#define S_SPEED 9600
#endif
#ifdef ESP32
#define S_SPEED 115200
#endif

  Serial.begin(S_SPEED);

  // Don't wait for console when in production....
  if (g_debug > 0) delay(2000); // get serial port negotiation finished

  Serial.print(F("Digital PWM Temperature controler for car radiator\n Version: ")); //Print a message
  Serial.println(C_PROGVERSION);
  Serial.print(" Main source file: ");
  Serial.println(__FILE__);
  Serial.print(" Compile date & time: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  
  CheckHardware();    // Display Chip revision and temperature 


  Serial.println("(GP)IO Port assignments: ");
  Serial.print(" LED_BUILTIN PIN: ");
  Serial.println(LED_BUILTIN);

  Serial.print(" Tacho_PIN: ");
  Serial.println(Tacho_PIN);

  Serial.print(" PWM_PIN_OUT: ");
  Serial.println(PWM_PIN_OUT);

  Serial.print(" ONE_WIRE_BUS_PIN: ");
  Serial.println(ONE_WIRE_BUS_PIN);

  Serial.print(" analogInput_PIN: ");
  Serial.println(analogInput_PIN);

  Serial.println("I2C pins and addresses: ");

  Serial.print( " I2C_ADDRES: 0x");
  Serial.println(I2C_ADDRESS,HEX);
#ifdef ESP32
  Serial.print(" I2C SDA1_PIN: ");
  Serial.println(SDA1_PIN);
  Serial.print(" I2C SCL1_PIN: ");
  Serial.println(SCL1_PIN);

  // Timers used
  Serial.println("Timers used: ");
  
  Serial.print(" Watchdog timer used C_WDT_HW_TIMER_NR: ");
  Serial.println(C_WDT_HW_TIMER_NR);

  Serial.print(" PWM Control timer used C_PWM_CONTROL_HW_TIMER_NR: ");
  Serial.println(C_PWM_CONTROL_HW_TIMER_NR);

  Serial.print(" LEDC channel/timer g_ledChannel ");
  Serial.println(g_ledChannel);


  Serial.print("webapp SSE update thottle (ms) : " );
  //Serial.print(C_SSE_Update_throttle_time_ms);
  Serial.print("oled screen update thottle (ms) : " );

  Serial.print("ESP.getFreeHeap: ");
  Serial.println(ESP.getFreeHeap());


//  ADC_MODE(ADC_VCC);  //Vcc read mode
//  ESP.getVcc();   // measure VCC
#endif

#ifdef __AVR__
  wdt_disable();    // prevent settings previous programm disturb this setup.
  //WDT_Disable(WDT);
#endif

#ifdef __AVR__
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();
  oled.setScroll(true);
#endif

#ifdef ESP32
  display.init();
  display.drawString(0, 0, "ESP32 oled init");
  display.display();
  //delay(5000);
#endif


  //Serial.println(F("DEBUG_001")); //Print a message


  // Wait only on serial, when debug is on compile time.
  if (g_debug) {
#ifdef __AVR__
    oled.print(F("1306OLED\nScreen initialized\nDEBUG: Waiting for serial console"));
#endif
#ifdef ESP32
    display.clear();
    display.drawString(0, 0, "1306OLED\nScreen initialized\nDEBUG: Waiting for serial console");
    display.display();
#endif
    while (!Serial);             // Leonardo: wait for serial monitor, only use for realtime debugging, not for standalone!
  } else {
#ifdef __AVR__
    oled.print(F("1306OLED\nScreen initialized"));
#endif
#ifdef ESP32
    //int counter = 1;

    //int progress = (counter / 5) % 100;
    // draw the progress bar
    //display.drawProgressBar(0, 32, 120, 10, progress);

    // draw the percentage as String
    //display.setTextAlignment(TEXT_ALIGN_CENTER);
    //display.drawString(64, 15, String(progress) + "%");

    //display.drawString(0, 0, "1306OLED\nScreen initialized");
    //display.display();
#endif
  }

  persistent_variabeles_setup();

  // https://folk.uio.no/jeanra/Microelectronics/ArduinoWatchdog.html
  // enable the watchdog
  //wdt_enable(WDTO_4S);

  //
  // analog Volt meter setup
  pinMode(analogInput_PIN, INPUT);


  // PWM initialization based on Timer1 and Timer3 libaries
  pinMode(led_PIN, OUTPUT); // visual inspection of actual state slow = low duty cycle, hig is high duty cycle

  //Serial.println(F("DEBUG_0088")); //Print a message
  //delay(5000);
  //Serial.println(g_PWM_Period);

  //Serial.println(F("DEBUG_009")); //Print a message
  //delay(5000);

#ifdef ESP32
  // PWM/LED (FAN motor ) setup
  ledcSetup(g_ledChannel, 15000 , PWM_resolution);    // fixed type 150.000 ! (is overwritten anyway)
  ledcAttachPin(PWM_PIN_OUT, g_ledChannel);
  ledcWrite(g_ledChannel, 0);

  //Serial.println(F("DEBUG_0099")); //Print a message
  //delay(5000);


  // PWM soft update timer
  pwm_control_timer = timerBegin(C_PWM_CONTROL_HW_TIMER_NR, 80, true);
  timerAttachInterrupt(pwm_control_timer, &PWM_control_Timer_callback, true);  //attach callback
  timerAlarmWrite(pwm_control_timer, 1000, true); //set time in us, 3th paramter true=repeat!!
  timerAlarmEnable(pwm_control_timer);
  Serial.println("Startend pwm_control_timer");
#endif

  //Serial.println(F("DEBUG_00999")); //Print a message
  //delay(5000);

  set_PWM_Period_us(g_PWM_Period);     // Set Period at startup, might change later.   // TODO needs fix
  //Timer1.initialize(g_PWM_Period);  // Timer1 for PWM @15KHz
  //Timer1.pwm( PWM_PIN_OUT, PWM_MIN_DUTYCYCLE);  // initialize on pin 9 ,Dutcycle.  remark: 2 a dutycycle is minimum PWM at 15KHz/66, 1023 is maximum PWM (0,1 = 0 1024 = 1)

  //Timer3.initialize(g_pwm_control_timer_period_us);     // depends on motor start time (ex 10seconds)

  //Serial.println(F("DEBUG_010")); //Print a message
  //delay(5000);


  set_pwm_control_timer(g_FAN_SLOWSTART_TIME);
#ifdef __AVR__
  Timer3.attachInterrupt(PWM_control_Timer_callback); // soft update desired PWM state every ??? seconds
#endif

  //Serial.println(F("DEBUG_01010")); //Print a message
  //delay(5000);

  //oled.begin(&Adafruit128x64, I2C_ADDRESS);
  //oled.setFont(System5x7);
  //oled.clear();
  //oled.setScroll(true);
  //oled.setContrast(0);    // allmost unnotisable, but might make the difference at night
  //oled.ssd1306WriteCmd(SSD1306_SETCONTRAST);
  //oled.ssd1306WriteCmd(255);

  //oled.setTextColor(WHITE);


  //g_revolutions_measured=0;
  //g_calculated_rpm=0;
  //g_rpm_measurement_time_previous=millis();
  //https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/



  rpm_measurement_setup();

  setup_dallas();

//WiFiManager wifiManager;


#ifdef ESP32
  // We start by connecting to a WiFi network
  bool WiFiClient = false;

  if (WiFiClient) {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println();
    Serial.print("Setting up WiFi Server ");
    Serial.println(WIFI_Server_SSID);

    {
      // untested: to prevent providing a default route (and effectivily disable/overrule other 3,4G routing options)
    uint8_t val=0;
    tcpip_adapter_dhcps_option(TCPIP_ADAPTER_OP_SET, TCPIP_ADAPTER_ROUTER_SOLICITATION_ADDRESS, &val, sizeof(dhcps_offer_t));    // disable advertizement of default route
    }
    //wifi_softap_set_dhcps_offer_option(OFFER_ROUTER,0);
    WiFi.softAP(WIFI_Server_SSID, WIFI_PASS);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp8266.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    if (!MDNS.begin("esp32")) {
        Serial.println("Error setting up MDNS responder!");
        /*
         * refuse to halt , more importand things to do like cooling engine
        while(1) {
            delay(1000);
        }
        */
    }
    // Set a friendly name, (BonjourSearch)
    MDNS.setInstanceName("My Radator FAN control Service interface");
    //MDNS.enableArduino();

    // ESP32 logging macro's:
    // http://esp-idf.readthedocs.io/en/latest/api-reference/system/log.html
    ESP_LOGE("TAG","ESP32 log macro test");
    
#ifdef USE_HTTP_SERVER
  async_http_server_setup();
  //AsyncWiFiManager wifiManager(&server);

    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  
#endif

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();


#ifdef USE_BLE_SERVER
  Serial.println("Starting BLE work!");

  Serial.print("ESP.getFreeHeap (before BLE): ");
  Serial.println(ESP.getFreeHeap());

  InitBLE();
  //bpm = 1;

#endif

#endif

#ifdef ESP32
  Serial.print("ESP.getFreeHeap (end setup): ");
  Serial.println(ESP.getFreeHeap());
#endif



  // setup WD as last, connecting to WiFi network may take some serious time for example.


#ifdef __AVR__
  wdt_enable(WDTO_4S);
#endif
#ifdef ESP32
  esp32_wdt_setup(8000);    // 4 seconds
  Serial.println("Watchdog started with timeout of 8000 ms");
#endif

}


//-------------------------------------------------------------------------------


void loop(void)
{
  uint32_t  loop_begin_time, loop_end_time;
  loop_begin_time=millis();
  
  //unsigned long blinkCopy;  // holds a copy of the blinkCount
  //static unsigned long dutycycle_desired , __dutycycle_actual=0;
  // Send the command to get temperatures
  // call sensors.requestTemperatures() to issue a global temperature

  if(g_debug>0) Serial.println("DEBUG: Start new loop");
  if(shouldReboot){
    Serial.println("Rebooting... as requested by the software...");
    delay(100);
    ESP.restart();
  }

  if(g_debug>0) { 
    Serial.print("DEBUG: before updating SSE values (ms): ");
    Serial.println(millis()-loop_begin_time);
  }
  update_html5_sse_values_throttled(false);    // smart (enforce=false) update values in Cockpitview
  //delay(700);
  if(g_debug>0) Serial.println("DEBUG: after updating SSE values");

  /*
     Nice promissing , but blocks Timer1/PWM!!!!
    FreqCount.begin(1000);
    while (!FreqCount.available());
    Serial.print("FreqCount: ");
    Serial.println(FreqCount.read());
    FreqCount.end();
  */

  /*Serial.print("Tacho_PIN Value: ");
    Serial.println(digitalRead(Tacho_PIN));
  */
  //Serial.print("ESP.getFreeHeap: ");
  //Serial.println(ESP.getFreeHeap());

#ifdef USE_BLE_SERVER
  update_BLE_values();
#endif

#ifdef ESP32
  //checkWifiClient();      // handle optional client command through WiFI
  if(g_debug>0) Serial.println("DEBUG: before OTA.handle()");
  ArduinoOTA.handle();    // handle OTA initiations
  if(g_debug>0) Serial.println("DEBUG: after OTA.handle()");
#endif

  //rpm_fun();
  //Serial.println(F("DEBUG_00010")); //Print a message
  //delay(5000);


  // update the mtoror temperature automatic to stress test the PWM update
  if (g_operating_mode == C_OPERATING_MODE_SIMULATED) { 
    // Dynamic adjust temperature, if set to manual
    static bool high=false;
    static uint32_t start=millis();
    uint32_t current=millis();

    if(current - start > 10000) {
      if(high==false) {
        Serial.println("Setting Motor temp to 100");
        high=true;
        g_simulate_values[MotorTemp_IDX]=100;
        g_CoolingWaterTemp_Motor_OUT_InC=g_simulate_values[MotorTemp_IDX];
      }
      else {
        Serial.println("Setting Motor temp to 80");
        high=false;
        g_simulate_values[MotorTemp_IDX]=80;
      }

      start=current;
    }
    // in case no sensors are attached..:
    g_CoolingWaterTemp_Motor_OUT_InC=g_simulate_values[MotorTemp_IDX];

  }

  if (g_Dallas_Synchronous) {
    get_Dallas_Temperatures_sync();   // old, blocking and slow
  } else {
    get_Dallas_Temperatures_async();  // new, nonblocking, and smarter => fast
  }
  //checkAlarm(MotorOutThermometer);  // TODO
  //checkAlarm(RadiatorOutThermometer);

  if(g_debug>0) Serial.println("DEBUG: before update PWM dutycycle");

  update_PWM_dutycycle_desired();
  //dutycycle_desired=512;          // overrule dutycycle hardcoded for debug

  if(g_debug>0) Serial.println("DEBUG: before update oled");

  update_oled_display();

  if(g_debug>0) Serial.println("DEBUG: before getDataFromPC");

  // read cmd's and setting updates
  getDataFromPC();

  //processDataFromPC();

  // Just wait, until time passes, or serial data is available
  // Still the maximum response time is the loop execution (mainly dallas sensor conversion time)

  //esp_deep_sleep_enable_timer_wakeup(100);
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //sleep_enable();
  // https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/DeepSleep/TimerWakeUp/TimerWakeUp.ino
  //esp_sleep_enable_timer_wakeup(5 * 1000000);
  //esp_deep_sleep_start();

  if (g_Dallas_Synchronous) {
    delay_until_Serial_available(500);
  } else {
    //delay(500);
    delay_until_Serial_available(1);
  }

  if(g_debug>0) Serial.println("DEBUG: before RPM events handle");

  //handle_rpm_events_count();
  handle_rpm_events_time();

  // Trigger keepalive and reenable watchdog if it was disable (for example due to USB state change)
  WatchdogWatch();

  if(g_debug>0) {
    Serial.print("Server Sent event clients registered for /events/ :");
    Serial.println(events.count());
  }
  if(g_debug>0) Serial.println("DEBUG: Loop END");
  loop_end_time=millis();

  if(g_debug>0) {
  Serial.print("Loop took : ");
  Serial.print(loop_end_time - loop_begin_time);
  Serial.println("ms");
  } 
}

//--------------------------------------------------------------------------



void blinkLED(void)
{
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
  } else {
    ledState = LOW;
  }
  digitalWrite(led_PIN, ledState);
}




#ifdef ESP32
void testSpi()
{
  usleep(10);
  // SPI WRITE
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  //byte stuff = 0b11001100;
  //hspi->transfer(stuff);
  digitalWrite(33, LOW);
  toTransfer = 0b11001100 << 8;
  toTransfer |= 0b11001100 ;
  vspi->transfer16(toTransfer);
  digitalWrite(33, HIGH);
  vspi->endTransaction();
  usleep(10);
}

#endif

