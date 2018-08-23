# CarElectricalRadatorFanController

PWM car radiator fan control, as addon or repacement for thermostat swich with WiFi support

## Introduction

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
 *   After some several firmware updates, I enabled firmware updates over wifi, as the USB port is hidden/water proofed inside the motor room, and my laptop contains policies which make native OTA (read incoming connections) imposible.
 *  

