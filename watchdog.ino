// ESP32/Timer/Watchdogtimer


#ifdef ESP32
hw_timer_t * wdt_timer = NULL;
//#error
#endif


// loop, and do never return. watchdog should notice, and cause a reboot.
void testWatchDog(void) {
  while (1);
}




//=========================
void WatchdogWatch(void) {
  static uint32_t previous_time = millis();
  uint32_t current_time = millis();

  if ( (current_time - previous_time) > 250 ) {  // Update only this much, to prevent USB/serial IDE blocking

    //wdt_reset();  // needs testing before production, not every bootloader supports it (in clones)

    // used :
    // https://forum.arduino.cc/index.php?topic=295345.0
    // for solving the watchdog didn't work properly on leonardo

#ifdef __AVR_ATmega32U4__

    //Test if watchdog interrupt enabled
    //if (WDTCSR & (1<<WDIE))
    {
      //Prolong wtachdog timer
      //Serial.write("wdt_reset wdtr_reset wdt_reset\n");
      wdt_reset();

    }
    //No interrupt enabled - Test if watchdog reset enabled
    //else
    if (WDTCSR & (1 << WDE))
    {
      //Bootloader about to reset - do not prolong watchdog
    }
    //It has ben disabled - Enable and prolong
    else
    {
      //Watchdog disabled - Enable again
      Serial.print(F("wdt_enable wdtr_enable wdt_enable\n"));
      wdt_enable(WDTO_4S);
      //delay(2000);
    }
#endif
#ifdef ESP32
    timerWrite(wdt_timer, 0); //reset timer (feed watchdog)

#endif

  } else {
    if (g_debug > DEBUGLEVEL2 ) Serial.println(F("Watchdogwatch to soon to update"));
  }


}




//http://playground.arduino.cc/Code/EEPROMWriteAnything

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
#ifdef __AVR__
  asm volatile ("  jmp 0");
#endif
}



#ifdef ESP32
//const int wdtTimeout = 4000;  //time in ms to trigger the watchdog

void IRAM_ATTR resetModule() {
  ets_printf("reboot, not feeding watchdog...\n");
  esp_restart_noos();
}

void esp32_wdt_setup(int wdtTimeout) {
  wdt_timer = timerBegin(C_WDT_HW_TIMER_NR, 80, true);                  //timer 0, div 80 => 1Mhz
  timerAttachInterrupt(wdt_timer, &resetModule, true);  //attach callback
  timerAlarmWrite(wdt_timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(wdt_timer);                          //enable interrupt
}
#endif




