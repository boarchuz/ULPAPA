#include "ULPAPADriver.h"

#include "sdkconfig.h"
#include "esp32/ulp.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "soc/rtc_periph.h"

//Patch in required ULP instructions if ESP-IDF < 3.2.3
//https://github.com/espressif/esp-idf/commit/88a69823cfb23359a0bbb116ea71355c59756739
#ifndef I_JUMPS
	#define SUB_OPCODE_ALU_CNT 2    /*!< Arithmetic instruction between counter register and an immediate */
	#define ALU_SEL_SINC  0         /*!< Increment counter register */
	#define ALU_SEL_SRST  2         /*!< Reset counter register */
	#define SUB_OPCODE_BS  2        /*!< Branch to relative PC */
	#define JUMPS_LT 0              /*!< Branch if counter register < */

	#define I_JUMPS(pc_offset, imm_value, comp_type) { .halt = {\
		.unused = (SUB_OPCODE_BS << 25) | (((pc_offset >= 0) ? 0 : 1) << 24) | (abs(pc_offset) << 17) | (comp_type << 15) | imm_value, \
		.opcode = OPCODE_BRANCH } }

	#define I_STAGE_INC(imm_) { .halt = {\
		.unused = (SUB_OPCODE_ALU_CNT << 25) | (ALU_SEL_SINC << 21) | (imm_ << 4), \
		.opcode = OPCODE_ALU } }

	#define I_STAGE_RST() { .halt = {\
		.unused = (SUB_OPCODE_ALU_CNT << 25) | (ALU_SEL_SRST << 21), \
		.opcode = OPCODE_ALU } }
#endif

#if !(defined CONFIG_ESP32_ULP_COPROC_RESERVE_MEM) && defined(CONFIG_ULP_COPROC_RESERVE_MEM)
		#define CONFIG_ESP32_ULP_COPROC_RESERVE_MEM CONFIG_ULP_COPROC_RESERVE_MEM
#endif

ULPAPADriver::ULPAPADriver(ulp_apa_led_t *leds, uint16_t num)
{
	_leds = leds;
	_num = num;
}

void ULPAPADriver::show()
{
	// ulp_var_t *led0 = (ulp_var_t*)_leds;
	// led0->put(led0->get() | 0x8000);
	_leds[0].flagTransmit();
}

void ULPAPADriver::setBrightness(uint8_t brightness)
{
	for(int i=0;i<_num;i++)
	{
		_leds[i].brightness(brightness / 8);
	}
}

void ULPAPADriver::fill(uint32_t rgb)
{
	for(int i=0;i<_num;i++)
	{
		_leds[i].rgb(rgb);
	}
}

void ULPAPADriver::setIndex(uint16_t index, uint32_t rgb)
{
	_leds[index].rgb(rgb);
}

void ULPAPADriver::copy24(void *src)
{
	uint8_t *arr24 = (uint8_t *)src;
	for(int i=0; i<_num;i++)
	{
		_leds[i].rgb(arr24[i*3] << 16 | arr24[i*3+1] << 8 | arr24[i*3+2]);
	}
}

void ULPAPADriver::showFastLEDs(void *src)
{
	copy24(src);
	show();
}

void ULPAPADriver::begin(uint8_t data, uint8_t clock)
{
	gpio_num_t pin_clock = (gpio_num_t)clock;
	gpio_num_t pin_data = (gpio_num_t)data;

	initPin(pin_clock);
	initPin(pin_data);

	#define LED_ARR_OFFSET (((uint32_t)_leds - ((uint32_t)RTC_SLOW_MEM)) / 4)
	
	const ulp_insn_t ulpProgram[] = {
		//Reset Pointer
		I_MOVI(R1,LED_ARR_OFFSET),

		//Wait here until SoC sets flag to transmit
		I_LD(R0,R1,0),
		I_BL(-1, 1 << 15),

		//Clear the flag
		I_ANDI(R0,R0,0x7FFF),
		I_ST(R0,R1,0),

		//Transmit 32-bit start frame (0 * 32)
		I_STAGE_RST(),
        I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_STAGE_INC(1),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_JUMPS(-3,32,JUMPS_LT),

		//Now loop through all the APAs
		I_STAGE_RST(),

		//Load the first 16 bits of this APA
		I_LD(R0,R1,0),

		//3 bits 111
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_data].rtc_num), 1),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_STAGE_INC(1),
		I_LSHI(R0, R0, 1),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_JUMPS(-4,3,JUMPS_LT),

		//5 bits brightness, 8 bits blue
		I_BL(3, 32768),
        I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_data].rtc_num), 1),
        I_BGE(2,0),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_data].rtc_num), 1),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
        I_STAGE_INC(1),
        I_LSHI(R0, R0, 1),
        I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_JUMPS(-8,16,JUMPS_LT),

		//Increment pointer
		I_ADDI(R1,R1,1),

		//Load the next 16 bits of this APA
		I_LD(R0,R1,0),

		//8 bits green, 8 bits red
		I_BL(3, 32768),
        I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_data].rtc_num), 1),
        I_BGE(2,0),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_data].rtc_num), 1),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
        I_STAGE_INC(1),
        I_LSHI(R0, R0, 1),
        I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_JUMPS(-8,32,JUMPS_LT),

		//Increment pointer
		I_ADDI(R1,R1,1),

		//Check if all are done
		I_SUBI(R0,R1,LED_ARR_OFFSET + _num * 2),
		I_BXFI(10),

		//Transmit 32-bit end frame (1 * 32)
		I_STAGE_RST(),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_data].rtc_num), 1),
        I_WR_REG_BIT(RTC_GPIO_OUT_W1TS_REG, RTC_GPIO_OUT_DATA_W1TS_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_STAGE_INC(1),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_clock].rtc_num), 1),
		I_JUMPS(-3,32,JUMPS_LT),
		I_WR_REG_BIT(RTC_GPIO_OUT_W1TC_REG, RTC_GPIO_OUT_DATA_W1TC_S + (rtc_gpio_desc[pin_data].rtc_num), 1),

		//Loop
		I_BXI(0),
	};

	size_t programSize = sizeof(ulpProgram) / sizeof(ulp_insn_t);
	ESP_ERROR_CHECK(ulp_process_macros_and_load(0, ulpProgram, &programSize));
	// ESP_ERROR_CHECK(ulp_set_wakeup_period(x);
	ESP_ERROR_CHECK(ulp_run(0));
}

void ULPAPADriver::initPin(gpio_num_t pin)
{
	ESP_ERROR_CHECK(rtc_gpio_init(pin));
	rtc_gpio_set_level(pin, 0);
	rtc_gpio_pullup_dis(pin);
	rtc_gpio_pulldown_dis(pin);
	rtc_gpio_set_direction(pin, RTC_GPIO_MODE_OUTPUT_ONLY);
}