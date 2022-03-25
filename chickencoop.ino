#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Adafruit_LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/  HARDWARE  _/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//rotary encoder
#define rotary_clk 2
#define rotary_dt 4
#define rotary_sw 3
int rotary_state_clk;
int rotary_state_dt;


//for LCD 
#define lcd_address 0x27
LiquidCrystal_I2C lcd(lcd_address, 20, 4);

//for RTC (real time clock)
tmElements_t tm;


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/  SOFTWARE  _/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

bool dashboard_active = false;
unsigned long last_time_input;
bool input_left = false;
bool input_right = false;
bool input_switch = false;


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/ SETUP  _/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void setup() {
  //turn on connection to serial and lcd
  Serial.begin(9600);
  lcd.begin(20, 4);

  //set pinMode for IO-pins
  pinMode(rotary_clk, INPUT_PULLUP);
  pinMode(rotary_dt, INPUT);
  pinMode(rotary_sw, INPUT);

  //setup for rotary encoder 
  attachInterrupt(digitalPinToInterrupt(rotary_clk), input_handler, RISING);
  attachInterrupt(digitalPinToInterrupt(rotary_sw), switch_handler, RISING);
  //rotary_state_clk = digitalRead(rotary_clk);
  //rotary_state_dt = digitalRead(rotary_dt);

  lcd.backlight();
  lcd.setCursor(6, 1);
  lcd.print("SETUP DONE");
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/  LOOP  _/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void loop() {
  //update_time(tm);
  //draw_screen();

  //lcd.clear();

  /*
  rotary_state_clk = digitalRead(rotary_clk);
  rotary_state_dt = digitalRead(rotary_dt);
  Serial.print(rotary_state_clk);
  Serial.print(rotary_state_dt);
  Serial.print("\n");
  */

  if(input_left){
    Serial.println("left");
    input_left = false;
  }
  if(input_right){
    Serial.println("right");
    input_right = false;
  }
  if(input_switch){
    Serial.println("switch");
    input_switch = false;
  }

  //input_switch = false;
  //input_right = false;
  //input_left = false;
  delay(50);
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/ FUNCTIONS  _/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void switch_handler(){
  //cli();
  if(digitalRead(rotary_sw)){
    input_switch = true;
  }
  //sei();
}

void input_handler(){
  //cli(); //disable interrupts
  
  delay(10);
  //last_time_input = millis();
  if(dashboard_active){
    dashboard_active = false;
  }else{
    check_input();
  }
  
  //sei(); //enable interrupts
}

void check_input(){
  if(digitalRead(rotary_dt) && digitalRead(rotary_clk)){
    input_left = true;  
  }
  if(!digitalRead(rotary_dt) && digitalRead(rotary_clk)){
    input_right = true;  
  }
}

void update_time(tmElements_t tm){
  RTC.read(tm);
}

void draw_screen(){
  if(dashboard_active){
    draw_dashboard();
  } else{
    draw_menu();
  }

}

void draw_dashboard(){
  
}

void draw_menu(){

}

void lcd_print_time(){
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(tm.Hour);
  lcd.print(":");
  lcd.print(tm.Minute);
  lcd.print(":");
  lcd.print(tm.Second);
}
