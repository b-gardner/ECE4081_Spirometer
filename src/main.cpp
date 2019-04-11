#include <Arduino.h>
// User selectable variables
#define SAMPLE_PERIOD 25 // time between samples in ms
#define DISPLAY_PERIOD 200
#define EMA_WEIGHT 0.3 // ema = Exponential Moving Average, handy b/c don't need to
                      // remember extra values
#define NUM_GRAPH_VALS 60
//Setup specific definitions
#define READ_PIN A0

//Globals
int value_recorded = 0;
int value_displayed = 0;
float ema_value = 0;

// Display Settings
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI:
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
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

  display.begin(SSD1306_SWITCHCAPVCC);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(7,0);             // Start at top-left corner
  display.println(F("Spirometer"));
  display.setTextSize(1);
  display.setCursor(30,15);
  display.println(F("Welcome"));
  display.setCursor(15,24);
  display.println(F("ECE4081 Project"));
  display.display();
  delay(2000);
  display.fillRect(0,15,128,49,BLACK);
  display.setTextSize(1);
  display.setCursor(6,15);
  display.print(F("last:"));

}


void loop() {

  if(!value_recorded && (millis() % SAMPLE_PERIOD) < 2){ // these lines service analog write every samplePeriod ms
    int read_value = analogRead(READ_PIN);

    for(int i = NUM_GRAPH_VALS; i >= 1; i --){ // scroll values along the matrix
      values[i] = values[i-1];
    }

    values[0] = read_value;

    ema_value = (1.0-EMA_WEIGHT)*ema_value + EMA_WEIGHT*((float) read_value);
    Serial.print(read_value);
    Serial.print(" ");
    Serial.println(ema_value);
    value_recorded = 1;

  }
  else if(value_recorded && (millis() % SAMPLE_PERIOD) > 2) {
    value_recorded = 0;
  }

  if(!value_displayed && (millis() % DISPLAY_PERIOD) < 10) { // these lines display to the OLED every displayPeriod ms
    // Display single number
    display.fillRect(36,15,25,7,BLACK);
    display.setCursor(36,15);
    display.print(values[0]);

    //display graph
    graph_obj.drawGraph(display,0,63,77,38,values,NUM_GRAPH_VALS,WHITE);
    display.display();
  }
  else if(value_displayed && (millis() % DISPLAY_PERIOD) > 10) {
    value_displayed = 0;
  }

}
