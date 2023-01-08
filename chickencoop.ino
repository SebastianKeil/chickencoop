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

int incomingByte = 0; // for incoming serial data


// SUNRISE AND SUNSET OVER THE YEAR
// JAN 8:17 -> 7:49    minus 28 min
// FEB 7:48 -> 6:55    minus 53 min     
// MAR 6:53 -> 5:55    minus 58 min                     (Zeitumstellung 26.03) 6:52 -> 6:43  
// APR 6:41 -> 5:37    minus 64 min
// MAI 5:35 -> 4:50    minus 45 min
// JUN 4:49 -> 4:47    minus  2 min
// JUL 4:47 -> 5:24    plus  47 min
// AUG 5:26 -> 6:15    plus  49 min
// SEP 6:17 -> 7:05    plus  48 min
// OCT 7:09 -> 7:57    plus  48 min                     (Zeitumstellung 29.10) 6:58 -> 7:00
// NOV 7:02 -> 7:52    plus  50 min
// DEC 7:54 -> 8:17    plus  23 min

//monthlySunrise format: [start hour, start minute, weekly change]
int monthlySunrises[12][3] = {
  {8, 17, -7},
  {7, 48, -13},
  {6, 53, -14},
  {6, 41, -16},
  {5, 35, -11},
  {4, 49, 0},
  {4, 47, 12},
  {5, 26, 12},
  {6, 17, 12},
  {7, 9, 12},
  {7, 2, 12},
  {7, 54, 6}
};

int monthlySunsets[12][3] = {
  {16, 04, 12},
  {16, 53, 13},
  {17, 46, 14},
  {19, 41, 13},
  {20, 33, 12},
  {21, 21, 3},
  {21, 34, -9},
  {21, 1, -16},
  {19, 57, -18},
  {18, 46, -16},
  {16, 39, -11},
  {15, 58, 1}
};

//for RTC (real time clock)
tmElements_t tm;
int _hour;
int _min;
int _day;
int _month;
int _week;
int current_sunset_hour;
int current_sunset_min;
int current_sunrise_hour;
int current_sunrise_min;
int this_min;
int hour_bias;

//for Hardware Control
enum motion {UP, DOWN};
motion door_status;


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/  SETUP _/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void setup() {
  Serial.begin(9600);
  while (!Serial){};

  pinMode(motor_a, OUTPUT);
  pinMode(motor_b, OUTPUT);
  pinMode(endstop_not_open, INPUT_PULLUP);
  pinMode(endstop_not_close, INPUT_PULLUP);  

  //show_sunrises();
  //print_all_calculations();
  
  update_time();
  print_time();
  
  stop_door();
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/ FUNCTIONS  _/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//Hardware door control _/_/_/_/_/_/_/_/
void stop_door(){
  digitalWrite(motor_a, HIGH);
  digitalWrite(motor_b, HIGH);
}

void close_door(){
  Serial.println("CLOSING DOOR...");
  digitalWrite(motor_a, LOW);
  while(digitalRead(endstop_not_close)){
    delay(10);
  }
  digitalWrite(motor_a, HIGH);
  door_status = DOWN;
  Serial.println("DOOR IS CLOSED!");
}

void open_door(){
  Serial.println("OPENING DOOR...");
  digitalWrite(motor_b, LOW);
  while(digitalRead(endstop_not_open)){
    delay(10);
  }
  digitalWrite(motor_b, HIGH);
  door_status = UP;
  Serial.println("DOOR IS OPEN!");
}

//TIME _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
void update_time(){
  RTC.read(tm);
  _hour = tm.Hour;
  _min = tm.Minute;
  _day = tm.Day;
  _month = tm.Month;
}

void calculate_clock_bias(){
  hour_bias = 0;            
  if(this_min < 0){
    hour_bias = -1;
    this_min = this_min + 60;
  }else if(this_min > 59){
    hour_bias = 1;
    this_min = this_min - 60;
  }
}

void calculate_current_sunset(){
  _week = floor(_day / 7);
  this_min = (monthlySunsets[_month - 1][1]) + (_week * monthlySunsets[_month - 1][2]);
  calculate_clock_bias();
  current_sunset_hour = monthlySunsets[_month - 1][0] + hour_bias;
  current_sunset_min = this_min;
}

void calculate_current_sunrise(){
  _week = floor(_day / 7);
  this_min = (monthlySunrises[_month - 1][1]) + (_week * monthlySunrises[_month - 1][2]);
  calculate_clock_bias();
  current_sunrise_hour = monthlySunrises[_month - 1][0] + hour_bias;
  current_sunrise_min = this_min;
}

bool moment_of_sunset(){
  calculate_current_sunset();
  if(current_sunset_hour == _hour && current_sunset_min == _min){
    return true;
  }else return false;
}

bool moment_of_sunrise(){
  calculate_current_sunrise();
  if(current_sunrise_hour == _hour && current_sunrise_min == _min){
    return true;
  }else return false;
}

void check_door_action(){
  switch(door_status){
    case UP: 
      if(moment_of_sunset()){
        close_door();
      }
      break;
    case DOWN:
      if(moment_of_sunrise()){
        open_door();
      }
      break;
  }
}

void show_sunrises(){
  for(int i = 0; i < 12; i++){
    Serial.print("Month: ");
    Serial.println((i + 1));
    for(int j = 0; j < 4; j++){
      Serial.print(".....");
      Serial.print("Week ");
      Serial.println((j + 1));  

      Serial.print(".....");
      Serial.print(".....");
      
      int min = monthlySunrises[i][1] + j * monthlySunrises[i][2];
      int bias = 0;            
      if(min < 0){
        bias = -1;
        min = min + 60;
      }else if(min > 59){
        bias = 1;
        min = min - 60;
      }
      Serial.print(monthlySunrises[i][0] + bias);
      Serial.print(":");
      Serial.println(min);

    }
  }
}

void print_time(){
  Serial.print("tm.Hour: ");
  Serial.print(tm.Hour);
  Serial.print("tm.Minute: ");
  Serial.println(tm.Minute);
  Serial.print("tm.Day: ");
  Serial.println(tm.Day);
  Serial.print("tm.Month: ");
  Serial.println(tm.Month);
}

void print_sun_times(){
  calculate_current_sunrise();
  calculate_current_sunset();

  Serial.print("current sunrise: ");
  Serial.print(current_sunrise_hour);
  Serial.print(":");
  Serial.println(current_sunrise_min);

  Serial.print("current sunset: ");
  Serial.print(current_sunset_hour);
  Serial.print(":");
  Serial.println(current_sunset_min);
}

void print_all_calculations(){
  for(int i = 0; i < 12; i++){
    Serial.println("_/_/_/_/_/_/_/_/_/_/_/_/"); 
    Serial.print("Month: ");
    Serial.println(i + 1);
    for(int j = 0; j < 31; j = j + 7){
      _month = 1 + i;
      _day = j + 1;

      Serial.print(".....");
      Serial.print("Day: ");
      Serial.println(_day);
       
      print_sun_times();
      Serial.println("_/_/_/_/_/_/_/_/_/_/_/_/"); 
    }
  }
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/  LOOP  _/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void loop() {  
  stop_door();
  update_time();

  incomingByte = 0;  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
  }

  switch(incomingByte){
    case 0:
      break;      
    case -1:
      break;      
    case 49:
      close_door();
      break;
    case 50:
      open_door();
      break;  
  }
  
  //check_door_action();
  
}
