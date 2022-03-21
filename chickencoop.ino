#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Adafruit_LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>


//for rotary encoder
#define INPUT_rotary_clk 10
#define INPUT_rotary_dt 9
int current_rotary_state;
int previous_roatary_state;

//for move sensor
//#define INPUT_moveSens 7

//making a display object
LiquidCrystal_I2C lcd(0x27, 16, 2);

//making a time datatype
tmElements_t tm;

int global_counter;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(INPUT_rotary_clk, INPUT);
  pinMode(INPUT_rotary_dt, INPUT);
  //pinMode(INPUT_moveSens, INPUT);

  previouse_rotary_state = digitalRead(INPUT_rotary_clk);
}


void loop() {
  RTC.read(tm);
  lcd_print_time();
  global_counter_manage();
  attachInterrupt()
}

void global_counter_manage(){
    
  }


void lcd_print_time(){
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(tm.Hour);
  lcd.print(":");
  lcd.print(tm.Minute);
  lcd.print(":");
  lcd.print(tm.Second);
  lcd.setCursor(0,1);
  lcd.print(global_counter);
}
