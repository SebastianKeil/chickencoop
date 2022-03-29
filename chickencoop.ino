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
#define motor_a 10
#define motor_b 11
int rotary_state_clk;
int rotary_state_dt;


//for LCD 
#define lcd_address 0x27
LiquidCrystal_I2C lcd(lcd_address, 20, 4);

//for RTC (real time clock)
tmElements_t tm;
const char *monthName[12] = {
  "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/  SOFTWARE  _/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define MENU_ITEM_COUNT 5

//menu timer
bool backlight_on = false;
bool clear_lcd_flag = false;
bool backlight_on_flag = false;
bool dashboard_active = true;
unsigned long last_time_input;
unsigned long backlight_time;

bool input_left = false;
bool input_right = false;
bool input_switch = false;

//menu
int menu_item = 0;
int menu_level = 0;
int menu_select = 0;

//hardware data
int door_time_open_hours = 7;
int door_time_open_minutes = 30;
int door_time_close_hours = 20;
int door_time_close_minutes = 30;

//status
bool door_closed = true;

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/ SETUP  _/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void setup() {
  //turn on connection to serial and lcd
  //Serial.begin(9600);
  lcd.begin(20, 4);

  //set pinMode for IO-pins
  pinMode(rotary_clk, INPUT_PULLUP);
  pinMode(rotary_dt, INPUT);
  pinMode(rotary_sw, INPUT);

  //setup for rotary encoder 
  attachInterrupt(digitalPinToInterrupt(rotary_clk), input_handler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rotary_sw), switch_handler, RISING);

  backlight_on_flag = true;
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/  LOOP  _/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void loop() {
  update_time();

  if(input_left){
    switch(menu_level){
      case 0:
        if(menu_item >= 1) menu_item --;
        break;
      case 1:
        if(menu_select >= 1) menu_select --;
        break;
    }
    input_left = false;
  }
  if(input_right){
    switch(menu_level){
      case 0:
        if(menu_item < MENU_ITEM_COUNT - 1) menu_item ++;
        break;
      case 1:
        if(menu_select < 1) menu_select ++;
        break;
    }
    
    input_right = false;
  }
  
  if(input_switch){
    if(menu_level < 2) menu_level ++;
    input_switch = false;
  }

  if(clear_lcd_flag){
    lcd.clear();
    clear_lcd_flag = false;
  }

  if(backlight_on_flag){
    lcd.backlight();
    backlight_on = true;
    backlight_on_flag = false;
  }

  if(backlight_on && backlight_time + 20000 < millis()){
    clear_lcd_flag = true;
    lcd.noBacklight();
    backlight_on = false;
  }

  if(last_time_input + 5000 < millis()){ 
    go_to_dashboard();
  }

  draw_screen();
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/ FUNCTIONS  _/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//INTERRUPTS _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void switch_handler(){
  if(digitalRead(rotary_sw)){
    input_switch = true;
  }
}

void input_handler(){
  delay(50);
  last_time_input = millis();
  backlight_time = last_time_input;
  clear_lcd_flag = true;

  if(backlight_on){
    if(dashboard_active){
      dashboard_active = false;
    }else{
      check_input();
    }
  }else{ 
    backlight_on_flag = true;
  }
}

void check_input(){
  if(digitalRead(rotary_dt) && digitalRead(rotary_clk)){
    input_left = true;  
  }
  if(!digitalRead(rotary_dt) && digitalRead(rotary_clk)){
    input_right = true;  
  }
}

//LCD _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void draw_screen(){
  //lcd.clear();
  if(dashboard_active){
    draw_dashboard();
    delay(500);
  } else{
    draw_menu();
  }
  //delay(500);

}

void draw_dashboard(){
  lcd_print_time();
}

void draw_menu(){
  switch(menu_item){
    case 0:
      draw_menu_0();
      break;
    case 1:
      draw_menu_1();
      break;
    case 2:
      draw_menu_2();
      break;
    case 3:
      draw_menu_3();
      break;
    case 4:
      draw_menu_4();
      break;
  }
}

