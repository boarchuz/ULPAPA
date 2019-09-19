# ULPAPA
This is a ULP coprocessor-based driver for APA102-type RGB LEDs on the ESP32. It can be used as a standalone driver or as a controller for FastLED.

## Overview

An array of ulp_led_t is created in RTC slow memory. The app manipulates these based on the desired colour, brightness, etc., then sets a flag when ready to be written out. The ULP polls this flag, then bitbangs the buffer out to connected APAs.

There are very few good reasons to use this over other hardware drivers such as RMT, I2S, etc. It is significantly slower (150-250kHz) and the maximum number of LEDs is limited by the small amount of available RTC slow memory.

It is very easy to use though. FastLED integration can be achieved in just a few lines!

## Compatible Pins

Pins used for data and clock must be in the RTC domain and output-capable.
These are: 0,2,4,12,13,14,15,25,26,27,32,33

## FastLED Usage

Short:

Open up the FastLED example provided with this library. Take note of ULPAPAData, ULPAPAFastLED and FastLED.addLeds(). That's it. Only those 3 lines are different. Use FastLED like you normally would.

Long:

In addition to the usual FastLED array of CRGB/CHSV, another array must be allocated in memory accessible to the ULP coprocessor. This can be easily declared using the ULPAPAData macro. eg.
```
#define NUM_LEDS 10
CRGB myFastLeds[NUM_LEDS]; //Normal FastLED array
ULPAPAData apaData[NUM_LEDS]; //Also need to allocate RTC memory using "ULPAPAData"
```
Then declare a ULPAPAFastLED controller, passing the name of the "ULPAPAData" array from above. eg.
```
#define DATA_PIN 13
#define CLK_PIN 14
#define COLOR_ORDER GRB
ULPAPAFastLED<DATA_PIN,CLK_PIN,apaData,NUM_LEDS,COLOR_ORDER> ulpFastLEDDriver;
```
You then need to pass that controller to FastLED.addleds(). eg.
```
FastLED.addLeds(&ulpFastLEDDriver, myFastLeds, NUM_LEDS).setCorrection(TypicalLEDStrip);
```
Careful not to get the FastLED array (CRGB/CHSV) and the ULPAPAData array mixed up!

You can then use FastLED as usual.

## Standalone Usage

Declare an array of ulp_led_t in RTC slow memory. It's highly recommended to use the ULPAPAData macro. eg.
```
#define NUM_LEDS 10
ULPAPAData apaData[NUM_LEDS];
```
Declare a ULPAPADriver, passing the ULPAPAData array from above. eg.
```
ULPAPADriver ulpDriver(apaData,NUM_LEDS);
```
The driver can then be started by passing the pins to begin():
```
#define DATA_PIN 13
#define CLK_PIN 14
ulpDriver.begin(DATA_PIN,CLK_PIN);
```
It's a good idea to set the brightness to max at this point:
```
ulpDriver.setBrightness(255);
```
setBrightness is one of a few basic functions included in the driver to manipulate the APA data. Individual APAs in the ULPAPAData array may also be manipulated directly.
Use show() to actually transmit the buffer to the APAs.
eg.
```
#define RGB_RED (255 << 16)
#define RGB_BLUE 255
ulpDriver.fill(RGB_BLUE); //Set all to blue
apaData[0].rgb(RGB_RED);  //Set the first one to red
ulpDriver.show();         //Update
```
See the example for more.
