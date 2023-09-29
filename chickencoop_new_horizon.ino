// if permission denied for "ttyUSB0" try: sudo chmod a+rw /dev/ttyUSB0 

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

//for bottons
#define BOTTON_green 8
#define BOTTON_red 9

//for motor
#define motor_a 10
#define motor_b 11

//endstops door
#define endstop_not_open 6
#define endstop_not_close 7


// SUNRISE AND SUNSET OVER THE YEAR
// JAN 8:17 -> 7:49    minus 28 min
// FEB 7:48 -> 6:55    minus 53 min     
// MAR 6:53 -> 5:55 (Zeitumstellung 26.03) 6:52 -> 6:43  
// APR 6:41 -> 5:37    minus 64 min
// MAI 5:35 -> 4:50    minus 45 min
// JUN 4:49 -> 4:47    minus  2 min
// JUL 4:47 -> 5:24    plus  47 min
// AUG 5:26 -> 6:15    plus  49 min
// SEP 6:17 -> 7:05    plus  48 min
// OCT 7:09 -> 7:57 (Zeitumstellung 29.10) 6:58 -> 7:00
// NOV 7:02 -> 7:52    plus 50 min
// DEC 7:54 -> 8:17    plus 23 min

uint8_t dailySunrises[365];


void setup() {
  for(uint8_t i = 0; i < sizeof(dailySunrises); i++){
    dailySunrises[i] = i;
  }
}


void loop() {
}
