#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
RTC_DS3231 rtc;

SoftwareSerial primary_bluetooth(16, 14);
SoftwareSerial secondary_bluetooth(8, 9);

#define MODE_SPEED 13
#define MODE_COOLANT 5
#define MODE_OIL_TEMP 92
#define MODE_FUEL_PRESSURE 10
#define MODE_DISTANCE 49

String lcd_first_line;
String lcd_second_line;

char months[12][4] = {"Jan\0", "Feb\0", "Mar\0", "Apr\0", "May\0", "Jun\0", "Jul\0", "Aug\0", "Sep\0", "Oct\0", "Nov\0", "Dec\0"};
char weekday[7][4] = {"Sun\0", "Mon\0", "Tue\0", "Sat\0", "Thr\0", "Fri\0"};

int cur;
byte button_pins[3] = {6, 5, 4};
bool button_state[3];

char last;

byte clock_state = 0;
byte clock_buff = 90;

byte idx = 0;
byte prev_cmd = -1;
int commands[5] = {MODE_SPEED, MODE_COOLANT, MODE_OIL_TEMP, MODE_FUEL_PRESSURE, MODE_DISTANCE};


void print_time_segment(byte segment) {
  if (segment < 10) {
    lcd.print('0');
  }
  
  lcd.print(segment);
}
 
void print_time(int state) {
  DateTime now = rtc.now();
  
  switch(state) {
    case 1:
      if (last != now.minute() || clock_buff != state) {
        lcd.clear();
        lcd.setCursor(2, 1);
     
        lcd.print(now.year());
        lcd.print("-");
        lcd.print(months[now.month() - 1]);
        lcd.print("--");
        print_time_segment(now.day());
        lcd.print(" ");

        lcd.setCursor(2, 0);
    
        lcd.print(weekday[now.dayOfTheWeek()]);
        lcd.print(" "); 
        
        print_time_segment(now.hour());
        lcd.print(":");
        print_time_segment(now.minute());
        lcd.print(":");
      }

      lcd.setCursor(12, 0);
      print_time_segment(now.second());
      lcd.print("     ");

      last = now.minute();
      clock_buff = state;
      break;

    case 0:
      lcd.setCursor(1, 0);

      if (last != now.minute() || clock_buff != state) {
        lcd.clear();
        lcd.setCursor(1, 0);
        print_time_segment(now.month());
        lcd.print("-");
        print_time_segment(now.day());
        lcd.print(" ");
        print_time_segment(now.hour());
        lcd.print(":");
        print_time_segment(now.minute());
        lcd.print(":");
      }
      
      lcd.setCursor(13, 0);
      print_time_segment(now.second());
      lcd.print("     ");

      last = now.minute();
      clock_buff = state;
      break;
    case 2:
        if (clock_buff != state) {
          lcd.clear();
        }
        clock_buff = state;
        return;
  }
}

int get_data(SoftwareSerial *bluetooth_module) {
     if (bluetooth_module->available()) {
       String str = bluetooth_module->readStringUntil('\0');

       return str.toInt(); 
     }
}

int get_speed() {
  
}

void mode_switching(char *message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: ");
  lcd.setCursor(0, 1);
  lcd.print(message);
  delay(1000);
  lcd.clear();
  clock_buff = -1;
}

void preview_data(int command_idx) {  
  switch (commands[command_idx]) {
    case MODE_SPEED:
        mode_switching("Vehicle Speed");      
      break;
    case MODE_COOLANT:
        mode_switching("Coolant Temp.");
    break;
      case MODE_OIL_TEMP:
          mode_switching("Oil Temperature");
    break;
      case MODE_FUEL_PRESSURE:
          mode_switching("Fuel Pressure");
    break;
      case MODE_DISTANCE:
          mode_switching("Distance");
      break;
  }
}

void lcd_print(int clock_state, int command) {
  clock_state %= 3;

  int res;

  if (command != prev_cmd) {
    secondary_bluetooth.println(commands[command]);

    preview_data(command);
    lcd.clear();
    
    prev_cmd = command;
  }

  lcd.setCursor(0, 1);

  if (clock_state == 2) {
    lcd.setCursor(0, 0);
  }

  res = get_data(&secondary_bluetooth);

  if (clock_state != 1) {
    switch(commands[command]) {
      case MODE_SPEED:
        lcd.print(res);
        lcd.print(" km/h              ");
        break;
      case MODE_COOLANT:
        lcd.print(res);
        lcd.print(" deg C             ");
        break;
      case MODE_OIL_TEMP:
        lcd.print(res);
        lcd.print(" deg C             ");
        break;
      case MODE_FUEL_PRESSURE:
        lcd.print(res * 3);
        lcd.print(" kPa               ");
        break;
      case MODE_DISTANCE:
        //int mod = res % 256;
        //res -= mod;
        //res = float()
        lcd.print(res);
        lcd.print(" km                ");
        break;
    }
    
  }
 
  print_time(clock_state);
  
  
}
//*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  primary_bluetooth.begin(9600);
  secondary_bluetooth.begin(9600);

  rtc.begin();
  Wire.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(8, INPUT);
  pinMode(9, OUTPUT);

  pinMode(16, INPUT);
  pinMode(14, OUTPUT);

  for (int i = 0; i < 3; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
  }

}



void loop() {
  for (int i = 0; i < 3; i++) {
    cur = digitalRead(button_pins[i]);

    if (cur != button_state[i]) {
      button_state[i] = cur;
      if (button_state[i] == 0) {
        bool first_button = digitalRead(button_pins[0]);
        bool second_button = digitalRead(button_pins[1]);
        bool third_button = digitalRead(button_pins[2]);

        delay(100);

        if (!digitalRead(button_pins[(i + 1) % 3])) {
          delay(900);
          if (!digitalRead(button_pins[(i + 2) % 3])) {
            lcd.clear();
            lcd.init();
            //lcd.print("RESET");
            lcd.clear();
            delay(100);
            clock_buff = 90;
            continue;
          }
        }

        if (!first_button && second_button && third_button) {
          idx--;
        }

        if (first_button && second_button && !third_button) {
          idx++;
        }
          
        if (first_button && !second_button && third_button) {
          clock_state++;
        }
          
      }
    }
  }
  
  lcd_print(clock_state, idx % 5);

  //Serial.println((float)analogRead(10) / 1024 * 5);
  
}
