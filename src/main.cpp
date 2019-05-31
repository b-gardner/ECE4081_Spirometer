#include <Arduino.h>
#include <math.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <EEPROM.h>

// User selectable variables
#define SAMPLE_PERIOD 25 // time between samples in ms
#define disp_PERIOD 200
#define EMA_WEIGHT 0.3 // ema = Exponential Moving Average, handy b/c don't need to
                      // remember extra values
#define NUM_GRAPH_VALS 60
//Setup specific definitions
#define READ_PIN A0
#define SW_PIN  8
#define CAL_PIN 6

#define RESTING_VALUE 780
#define THRESHOLD_LIMIT 10

#define CALIBRATION_VOLUME 4.2

//Globals
int value_recorded = 0;
int value_displayed = 0;
float ema_value = 0;
float Vdd = 5.0; //check if 5 exactly, might be slightly off
int PSI = 0;
float voltage = 0;
float area = 0.001794509139657; //cross sectional area of tube in m^2 
float pressure_PSI, pressure_Pa, flow_rate;
float rho = 1.1225; //air density in kg/m^3
float D = 23.9*pow(10,-3);
float d = D*0.25;
float Cd = 0.6; //coeff of discharge of an orfice meter, supposed to be between 0.6-0.85
float beta = d/D;
float E = 1; //compressibility factor, 1 for incompressible air/fluid, <1 for others
int calibrated = 0;
int offset_value = 0;

int absInt(int x);

// disp Settings

#define SCREEN_WIDTH 128 // OLED disp width, in pixels
#define SCREEN_HEIGHT 64 // OLED disp height, in pixels

