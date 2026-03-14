#include <Adafruit_NeoPixel.h>
#include "rgb/myrgb.h"

#define RGB_Pin 4

Adafruit_NeoPixel *pixels;

int numPixels   = 2;
int pixelFormat = NEO_GRB + NEO_KHZ800;

void setup_rgb(void)
{
  pixels = new Adafruit_NeoPixel(numPixels, RGB_Pin, pixelFormat);
  pixels->begin();
  rgb_mode(1,false);
}

void rgb_mode(unsigned char mode,bool state)
{
  pixels->clear();
  switch(mode)
  {
    case 0:
      break;
    case 1:
      pixels->setPixelColor(0, pixels->Color(0, 0, state ? 255 : 0));
      break;
    case 2:
      pixels->setPixelColor(1, pixels->Color(state ? 255 : 0, 0, 0));
      break;
    default:
      break;
  }
  pixels->show();
}