void draw_menu_0(){
  lcd.setCursor(0,0);
  lcd.print("DOOR TIME");

  switch(menu_level){
    case 0:
      lcd.setCursor(0,2);
      lcd.print(" OPEN  AT: ");
      lcd.print(print_two_digits(door_time_open_hours));
      lcd.print(":");
      lcd.print(print_two_digits(door_time_open_minutes));

      lcd.setCursor(0,3);
      lcd.print(" CLOSE AT: ");
      lcd.print(print_two_digits(door_time_close_hours));
      lcd.print(":");
      lcd.print(print_two_digits(door_time_close_minutes));
      break;

    case 1:
      lcd.setCursor(0,2);
      if(menu_select == 0){ lcd.print(">");}else{lcd.print(" ");}
      lcd.print("OPEN  AT: ");
      lcd.print(print_two_digits(door_time_open_hours));
      lcd.print(":");
      lcd.print(print_two_digits(door_time_open_minutes));

      lcd.setCursor(0,3);
      if(menu_select == 1){ lcd.print(">");}else{lcd.print(" ");}
      lcd.print("CLOSE AT: ");
      lcd.print(print_two_digits(door_time_close_hours));
      lcd.print(":");
      lcd.print(print_two_digits(door_time_close_minutes));
      break;

    case 2:
      switch(menu_select){
        case 0:
          set_door_open_time();                                      
          break;
        case 1:
          set_door_close_time();
          break;
      }
      break;
  }
 
}

void draw_menu_1(){
  lcd.setCursor(0,0);
  lcd.print("SYSTEM TIME");
  switch(menu_level){
    case 0:
      lcd.setCursor(0,2);
      lcd.print(" ");
      lcd.print(print_two_digits(tm.Hour));
      lcd.print(":");
      lcd.print(print_two_digits(tm.Minute));
      lcd.setCursor(0,3);
      lcd.print(" ");
      lcd.print(print_two_digits(tm.Day));
      lcd.print("/");
      lcd.print(monthName[tm.Month - 1]);
      break;
    case 1:
      lcd.setCursor(0,2);
      if(menu_select == 0){lcd.print(">");}else lcd.print(" ");
      lcd.print(print_two_digits(tm.Hour));
      lcd.print(":");
      lcd.print(print_two_digits(tm.Minute));
      lcd.setCursor(0,3);
      if(menu_select == 1){lcd.print(">");}else lcd.print(" ");
      lcd.print(print_two_digits(tm.Day));
      lcd.print("/");
      lcd.print(monthName[tm.Month - 1]);
      break;
    case 2:
      switch(menu_select){
          case 0:
            set_system_time();                                      
            break;
          case 1:
            set_system_date();
            break;
        }
      break;
  }
}

void draw_menu_2(){
  lcd.setCursor(0,0);
  lcd.print("VENTILATION");
  switch(menu_level){
    case 0:
      break;
    case 1:
      lcd.setCursor(0,2);
      if(menu_select == 0){lcd.print(">");}else lcd.print(" ");
      lcd.setCursor(0,3);
      if(menu_select == 1){lcd.print(">");}else lcd.print(" ");
      break;
    case 2:
      switch(menu_select){
          case 0:
            //set_ventilation_temp();                                      
            break;
          case 1:
            //set_ventilation_time();
            break;
        }
      break;
  }
}

void draw_menu_3(){
  lcd.setCursor(0,0);
  lcd.print("DEFENSE SYSTEM");
  switch(menu_level){
    case 0:
      lcd.setCursor(0,2);
      lcd.print(" ACTIVATE");
      lcd.setCursor(0,3);
      lcd.print(" DEACTIVATE");
      break;
    case 1:
      lcd.setCursor(0,2);
      if(menu_select == 0){lcd.print(">");}else lcd.print(" ");
      lcd.print("ACTIVATE");
      lcd.setCursor(0,3);
      if(menu_select == 1){lcd.print(">");}else lcd.print(" ");
      lcd.print("DEACTIVATE");
      break;
    case 2:
      switch(menu_select){
        case 0:
          //set_defense_activate();                                      
          break;
        case 1:
          //set_defense_deactivate();
          break;
      }
      go_to_dashboard();
      break;
  }
}

void draw_menu_4(){
  lcd.setCursor(0,0);
  lcd.print("DATA CONNECTION");
  switch(menu_level){
    case 0:
      lcd.setCursor(0,2);
      lcd.print(" INTERVAL");
      lcd.setCursor(0,3);
      lcd.print(" START");
      break;
    case 1:
      lcd.setCursor(0,2);
      if(menu_select == 0){lcd.print(">");}else lcd.print(" ");
      lcd.print("INTERVAL");
      lcd.setCursor(0,3);
      if(menu_select == 1){lcd.print(">");}else lcd.print(" ");
      lcd.print("START");
      break;
    case 2:
      switch(menu_select){
        case 0:
          //set_defense_activate();                                      
          break;
        case 1:
          //set_defense_deactivate();
          break;
      }
      go_to_dashboard();
      break;
  }
}

