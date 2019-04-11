#ifndef BRYCES_SSD1306_GRAPH
#define BRYCES_SSD1306_GRAPH

#ifndef _Adafruit_SSD1306_H_
#include "Adafruit_SSD1306.h"
#endif

#ifndef _ADAFRUIT_GFX_H
#include "Adafruit_SSD1306.h"
#endif

#define MIN_WINDOW_HEIGHT 50

class SSD1306_Graph
{
public:
  SSD1306_Graph();

  void drawGraph(Adafruit_SSD1306 disp,
  int x,
  int y,
  int w,
  int h,
  int vals[],
  int val_count,
  uint16_t colour);

int scaleVal(int value, int h);
private:
  int _min( int vals[],
            int val_count);
  int _max( int vals[],
            int val_count);
  int _range( int vals[],
              int val_count);
  void _setMinMax(int vals[],
              int val_count);
  int min_val;
  int max_val;
  int range_val;
};
#endif
