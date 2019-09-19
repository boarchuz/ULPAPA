#ifndef ULPAPAFastLED_h

#include "ULPAPADriver.h"
#include "FastLED.h"

/// ULP APA Controller class for FastLED
/// @tparam DATAPIN the pin to write data out on
/// @tparam CLOCK_PIN the clock pin for these leds
/// @tparam ULPAPAs pointer to the RTC data
/// @tparam NUMAPAs the number of APAs
template<uint8_t DATA_PIN, uint8_t CLOCK_PIN, ulp_apa_led_t *ULPAPAs, uint16_t NUMAPAs, EOrder RGB_ORDER = RGB>
class ULPAPAFastLED : public CPixelLEDController<RGB_ORDER> {
	ULPAPADriver ulp;

public:
	ULPAPAFastLED() : ulp(ULPAPAs,NUMAPAs) {}

	virtual void init() {
		ulp.begin(DATA_PIN,CLOCK_PIN);
		ulp.setBrightness(255);
	}

protected:

	virtual void showPixels(PixelController<RGB_ORDER> & pixels) {
		uint16_t index = 0;
        while (pixels.has(1)) {
			ulp.setIndex(index,pixels.loadAndScale0() << 16 | pixels.loadAndScale1() << 8 | pixels.loadAndScale2());
            pixels.advanceData();
            pixels.stepDithering();
            index++;
        }
		ulp.show();
	}
};

#endif