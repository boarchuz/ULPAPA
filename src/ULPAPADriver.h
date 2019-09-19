#ifndef ULPAPADriver_h
#define ULPAPADriver_h

#include "sdkconfig.h"
#include "driver/gpio.h"

#define ULPAPAData RTC_SLOW_ATTR WORD_ALIGNED_ATTR ulp_apa_led_t

struct ulp_apa_led_t {
	uint8_t red(){return _raw.red;};
	uint8_t green(){return _raw.green;};
	uint8_t blue(){return _raw.blue;};
	void red(uint8_t val){_raw.red = val;};
	void green(uint8_t val){_raw.green = val;};
	void blue(uint8_t val){_raw.blue = val;};

	uint8_t brightness(){return _raw.global;};

	/**
	 * @brief Directly set the brightness of this APA. Note: 5-bit value.
	 * 
	 * @param  val Desired brightness, 0(off)-31(max)
	 */
	void brightness(uint8_t val){_raw.global = val & 0x1F;};

	uint32_t rgb(){return (uint32_t)_raw.red << 16 | (uint32_t) _raw.green << 8 | _raw.blue;}
	void rgb(uint32_t val){_raw.red = (val >> 16) & 0xFF; _raw.green = (val >> 8) & 0xFF; _raw.blue = val & 0xFF;};

	/**
	 * @brief When set on the first ULP APA in the array, this triggers the ULP to transmit out the buffer.
	 * Use ULPAPA::show() instead.
	 */
	void flagTransmit(){_raw.update_flag = 1;};

	private:
		struct {
			uint32_t blue : 8;         	/*!< 8 bits blue */
			uint32_t global : 5;        /*!< 5 bit 'global' (brightness) value */
			uint32_t unused : 2;        /*!< First 3 bits unused (1 employed as update_flag) */
			uint32_t update_flag : 1;   /*!< Bit for SoC to flag ULP to transmit (not APA protocol) */
			
			uint32_t upper1 : 16;       /*!< 16 bits inaccessible to ULP */
			
			uint32_t red : 8;         	/*!< 8 bits red */
			uint32_t green : 8;         /*!< 8 bits green */

			uint32_t upper2 : 16;       /*!< 16 bits inaccessible to ULP */
		} _raw;
};

class ULPAPADriver
{
	public:
		/**
		 * @param  leds Pointer to the ULP_APA_DATA array
		 * @param  num The number of APAs
		 */
		ULPAPADriver(ulp_apa_led_t *leds, uint16_t num);

		/**
		 * @brief Initialise the pins, load the ULP APA driver into memory and start the coprocessor. 
		 *
		 * @param  pin_clock Clock GPIO (Must be in RTC domain!)
		 * @param  pin_data Data GPIO (Must be in RTC domain!)
		 *
		 */
		void begin(uint8_t pin_data, uint8_t pin_clock);

		/**
		 * @brief Update APAs. This will trigger the ULP to transmit the buffer.
		 *
		 */
		void show();

		/**
		 * @brief Set the global brightness value for all APAs.
		 * 
		 * While this function accepts an 8-bit value (0-255), it will be scaled down accordingly as the APA
		 * protocol accepts a 5-bit value (0-31).
		 * This should generally be avoided except in cases where resolution is not important (eg. setting to 
		 * max brightness or turning off).
		 * For much smoother dimming effects, it is strongly advised to scale down the RGB values in order to
		 * take advantage of their 8-bit resolution.
		 *
		 * @param  brightness Desired brightness 0(off)-255(max)
		 *
		 */
		void setBrightness(uint8_t brightness);

		/**
		 * @brief Set the colour of all APAs with the given RGB value.
		 *
		 * @param  rgb Desired 32-bit RGB colour
		 *
		 */
		void fill(uint32_t rgb);

		/**
		 * @brief Set one by index.
		 * 
		 * @param  index Index of the LED to set
		 * @param  rgb Desired 32-bit RGB colour
		 *
		 */
		void setIndex(uint16_t index, uint32_t rgb);

		/**
		 * 
		 * @brief Update the buffer with 24-bit RGB data then transmit.
		 * The array is assumed to have the same number of elements as the number of ULP APAs.
		 * *** Use ULPAPAFastLED controller instead ***
		 *
		 * @param  src Pointer to the 24-bit RGB array (eg. FastLED CRGBs)
		 *
		 */
		void showFastLEDs(void *src);

	private:
		ulp_apa_led_t *_leds;
		uint16_t _num;

		void initPin(gpio_num_t pin);
		void copy24(void *src);
};

#endif