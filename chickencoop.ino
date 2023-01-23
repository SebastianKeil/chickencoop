// if permission denied for "ttyUSB0" try: sudo chmod a+rw /dev/ttyUSB0 

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

//for bottons
#define BOTTON_green 8
#define BOTTON_red 9

//for motor
#define motor_a 10
#define motor_b 11

//endstops door
#define endstop_not_open 7
#define endstop_not_close 6

//for lamp with neopixel
#define neopixels 9
#define NUMPIXEL 18
Adafruit_NeoPixel pixels(NUMPIXEL, neopixels, NEO_GRB + NEO_KHZ800);

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
  //{11, 2, 0}, //->10:17
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
  //{9, 30, 0}, //->10:15
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

#define DOOR_CLOSING_OFFSET 45
#define DOOR_OPENING_OFFSET 45
#define LAMP_ON_OFFSET -45
#define LAMP_OFF_OFFSET 45
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
motion door_status = UP;

enum light {ON, OFF};
light lamp_status = OFF;

int r;
int g;
int b;


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/PRINTERS_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

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

void show_sunsets(){
  for(int i = 0; i < 12; i++){
    Serial.print("Month: ");
    Serial.println((i + 1));
    for(int j = 0; j < 4; j++){
      Serial.print(".....");
      Serial.print("Week ");
      Serial.println((j + 1));  

      Serial.print(".....");
      Serial.print(".....");
      
      int min = monthlySunsets[i][1] + j * monthlySunsets[i][2];
      int bias = 0;            
      if(min < 0){
        bias = -1;
        min = min + 60;
      }else if(min > 59){
        bias = 1;
        min = min - 60;
      }
      Serial.print(monthlySunsets[i][0] + bias);
      Serial.print(":");
      Serial.println(min);

    }
  }
}

void print_time(){
  Serial.print(tm.Hour);
  Serial.print(":");
  Serial.println(tm.Minute);

  // Serial.print(tm.Day);
  // Serial.print("/");
  // Serial.println(tm.Month);
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

void print_current_sunrise(){
  calculate_current_sunrise();
  Serial.print("sunrise: ");
  Serial.print(current_sunrise_hour);
  Serial.print(":");
  Serial.println(current_sunrise_min); 
}

void print_current_sunset(){
  calculate_current_sunrise();
  Serial.print("sunset: ");
  Serial.print(current_sunset_hour);
  Serial.print(":");
  Serial.println(current_sunset_min); 
}

void print_min_around_sunset(){
  Serial.print("min_to_sunset: ");
  Serial.println(min_to_sunset());

  Serial.print("min_after_sunset: ");
  Serial.println(min_after_sunset());

  Serial.print("min_to_sunrise: ");
  Serial.println(min_to_sunrise());
}


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/  SETUP _/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void setup() {
  Serial.begin(9600);
  while (!Serial){};

  pixels.begin();

  pinMode(motor_a, OUTPUT);
  pinMode(motor_b, OUTPUT);
  pinMode(endstop_not_open, INPUT_PULLUP);
  pinMode(endstop_not_close, INPUT_PULLUP);  

  // show_sunrises();
  // show_sunsets();
  //print_all_calculations();
  
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

void check_for_command(){
  incomingByte = 0;  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
  }

  switch(incomingByte){
    case 0:
      break;      
    case -1:
      break;      
    case 49: //1
      close_door();
      break;
    case 50: //2
      open_door();
      break;  
  }
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

int calculate_diff_min(int hour1, int hour2, int min1, int min2){
  int hour_diff = hour1 - hour2;
  int min_diff = min1 - min2;

  if(min_diff < 0 && hour_diff > 0){
    hour_diff--;
    min_diff = 60 + min_diff;
  }
  return hour_diff * 60 + min_diff;
}

int min_after_sunset(){
  //print_current_sunset();
  calculate_current_sunset();
  return calculate_diff_min(_hour, current_sunset_hour, _min, current_sunset_min);
}

int min_to_sunset(){
  //print_current_sunset();
  calculate_current_sunset();
  return calculate_diff_min(current_sunset_hour, _hour, current_sunset_min, _min);
}

int min_to_sunrise(){
  //print_current_sunrise();
  calculate_current_sunrise();
  return calculate_diff_min(current_sunrise_hour, _hour, current_sunrise_min, _min);
}

void check_door_action(){
  // Serial.print("Checking door, status: ");
  // Serial.println(door_status);

  switch(door_status){
    case UP: 
      if(min_after_sunset() == DOOR_CLOSING_OFFSET){
        close_door();
      }
      break;
    case DOWN:
      if(min_to_sunrise() == DOOR_OPENING_OFFSET){
        open_door();
      }
      break;
  }
}

//for lamp
void set_lamp(int intensity){
  if(intensity == -1) {
    r = 0;
    g = 0;
    b = 0;    
  }else if(intensity > 15){
    r = map(intensity, 0, 90, 150, 255);
    g = map(intensity, 0, 90, 20, 255);
    b = map(intensity, 0, 90, 0, 100);
  }else{
    r = map(intensity, 0, 90, 20, 255);
    g = map(intensity, 0, 90, 0, 255);
    b = 0;
  }
  for(int i = 0; i < NUMPIXEL; i++){
    pixels.setPixelColor(i, pixels.Color(r, g, b)); //RGB!
  }
  pixels.show();
}

void check_lamp_action(){
  if(min_after_sunset() > LAMP_ON_OFFSET && min_after_sunset() < LAMP_OFF_OFFSET){ //LAMP_ON_OFFSET < min_after_sunset() < LAMP_OFF_OFFSET
    set_lamp(45 - min_after_sunset());
  }else{
    set_lamp(-1);
  }
}

void test_lamp(){
  for(int i = 80; i > 0; i--){
    set_lamp(i);
    delay(350);
  }  
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/  LOOP  _/_/_/_/_/_/_/_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

void loop() {  
  stop_door();

  update_time();
  print_time();
  print_min_around_sunset();

  check_for_command();
  //print_current_sunset();
  //print_current_sunrise();

  check_lamp_action();
  check_door_action();

  delay(1000);
}
