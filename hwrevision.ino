/*
 * Based on:
 *   2017-07-17 Andreas Spiess

  + Fix from issues on github
  https://github.com/SensorsIot/ESP32-Revision-level/pull/1/commits/b18eb92053dc31b71773e3977d82ebe6e6b3c443

*/


#ifdef ESP32

#include "soc/efuse_reg.h"

#endif



// Check Chip revisions etc
void CheckHardware(void){
#ifdef __AVR__
 // anything to check at startup?
#endif

#ifdef ESP32
  Serial.print("ESP32 Chip Revision (official version): ");
  Serial.println(getChipRevision());

  Serial.print("ESP32 Chip Revision from shift Opration ");
  Serial.println(REG_READ(EFUSE_BLK0_RDATA3_REG) >> 15, BIN);

  uint64_t chipid; 
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

  Serial.print("Internal ESP32 Temperature in Celsius: ");
  Serial.println(getChipTemperatureInC());

#endif

  

  return;
}

#ifdef ESP32


int getChipRevision()
{
  return (REG_READ(EFUSE_BLK0_RDATA3_REG) >> EFUSE_RD_CHIP_VER_REV1_S & EFUSE_RD_CHIP_VER_REV1_V) ;
}



#endif