// Declaration for SSD1306 disp connected using software SPI:
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 disp(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// my graphing "library"
#include <SSD1306_Graph.h>
SSD1306_Graph graph_obj;
int values[NUM_GRAPH_VALS] = {};


//debug variables
int loop_count = 0;
void setup() {

  Serial.begin(115200);
  pinMode(READ_PIN,INPUT);

  pinMode(SW_PIN,INPUT_PULLUP);
  pinMode(CAL_PIN,INPUT_PULLUP);

  disp.begin(SSD1306_SWITCHCAPVCC);

  disp.clearDisplay();
  disp.setTextSize(2);
  disp.setTextColor(WHITE);        // Draw white text
  disp.setCursor(7,0);             // Start at top-left corner
  disp.println(F("Spirometer"));
  disp.setTextSize(1);
  disp.setCursor(30,15);
  disp.println(F("Welcome"));
  disp.setCursor(15,24);
  disp.println(F("ECE4081 Project"));
  disp.display();
  delay(1000);

// get calib offset value
  offset_value = analogRead(READ_PIN);
  disp.print("Offset:");
  disp.print(offset_value);
  Serial.print("Offset Value:");
  Serial.println(offset_value);
  delay(1000);

  disp.fillRect(0,15,128,49,BLACK);
  disp.setTextSize(1);
  disp.setCursor(6,15);
  disp.print(F("last:"));

}


void loop() {
  unsigned long time_read = millis();
  unsigned long last_time_read;
  double volume_total = 0;
  double cal_volume = 0;
  double calibration_value;
  EEPROM.get(0,calibration_value); // random value

  int prev_sw_read = 1;
  int prev_cal_read = 1;
  uint32_t cal_start_time = 0;
  uint32_t breath_start_time = 0;

  while(1){
    if(!value_recorded && (millis() % SAMPLE_PERIOD) < 2){ // these lines service analog write every SAMPLE_PERIOD ms
      last_time_read = time_read;
      int read_value = analogRead(READ_PIN);
      time_read = millis();

      int now_sw_read = digitalRead(SW_PIN);
      if(now_sw_read == 0 && prev_sw_read == 1){
        volume_total = 0;
        breath_start_time = millis();
        disp.setCursor(96,40);
        disp.print("B");
        Serial.print(F("BREATH_START "));
        offset_value = analogRead(READ_PIN);
      }
      if(now_sw_read == 0){ // SW_PIN marks a breath
        volume_total += (absInt(read_value-offset_value))*(time_read-last_time_read)*calibration_value;
      }
      if(now_sw_read == 1){
        disp.fillRect(96,40,5,8,BLACK);
      }
      if(now_sw_read == 1 && prev_sw_read == 0 && (millis() - breath_start_time)>750){
        disp.fillRect(90,40,5,8,BLACK);
        Serial.print(F("Breath End, Volume: "));
        Serial.print(volume_total);
        Serial.println("L ");
      }

      prev_sw_read = now_sw_read;
      

      int now_cal_read = digitalRead(CAL_PIN);
      if(now_cal_read == 0 && prev_cal_read == 1){
        cal_volume = 0;
        cal_start_time = millis();
        disp.setCursor(90,40);
        disp.print("C");
        Serial.print(F("CAL_START "));
        offset_value = analogRead(READ_PIN);
      }
      if(now_cal_read == 0 || (millis() - cal_start_time)<750 ){ // SW_PIN marks a breath
        cal_volume += (absInt(read_value-offset_value))*(time_read-last_time_read);
      }
      if(now_cal_read == 1 && prev_cal_read == 0 && (millis() - cal_start_time)>750){
        calibration_value = CALIBRATION_VOLUME/cal_volume;
        EEPROM.put(0,calibration_value);
        disp.fillRect(90,40,5,8,BLACK);
        Serial.print("CAL_END------ ");
        Serial.println(calibration_value);
      }

      prev_cal_read = now_cal_read;

      // voltage = (read_value*Vdd)/1023;
      // pressure_PSI = (7.5)*(voltage - 3.822);  //7.5 = 15/2 from the formula. voltage to PSI from a similar pressure sensor, change if not accurate
      // pressure_Pa = pressure_PSI*6894.75729; //in Pascals
      // //mass_flow = sqrt((pressure_Pa*2)*rho)/(1/pow(area,2); //volumeric flow rate in m^3/s
      // flow_rate = Cd/(sqrt(1-pow(beta,4)))*E*pow(d,2)*sqrt(2*rho*read_value);
    
      for(int i = NUM_GRAPH_VALS; i >= 1; i --){ // scroll values along the matrix
        values[i] = values[i-1];
      }

      values[0] = read_value;
      //values[0] = pressure_PSI;
      ema_value = (1.0-EMA_WEIGHT)*ema_value + EMA_WEIGHT*((float) read_value); //averaging?

      // print out values, compatiable with Arduino Serial Plotter
      // Serial.print("\n read_value: ");
      // Serial.print(read_value);
      //Serial.print(", ema_value: ");
      //Serial.println(ema_value);
      // Serial.print(", voltage: ");
      // Serial.print(voltage);
    //   Serial.print(",\tPa: ");
    //   Serial.print(pressure_Pa);
    //   Serial.print("\t FLow R8:");
    //   Serial.print(flow_rate*1000);
    //   value_recorded = 1;
    }
    else if(value_recorded && (millis() % SAMPLE_PERIOD) > 2) {
      value_recorded = 0;
    }

    if(!value_displayed && (millis() % disp_PERIOD) < 10) { // these lines disp to the OLED every dispPeriod ms
      // disp single number
      disp.fillRect(36,15,75,7,BLACK);
      disp.setCursor(36,15);
      disp.print(values[0]);
      
      disp.print(" vol:");
      disp.print(volume_total);

      //disp graph
      graph_obj.drawGraph(disp,0,63,77,38,values,NUM_GRAPH_VALS,WHITE);
      disp.display();
    }
    else if(value_displayed && (millis() % disp_PERIOD) > 10) {
      value_displayed = 0;
    }
    _delay_us(500);
  }
}


int absInt (int x){
  if(x>0) return x;
  else return (-x); 
}