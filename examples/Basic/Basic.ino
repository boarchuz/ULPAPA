#include <ULPAPADriver.h>

#define DATA_PIN    13
#define CLK_PIN   14
#define NUM_LEDS    64

#define BRIGHTNESS 127

ULPAPAData ulpleds[NUM_LEDS];
ULPAPADriver ulpDriver(ulpleds,NUM_LEDS);

void setup() {
  ulpDriver.begin(DATA_PIN,CLK_PIN);
  ulpDriver.setBrightness(BRIGHTNESS);
}

#define RGB_OFF 0
#define RGB_RED (255<<16)
#define RGB_GREEN (255<<8)
#define RGB_BLUE (255)
#define RGB_YELLOW (RGB_RED | RGB_GREEN)
#define RGB_PURPLE (RGB_RED | RGB_BLUE)
#define RGB_WHITE (RGB_RED | RGB_GREEN | RGB_BLUE)
  
void loop()
{
  ulpDriver.setBrightness(BRIGHTNESS);
  
  ulpDriver.fill(RGB_RED);
  ulpDriver.show();
  delay(1000);
  
  ulpDriver.fill(RGB_GREEN);
  ulpDriver.show();
  delay(1000);
  
  ulpDriver.fill(RGB_BLUE);
  ulpDriver.show();
  delay(1000);
  
  for(int i=128;i>=0;i--)
  {
	  ulpDriver.fill( ((128 - i) << 16) | (i << 8) | (128 - i) );
	  ulpDriver.show();
	  delay(20);
  }
  for(int i=1;i<=128;i++)
  {
	  ulpDriver.fill( ((i) << 16) | ((128 - i) << 8) | (i) );
	  ulpDriver.show();
	  delay(20);
  }
  
  for(int a=0; a<6; a++)
  {
	  for(int i=0; i< NUM_LEDS; i++)
	  {
	    ulpDriver.setIndex(i, a%2 ? (i%2 ? RGB_YELLOW : RGB_BLUE) : (i%2 ? RGB_BLUE : RGB_YELLOW) );
	  }
	  ulpDriver.show();
	  delay(500);
  }
  
  ulpDriver.setBrightness(0);
  for(int i=0;i<4 && i<NUM_LEDS;i++)
  {
	  ulpDriver.setIndex(i,0);
	  ulpleds[i].brightness(31);
  }
  for(int a=0;a<255;a++)
  {
	  ulpleds[0].blue(ulpleds[0].blue() + 1);
	  for(int i=1;i<4 && i<NUM_LEDS;i++)
    {
	    ulpDriver.setIndex(i,a << ((i-1) * 8));
    }
	  ulpDriver.show();
	  delay(10);
  }
  
  ulpDriver.fill(RGB_OFF);
  ulpDriver.setBrightness(BRIGHTNESS);
  ulpleds[NUM_LEDS-1].rgb(RGB_GREEN);
  for(int i=NUM_LEDS-1;i>=0;i--)
  {
	  ulpleds[i].rgb(ulpleds[NUM_LEDS-1].rgb());
	  ulpDriver.show();
	  delay(50);
  }
  ulpleds[0].rgb(RGB_RED);
  ulpDriver.show();
  delay(2000);
  
  
  ulpDriver.fill(RGB_WHITE);
  for(int i=0;i<4;i++)
  {
	  ulpDriver.setBrightness(0);
	  ulpDriver.show();
	  delay(250);
	  ulpDriver.setBrightness(BRIGHTNESS);
	  ulpDriver.show();
	  delay(250);
  }

  //Can take an array of 24-bit RGBs, such as FastLED CRGBs (see the dedicated controller instead if using FastLED). Demonstrating with an array of random bytes here.
  uint8_t randomRgbData[3 * NUM_LEDS];
  for(int i=0; i<sizeof(randomRgbData);i++)
  {
    randomRgbData[i] = random(0,255);
  }
  ulpDriver.setBrightness(BRIGHTNESS / 2);
  ulpDriver.showFastLEDs(randomRgbData);
  delay(2000);
  
}