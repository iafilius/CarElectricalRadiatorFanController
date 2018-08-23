





//=========================
void update_oled_display(void) {
  static uint32_t previous_display_update_time = millis();
  uint32_t current_time = millis();

//return;   // mask stack mesh-up crash 2018/07/31

  //char TimeString[16];

  //g_display_update_time=500;    // override display update refresh rate

  if ( ( current_time - previous_display_update_time ) >= g_display_update_time ) {
    /*    Serial.print("Update Screen");
        Serial.println(current_time);
        Serial.println(g_display_update_time);
    */
    previous_display_update_time = current_time;
#ifdef __AVR__
    char buffer[C_STR_BUFSIZE];

    //oled.clear();
    //oled.setContrast(255);  // doet niets
    oled.setCursor(0, 0); // cursort in start position
    //oled.set2X();
    oled.print(F("Motor OUT T[0]: "));
    oled.print(g_CoolingWaterTemp_Motor_OUT_InC);
    oled.clearToEOL(); oled.println();

    //oled.set1X();

    //setContrast(&oled,128);
    oled.print(F("Motor IN  T[1]: "));
    oled.print(g_CoolingWaterTemp_Motor_IN_InC);
    oled.clearToEOL(); oled.println();

    oled.print(F("TimeTempSensors: "));  oled.print(t_dallas_sensors); ; oled.clearToEOL(); oled.println();
    oled.print("Time:"); get_RunningTime(buffer); oled.print(buffer); oled.clearToEOL(); oled.println();
    oled.print(F("Volt/Lamda: "));  oled.print(get_Voltage_f()); oled.print("/"); oled.print(-1);  oled.clearToEOL(); oled.println();
    oled.print(F("PWM (D/A): ")); oled.print(dutycycle_desired); oled.print(F("/")); oled.print(__dutycycle_actual); oled.clearToEOL(); oled.println();
    //oled.print(F("Vacuum: "));  oled.clearToEOL(); oled.println();
    //oled.print(F("FuelPressure: "));  oled.clearToEOL();oled.println();
    oled.print(F("RPM: ")); oled.print(g_calculated_rpm); oled.clearToEOL(); oled.println();
    oled.print(F("REV: ")); oled.print(g_revolutions_measured_frozen); oled.clearToEOL();
#endif
#ifdef ESP32
    char buffer[C_STR_BUFSIZE];
    display.clear();
    display.setFont(ArialMT_Plain_10);        // this seems to be the smallest font ... duh....
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "MTemp O/I:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    snprintf(buffer, C_STR_BUFSIZE , "%3.2f / %3.2f", g_CoolingWaterTemp_Motor_OUT_InC, g_CoolingWaterTemp_Motor_IN_InC);
    display.drawString(128, 0, buffer);


    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 10, "TimeTempSensors:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 10, itoa(t_dallas_sensors, buffer, 10));

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 20, "Running:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    get_RunningTime(buffer,C_STR_BUFSIZE);
    display.drawString(128, 20, buffer);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 30, "Volt/Lamda:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    snprintf(buffer, C_STR_BUFSIZE, "%2.3f/%2.3f", get_Voltage_f(), -1.0);
    display.drawString(128, 30, buffer);


    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 40, "PWM (D/A):");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    snprintf(buffer, C_STR_BUFSIZE, "%4lu/%4lu", dutycycle_desired, __dutycycle_actual);
    display.drawString(128, 40, buffer);


    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 50, "RPM/REV:");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    snprintf(buffer, C_STR_BUFSIZE, "%4lu,%4lu/%4lu", g_calculated_rpm * 60, g_calculated_rpm, g_revolutions_measured_frozen);
    if(g_debug >= DEBUGLEVEL2) {
      // if not correct (this would have caused a stack smashing bug..., not it cuts silently with snprintf
      Serial.print("buffer containing RPM info: ");
      Serial.println(buffer);
      Serial.println(g_calculated_rpm*60);
      Serial.println(g_calculated_rpm);
      Serial.println(g_revolutions_measured_frozen);
    }
    display.drawString(128, 50, buffer);



    /*
          oled_printRunningTime(); oled.clearToEOL(); oled.println();
          oled.print(F("Volt/Lamda: "));  oled_printVoltage();  oled.clearToEOL(); oled.println();
          oled.print(F("PWM (D/A): ")); oled.print(dutycycle_desired); oled.print(F("/")); oled.print(__dutycycle_actual); oled.clearToEOL(); oled.println();
          //oled.print(F("Vacuum: "));  oled.clearToEOL(); oled.println();
          //oled.print(F("FuelPressure: "));  oled.clearToEOL();oled.println();
          oled.print(F("RPM: ")); oled.print(g_calculated_rpm); oled.clearToEOL();oled.println();
          oled.print(F("REV: ")); oled.print(g_revolutions_measured_frozen); oled.clearToEOL();
    */
    display.display();
#endif
  } else {
    // nothing to do , when not time to update screen.
  }


}





// Function to print execution time from start
/*
  void oled_printRunningTime()
  {
  char TimeString[16];
  uint32_t time;

    // print millisecond from start of time
  #ifdef __AVR__
    oled.print(F("Time: "));
  #endif
    time = millis();
    seconds = (time / 1000) % 60; // module minutes
    minutes = (time / 1000 / 60) % 60; // modulo hours
    hours =  (time / 1000 / 60) / 60; // rest
    //prints time since program started
    sprintf(TimeString, "%02d:%02d:%02d", hours, minutes, seconds);
  #ifdef __AVR__
    oled.print(TimeString);
  #endif
    //oled.clearToEOL();
  }

*/




// Function to print execution time from start into TimeString
void get_RunningTime(char * TimeString, size_t len)
{
  //  char TimeString[16];
  uint32_t time;

  // print millisecond from start of time
#ifdef __AVR__
  oled.print(F("Time: "));
#endif
  time = millis();
  seconds = (time / 1000) % 60; // module minutes
  minutes = (time / 1000 / 60) % 60; // modulo hours
  hours =  (time / 1000 / 60) / 60; // rest
  //prints time since program started
  snprintf(TimeString, len, "%02d:%02d:%02d", hours, minutes, seconds);
  //#ifdef __AVR__
  //    oled.print(TimeString);
  //#endif
  //    //oled.clearToEOL();
}