void set_system_time(){
  lcd.clear();
  while(!input_switch){
    if(input_left){
      if(tm.Minute >= 1){
        tm.Minute --;
      }else{ 
        tm.Minute = 59;
        if(tm.Hour >= 1){
          tm.Hour --;
        }else tm.Hour = 23;
      }
      input_left = false;
    }
    
    if(input_right){
      if(tm.Minute <= 58){
        tm.Minute ++;
      }else{ 
        tm.Minute = 0;
        if(tm.Hour <= 22){
          tm.Hour ++;
        }else tm.Hour = 0;
      }
      input_right = false;
    }
    lcd.setCursor(0, 2);
    lcd.print(print_two_digits(tm.Hour));
    lcd.print(":");
    lcd.print(print_two_digits(tm.Minute));
  }
  RTC.write(tm);
  go_to_dashboard();
}

void set_system_date(){
  lcd.clear();
  while(!input_switch){
    if(input_left){
      if(tm.Day >= 1){
        tm.Day --;
      }else{
        tm.Day = 31;
        if(tm.Month >= 1){
          tm.Month --;
        }else tm.Month = 11;
      }
      input_left = false;
    }
    
    if(input_right){
      if(tm.Day <= 30){
        tm.Day ++;
      }else{
        tm.Day = 1;
        if(tm.Month <= 10){
          tm.Month ++;
        }else tm.Month = 0;
      }
      input_right = false;
    }
    lcd.setCursor(0, 2);
    lcd.print(print_two_digits(tm.Day));
    lcd.print("/");
    lcd.print(monthName[tm.Month - 1]);
  }
  RTC.write(tm);
  go_to_dashboard();
}

void set_door_open_time(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SET DOOR OPEN TIME");
  
  while(!input_switch){
    if(input_left){
      if(door_time_open_minutes >= 10){
        door_time_open_minutes -= 10;
      }else{ 
        door_time_open_minutes = 50;
        if(door_time_open_hours >= 1){
          door_time_open_hours --;
        }else door_time_open_hours = 23;
      }
      input_left = false;
    }
    
    if(input_right){
      if(door_time_open_minutes <= 40){
        door_time_open_minutes += 10;
      }else{ 
        door_time_open_minutes = 0;
        if(door_time_open_hours <= 22){
          door_time_open_hours ++;
        }else door_time_open_hours = 0;
      }
      input_right = false;
    }
    lcd.setCursor(0, 2);
    lcd.print(print_two_digits(door_time_open_hours));
    lcd.print(":");
    lcd.print(print_two_digits(door_time_open_minutes));
  }
  go_to_dashboard();
}

void set_door_close_time(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SET DOOR CLOSE TIME");
  
  while(!input_switch){
    if(input_left){
      if(door_time_close_minutes >= 10){
        door_time_close_minutes -= 10;
      }else{ 
        door_time_close_minutes = 50;
        if(door_time_close_hours >= 1){
          door_time_close_hours --;
        }else door_time_close_hours = 23;
      }
      input_left = false;
    }
    
    if(input_right){
      if(door_time_close_minutes <= 40){
        door_time_close_minutes += 10;
      }else{ 
        door_time_close_minutes = 0;
        if(door_time_close_hours <= 22){
          door_time_close_hours ++;
        }else door_time_close_hours = 0;
      }
      input_right = false;
    }
    lcd.setCursor(0, 2);
    lcd.print(print_two_digits(door_time_close_hours));
    lcd.print(":");
    lcd.print(print_two_digits(door_time_close_minutes));
  }
  go_to_dashboard();   
}

void go_to_dashboard(){
  clear_lcd_flag = true;
  dashboard_active = true;
  menu_level = 0;
  menu_select = 0;
}

//TIME _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void update_time(){
  RTC.read(tm);
}

void lcd_print_time(){
  lcd.setCursor(0,0);
  lcd.print(print_two_digits(tm.Hour));
  lcd.print(":");
  lcd.print(print_two_digits(tm.Minute));
  lcd.print(" UHR");
  lcd.setCursor(12, 0);
  lcd.print(print_two_digits(tm.Day));
  lcd.print("/");
  lcd.print(monthName[tm.Month - 1]);
}

int print_two_digits(int times){
  if(times < 10){
    lcd.print("0");
  }
  return times;
}
