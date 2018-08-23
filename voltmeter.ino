// Public global paramters related to this module



// Voltmeter
// http://www.electroschematics.com/9351/arduino-digital-voltmeter/




// END Public global paramters related to this module



float get_Voltage_f()
{
  //char VoltString[16];
  // read the value at analog input
  value = analogRead(analogInput_PIN); // 217 without anything attached
  vout = (value * 5.0) / 1023.0; // see text (10 bits ) 0x3FF:  11 1111 1111
  vin = vout / (R2 / (R1 + R2));
  if (vin < 0.09) {
    vin = 0.0; //statement to quash undesired reading !
  }
  //sprintf(VoltString,"%2.3f", vin);// float formatting is not included in arduino
  // http://stackoverflow.com/questions/27651012/arduino-sprintf-float-not-formatting
  //dtostrf(vin, 4, 2, VoltString);

  //oled.print(value);
  //oled.print(vout);
  //oled.print(vin);
  //#ifdef __AVR__
  //  oled.print(VoltString);
  //#endif
  return vin;
}


