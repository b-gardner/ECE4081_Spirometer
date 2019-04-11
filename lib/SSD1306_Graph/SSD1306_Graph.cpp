#include "SSD1306_Graph.h"

SSD1306_Graph::SSD1306_Graph(){}

void SSD1306_Graph::drawGraph(Adafruit_SSD1306 disp, int x, int y, int w, int h,  int vals[],  int val_count, uint16_t colour)
{
  /*
    @author: Bryce Gardner
    @since: 10/04/19
    @description: This function will display a graph using an ssd1306 OLED display
    @param x: the x position of the lower left corner of the graph
    @param y: y position of lower left corner of graph
    @param w: the width of the graph window (in pixels)
    @param h: the height of the graph window (in pixels)
    @param vals: the values to be displayed int only
    @param val_count: the number of values to be displayed
  */
    int x_bucket_width = w/val_count;
    _setMinMax(vals, val_count);

    disp.fillRect(x,y-h,w,h,BLACK); // erase previous graph

    // force min_max to me minimum of MIN_WINDOW_HEIGHT values apart
    if (SSD1306_Graph::range_val < MIN_WINDOW_HEIGHT)
    {
      int space_to_add = (MIN_WINDOW_HEIGHT - SSD1306_Graph::range_val) /2;

      if(SSD1306_Graph::min_val - space_to_add < 0)
      {
        SSD1306_Graph::min_val = 0;
        SSD1306_Graph::range_val = MIN_WINDOW_HEIGHT;
        SSD1306_Graph::max_val = 50;
      }
      else
      {
        SSD1306_Graph::min_val -= space_to_add;
        SSD1306_Graph::max_val += space_to_add;
        SSD1306_Graph::range_val = max_val - min_val;
      }
    }

    //print axis values
    //disp.setRotation(1); // TODO ensure this is the right rotation
    disp.setTextSize(1);
    disp.setTextColor(WHITE);
    disp.setCursor(x,y-8);
    disp.print(SSD1306_Graph::min_val);

    disp.setCursor(x,y-h+1);
    disp.print(SSD1306_Graph::max_val);
    //print y_axis line
    disp.drawFastVLine(x+17,y-h,h,WHITE);
    disp.drawFastHLine(x,y-h,17,WHITE);
    //
    // // print x_axis line
    disp.drawFastHLine(x,y,w,WHITE);

    // note: we will have values scroll from the right across, so first pixel will go to x+w x position
    // do y-scaling
    for(int i = 0; i < val_count; i++){ // for each value
      int pix_y = y - 1 - SSD1306_Graph::scaleVal(vals[i],h); // get its y offset
      int pix_x = x + w - (i+1)*x_bucket_width; // get its x_offset
      disp.drawFastHLine(pix_x,pix_y,x_bucket_width,WHITE); // draw the value (a line b/c it might)
        // TODO: USE line interpolation rather than a horizontal line
    }
}


int SSD1306_Graph::_min( int vals[], int val_count)
{
  int min = vals[0];

  for(int i = 1; i < val_count; i++){
    if (vals[i] < min) min = vals[i];

  }

  return min;
}


int SSD1306_Graph::_max( int vals[], int val_count)
{
  int max = vals[0];

  for(int i = 1; i < val_count; i++){
    if (vals[i] > max) max = vals[i];

  }
  return max;
}


int SSD1306_Graph::_range( int vals[], int val_count)
{
  int max = vals[0];
  int min = vals[0];

  for(int i = 1; i < val_count; i++){
    if (vals[i] > max) max = vals[i];
    if (vals[i] < min) min = vals[i];
  }
  return max-min;
}

void SSD1306_Graph::_setMinMax(int vals[], int val_count)
{
  SSD1306_Graph::min_val = _min(vals,val_count);
  SSD1306_Graph::max_val = _max(vals,val_count);
  SSD1306_Graph::range_val = SSD1306_Graph::max_val - SSD1306_Graph::min_val;
}

int SSD1306_Graph::scaleVal(int value, int h)
{
  /*
  @author: Bryce Gardner
  @since: 10/04/19
  @description: once min/max values have been set (with set minMax), this outputs
    an integer y-offset corresponding to the input "value"
  @param value: the value we are going to scale
  @param h: the maximum offset we are scaling to
  @TODO: check how much quicker it is if we remove float math

  */

  // int temp = value - SSD1306_Graph::min_val;
  // float normalised = ((float) temp)/((float) SSD1306_Graph::range_val);
  // int ret = ((int) (normalised * h));
  // if (ret > max_val) return h;
  // else if (ret < min_val) return 0;
  // else return ret;
h -=1;
int temp = value - SSD1306_Graph::min_val;
int temp2 = temp*h;
return (temp2 / SSD1306_Graph::range_val);

}
