/**
/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Arduboy2Core.h"

#ifdef JOYSTICKANALOG
uint8_t Arduboy2Core::ADCJoystickState = 0;
unsigned int Arduboy2Core::JoystickXZero = 5000; // first run indicator. number greater than 2^10 (greater than 2^12 for mik32)
unsigned int Arduboy2Core::JoystickYZero = 5000; // first run indicator. number greater than 2^10 (greater than 2^12 for mik32)
#endif
#ifdef BEARBOARD
	uint8_t Arduboy2Core::chan_converted = 0;
	uint8_t Arduboy2Core::chan_selected = 0; 
#endif

#ifndef BEARBOARD
	#include <avr/wdt.h>
#else	
	#include <wdt.h>
#endif

#ifndef OLED_CONTRAST
# define OLED_CONTRAST 0x80 // 0xCF for high contrast or 0x80 low contrast
#endif

//========================================
//========== class Arduboy2Core ==========
//========================================


#if defined (TFT_ST7735_BLK) || defined (TFT_IL9341)

	

	//uint16_t color_buffer[WIDTH * HEIGHT];
	//uint16_t row_buffer[8 * WIDTH];  // 2 КБ вместо 16 КБ

	

	
	#if defined SCALED
		#define SCALE 2
		uint8_t dstBuffer[WIDTH*SCALE];    
	#else
		#define SCALE 1
	#endif

//uint16_t color_buffer[WIDTH*SCALE];
uint8_t color_buffer[WIDTH*SCALE];
#endif 
// Commands sent to the OLED display to initialize it
const PROGMEM uint8_t Arduboy2Core::lcdBootProgram[] = {
  // boot defaults are commented out but left here in case they
  // might prove useful for reference
  //
  // Further reading: https://www.adafruit.com/datasheets/SSD1306.pdf
  
#if defined(GU12864_800B)
  0x24, 0x40,                   // enable Layer 0, graphic display area on
  0x47,                         // set brightness
  0x64, 0x00,                   // set x position 0
  0x84,                         // address mode set: X increment
#elif defined(OLED_SH1106) || defined(OLED_SH1106_I2C)
  0x8D, 0x14,                   // Charge Pump Setting v = enable (0x14)
  #ifndef FLIPPED
	// Set Segment Re-map (A0) | (b0001)
	// default is (b0000)
	0xA1,
	// Set COM Output Scan Direction
	0xC8,
  #else
  	0xA0,
	0xC0,
  #endif
  0x81, OLED_CONTRAST,          // Set Contrast v = 0xCF
  0xD9, 0xF1,                   // Set Precharge = 0xF1
  OLED_SET_COLUMN_ADDRESS_LO,   //Set column address for left most pixel
  0xAF                          // Display On
#elif defined (TFT_IL9341)
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  TFTCMD_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  TFTCMD_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  TFTCMD_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  TFTCMD_VMCTR2  , 1, 0x86,             // VCM control2
  //TFTCMD_MADCTL  , 1, 0x48,             // Memory Access Control
  TFTCMD_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  TFTCMD_PIXFMT  , 1, 0x55,
  //TFTCMD_PIXFMT  , 1, 0x11,  
  TFTCMD_FRMCTR1 , 2, 0x00, 0x18,
  TFTCMD_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  TFTCMD_GAMMASET , 1, 0x01,             // Gamma curve selected
  TFTCMD_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  TFTCMD_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  TFTCMD_SLPOUT  , 0x80,                // Exit Sleep
  TFTCMD_DISPON  , 0x80,                // Display on
  //0x00                                   // End of list
  //TFTCMD_MADCTL, 1, 0x00,
  TFTCMD_MADCTL, 1, 0x20
  
#elif defined(TFT_ST7735_BLK)

// 7735R init, part 1 (red or green tab)
                             // 15 commands in list:
    TFTCMD_SWRESET,   TFTCMD_DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    TFTCMD_SLPOUT,    TFTCMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    TFTCMD_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    TFTCMD_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    TFTCMD_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    TFTCMD_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    TFTCMD_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    TFTCMD_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    TFTCMD_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    TFTCMD_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    TFTCMD_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    TFTCMD_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    TFTCMD_INVOFF,  0,              // 13: Don't invert display, no args
    TFTCMD_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    TFTCMD_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
      0x05,                       //     16-bit color
// next part
                            //  2 commands in list:
    TFTCMD_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x00,                   //     XEND = 127
    TFTCMD_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x00,                  //     XEND = 159// 7735R 3F=63
									
								//	init, part 3 (red or green tab)
                              //  4 commands in list:
    TFTCMD_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    TFTCMD_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
      0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    TFTCMD_NORON,     TFTCMD_DELAY, //  3: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    TFTCMD_DISPON,    TFTCMD_DELAY, //  4: Main screen turn on, no args w/delay
      100,                         //     100 ms delay

// Black TAB:
	TFTCMD_MADCTL, 1, 
	//0xC0
	0xAE
	
  
#elif defined(LCD_ST7565)
  0xC8,                         //SET_COM_REVERSE
  0x28 | 0x7,                   //SET_POWER_CONTROL  | 0x7
  0x20 | 0x5,                   //SET_RESISTOR_RATIO | 0x5
  0x81,                         //SET_VOLUME_FIRST
  0x13,                         //SET_VOLUME_SECOND
  0xAF                          //DISPLAY_ON
#elif defined(OLED_96X96) || defined(OLED_128X96) || defined(OLED_128X128) || defined(OLED_128X64_ON_96X96) || defined(OLED_128X64_ON_128X96) || defined(OLED_128X64_ON_128X128)|| defined(OLED_128X96_ON_128X128) || defined(OLED_96X96_ON_128X128) || defined(OLED_64X128_ON_128X128)
 #if defined(OLED_96X96) || defined(OLED_128X64_ON_96X96)
  0x15, 0x10, 0x3f, //left most 32 pixels are invisible
 #elif defined(OLED_96X96_ON_128X128)
  0x15, 0x08, 0x37, //center 96 pixels horizontally
 #elif defined(OLED_64X128_ON_128X128)
  0x15, 0x10, 0x2f, //center 64 pixels horizontally
 #else
  0x15, 0x00, 0x3f, //Set column start and end address
 #endif
 #if defined (OLED_96X96) 
  0x75, 0x20, 0x7f, //Set row start and end address
 #elif defined (OLED_128X64_ON_96X96) 
  0x75, 0x30, 0x6f, //Set row start and end address
 #elif defined (OLED_128X96)
  0x75, 0x00, 0x5f, //Set row start and end address
 #elif defined(OLED_128X64_ON_128X96)
  0x75, 0x10, 0x4f, //Set row start and end address
 #elif defined(OLED_96X96_ON_128X128) || defined(OLED_128X96_ON_128X128)
  0x75, 0x10, 0x6f, //Set row start and end address to centered 96 lines
 #elif defined(OLED_128X64_ON_128X128)
  0x75, 0x20, 0x5f, //Set row start and end address to centered 64 lines
 #else
  0x75, 0x00, 0x7F, //Set row start and end address to use all 128 lines
 #endif
 #if defined(OLED_64X128_ON_128X128)
  0xA0, 0x51,       //set re-map: split odd-even COM signals|COM remap|column address remap
 #else
  0xA0, 0x55,       //set re-map: split odd-even COM signals|COM remap|vertical address increment|column address remap
 #endif
  0xA1, 0x00,       //set display start line
  0xA2, 0x00,       //set display offset
  //0xA4,           //Normal display
  0xA8, 0x7F,       //Set MUX ratio 128MUX
  //0xB2, 0x23,
  //0xB3, 0xF0,     //set devider clock | oscillator frequency
  0x81, OLED_CONTRAST, //Set contrast
  //0xBC, 0x1F,     //set precharge voltage
  //0x82, 0xFE,     //set second Precharge speed
  0xB1, 0x21,       //reset and 1st precharge phase length  phase 2:2 DCLKs, Phase 1: 1 DCLKs
  //0xBB, 0x0F,     //set 2nd precharge period: 15 DCLKs
  //0xbe, 0x1F,     //output level high voltage com signal
  //0xB8, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, //set gray scale table
  0xAF              //Display on
#else
  // for SSD1306 and SSD1309 displays
  //
  // Display Off
  // 0xAE,
	#if defined(OLED_SSD1306_SPI)
		0xA6, // сброс инверсии
		0xA4, // команда отключения режима 0xA5
		0xAE, 0x8D, 0x10,
	#endif

  // Set Display Clock Divisor v = 0xF0
  // default is 0x80

  0xD5, 0x80, //0xF0 for low frequency or 0x80 for high frequency (Reduces coil noise for some OLED displays)

  // Set Multiplex Ratio v = 0x3F
  // 0xA8, 0x3F,

  // Set Display Offset v = 0
  // 0xD3, 0x00,

  // Set Start Line (0)
  // 0x40,
 #if defined OLED_SSD1309
  //Charge Pump command not supported, use two NOPs instead to keep same size and easy patchability
  0xE3, 0xE3,
 #else  
  // Charge Pump Setting v = enable (0x14)
  // default is disabled
  0x8D, 0x14,
 #endif


  #ifndef FLIPPED
  // Set Segment Re-map (A0) | (b0001)
  // default is (b0000)
  0xA1,
  // Set COM Output Scan Direction
	0xC8,
  #else
  	0xA0,
	0xC0,
  #endif

  // Set COM Pins v
  // 0xDA, 0x12,

  // Set Contrast v = 0xCF
  0x81, OLED_CONTRAST,

  // Set Precharge = 0xF1
  0xD9, 0xF1,

  // Set VCom Detect
  // 0xDB, 0x40,

  // Entire Display ON
  // 0xA4,

  // Set normal/inverse display
  // 0xA6,

  // Display On
  0xAF,

  // set display mode = horizontal addressing mode (0x00)
  0x20, 0x00,

 #if defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SSD1306_SPI)
  // set col address range
  0x21, 0x00, COLUMN_ADDRESS_END,

  // set page address range
  0x22, 0x00, PAGE_ADDRESS_END
 #endif
 
#endif
};

void Arduboy2Core::boot()
{


  #ifdef ARDUBOY_SET_CPU_8MHZ
  // ARDUBOY_SET_CPU_8MHZ will be set by the IDE using boards.txt
  setCPUSpeed8MHz();
  #endif

  // Select the ADC input here so a delay isn't required in generateRandomSeed()
  #ifndef BEARBOARD



  ADMUX = RAND_SEED_IN_ADMUX;

  bootPins();
  bootSPI();
  bootOLED();
  
  bootPowerSaving(); 
    
  #else

	  bootPins();

	  #ifdef SPIBEAR
		bootSPI();
	  #endif


	  bootOLED();	  

  #endif
}

#ifdef ARDUBOY_SET_CPU_8MHZ
// If we're compiling for 8MHz we need to slow the CPU down because the
// hardware clock on the Arduboy is 16MHz.
// We also need to readjust the PLL prescaler because the Arduino USB code
// likely will have incorrectly set it for an 8MHz hardware clock.
void Arduboy2Core::setCPUSpeed8MHz()
{
  uint8_t oldSREG = SREG;
  cli();                // suspend interrupts
  PLLCSR = _BV(PINDIV); // dissable the PLL and set prescale for 16MHz)
  CLKPR = _BV(CLKPCE);  // allow reprogramming clock
  CLKPR = 1;            // set clock divisor to 2 (0b0001)
  PLLCSR = _BV(PLLE) | _BV(PINDIV); // enable the PLL (with 16MHz prescale)
  SREG = oldSREG;       // restore interrupts
}
#endif

// Pins are set to the proper modes and levels for the specific hardware.
// This routine must be modified if any pins are moved to a different port
void Arduboy2Core::bootPins()
{
#ifdef ARDUBOY_10
#ifdef ECONSOLE
  	DDRC &= ~(_BV(RAND_SEED_IN_BIT)); // as input; это можно не добавлять. значение по-умолчанию "0"
	PORTC &= ~(_BV(RAND_SEED_IN_BIT)); // without pullup; это можно не добавлять. значение по-умолчанию "0"
	//PORTC |= (_BV(RAND_SEED_IN_BIT)); // подтяжка изменяет измеряемое значение, но не увеличивает разборс
  #ifndef  JOYSTICKANALOG
	// Port  INPUT_PULLUP
	PORTD |= _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) |
           _BV(B_BUTTON_BIT) |
		   _BV(RIGHT_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT);
	DDRD &= ~(_BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) |
	    _BV(B_BUTTON_BIT)|
		_BV(RIGHT_BUTTON_BIT) |
	    _BV(DOWN_BUTTON_BIT));
	PORTD |= _BV(A_BUTTON_BIT);
	DDRD &= ~(_BV(A_BUTTON_BIT)); 
  
	DDRB  |= _BV(RED_LED_BIT);
	DDRC  |= _BV(BLUE_LED_BIT) | _BV(GREEN_LED_BIT);
  #else
	//JOYSTICKANALOG
	power_adc_enable(); //disable power saving for ADC
	DDRC &= ~(_BV(X_AXIS_BIT) | _BV(Y_AXIS_BIT));
	PORTC |= _BV(X_AXIS_BIT) | _BV(Y_AXIS_BIT);
	PORTD |= _BV(B_BUTTON_BIT) | _BV(A_BUTTON_BIT);
    DDRD &= ~(_BV(B_BUTTON_BIT) | _BV(A_BUTTON_BIT));
	
	    // random init
	power_adc_enable(); //disable power saving for ADC

	ADMUX = REF_BACK_IN_ADMUX;
	ADCSRA=0b10010011; // ADEN=1, ADSC=0 (stop conversion), ADATE=0, ADIF=1 (conversion complete), ADIE=0 (no interputs), ADPS=011 CK/8
	ADCSRA |= _BV(ADSC); // start conversion (ADMUX has been pre-set in boot())
	while ((ADCSRA >> ADSC) & 1); // wait for conversion complete
		
		
  #endif
  // switch off LEDs by default
  PORTC &= ~(_BV(GREEN_LED_BIT)   | _BV(BLUE_LED_BIT) | _BV(RED_LED_BIT)); // если бы светодиоды там были, их надо было бы выключить....
#elif defined (BEARBOARD)
	
	//включаем тактирование GPIO_0, GPIO_1, ADC
	PM->CLK_APB_P_SET |=  PM_CLOCK_APB_P_GPIO_0_M | PM_CLOCK_APB_P_GPIO_1_M | PM_CLOCK_APB_P_ANALOG_REGS_M; 
	//включаем тактирование контроллера выводов, Wake up, Power manager
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M; 
	// инициализация I2C
	
	#if defined(OLED_SSD1306_I2C) || defined(OLED_SH1106_I2C)
		
		// порт селектора: I2C/аналог включаем A7/D25 (ADC5) к PORT 0.9
		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * SELA_A_PIN)); // Обнуление  (в режим GPIO)
		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * SELA_B_PIN)); // Обнуление  (в режим GPIO)	
		GPIO_1->DIRECTION_OUT = (1 << SELA_A_PIN) ;
		GPIO_1->DIRECTION_OUT = (1 << SELA_B_PIN) ;
		GPIO_1->SET = (1 << SELA_A_PIN) ; // ace-uno, ace-nano
		GPIO_1->SET = (1 << SELA_B_PIN) ; // ace-nano
		
		
		
		// SDA and SCL as inputs without pullups
		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * I2C_SDA)); // Обнуление для вывода 12 порта 1 (в режим GPIO)
		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * I2C_SCL)); // Обнуление для вывода 13 порта 1 (в режим GPIO)
		//PAD_CONFIG->PORT_1_PUPD &= ~(0b11 << (2 * I2C_SDA)); // Обнуление. Отключается подтяжка при работе в режиме выхода
		//PAD_CONFIG->PORT_1_PUPD &= ~(0b11 << (2 * I2C_SCL)); // Обнуление. Отключается подтяжка при работе в режиме выхода
		//PAD_CONFIG->PORT_1_DS &= ~(0b11 << (2 * I2C_SDA)); // Обнуление.
		//PAD_CONFIG->PORT_1_DS &= ~(0b11 << (2 * I2C_SCL)); // Обнуление
		PAD_CONFIG->PORT_1_DS |= (0b10 << (2 * I2C_SDA)); // Нагрузочная способность 8 мА
		PAD_CONFIG->PORT_1_DS |= (0b10 << (2 * I2C_SCL)); // Нагрузочная способность 8 мА
		i2c_stop();
	#endif		
	#if defined (OLED_SSD1306_SPI) || defined(TFT_ST7735_BLK)  || defined (TFT_IL9341)// OLED_SSD1306_SPI  !(defined(OLED_SSD1306_I2C) || defined(OLED_SH1106_I2C)) 

		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * CSOUT_BIT)); // Обнуление  (в режим GPIO)
		GPIO_1->DIRECTION_OUT = (1 << CSOUT_BIT);
		//GPIO_1->CLEAR = (1 << CSOUT_BIT) ; // oled display enabled
		GPIO_1->SET = (1 << CSOUT_BIT) ;// отключаем OLED на время инициализации (чтобы туда не улетел мусор, если OLED уже инициализирован)

		PM->CLK_APB_P_SET |=  PM_CLOCK_APB_P_SPI_0_M; // включаем тактирование SPI_0

		// порт селектора: SPI
		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * SELSPI_BIT)); // Обнуление  (в режим GPIO)
		GPIO_1->DIRECTION_OUT = (1 << SELSPI_BIT) ;
		GPIO_1->SET = (1 << SELSPI_BIT) ; // ace-uno, ace-nano SPI отключает nss IN, включает nss out к D9

		PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * CSIN_BIT)); // Обнуление (в режим GPIO)
		PAD_CONFIG->PORT_0_CFG |= (0b01 << (2 * CSIN_BIT)); // режим SPI
		PAD_CONFIG->PORT_0_PUPD |= (0b01 << (2 * CSIN_BIT)); // подтяжка к +
		//GPIO_0->DIRECTION_OUT = (1 << CSIN_BIT);
	
		// data mode
		PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * DC_BIT)); // Обнуление (в режим GPIO)
		GPIO_0->DIRECTION_OUT = (1 << DC_BIT) ;
		GPIO_0->SET = (1 << DC_BIT) ; // data mode		

	  		
		//PAD_CONFIG->PORT_0_PUPD |= (0b01 << (2 * CSOUT_BIT)); // подтяжка к +
		
		// создает трафик несущесвующих данных и команд, но обнуляет старший разряд. на случай режима b10 
		PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * SPI_MOSI_BIT)); // Обнуление (в режим GPIO)
		PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * SPI_SCK_BIT)); // Обнуление (в режим GPIO)
		PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * SPI_MISO_BIT)); // Обнуление (в режим GPIO)
	
		PAD_CONFIG->PORT_0_CFG |= (0b01 << (2 * SPI_MOSI_BIT)); //   (в режим 01 SPI)
		PAD_CONFIG->PORT_0_CFG |= (0b01 << (2 * SPI_SCK_BIT)); //   (в режим 01 SPI)
		PAD_CONFIG->PORT_0_CFG |= (0b01 << (2 * SPI_MISO_BIT)); //  (в режим 01 SPI)
		

		
	#endif


	
	// инициализация EEPROM. Пользовательские данные Arduino содержатся с EEPROM_START_ADDR=0x1C00 до EEPROM_END=0x1FFF

	EEPROM_REGS->EECON = 0;
	// инициализация ADC
	
	chan_selected = CHAN_RANDOM; 
	
	PAD_CONFIG->PORT_0_CFG |= (0b11 << (2 * PIN_RANDOM)); // аналоговый сигнал. порт 
 
	//PAD_CONFIG->PORT_0_PUPD &= ~(0b11 << (2 * PIN_RANDOM)); // без подтяжки
	GPIO_0->DIRECTION_IN = 1 << PIN_RANDOM; //	
	//PAD_CONFIG->PORT_0_PUPD |= (0b10 << (2 * PIN_RANDOM)); // подтяжка к -. ну
	//PAD_CONFIG->PORT_0_PUPD |= (0b01 << (2 * PIN_RANDOM)); // подтяжка к PW. Нужна для ACE-NANO, у которой без подтяжки не "шумят" аналоговые каналы A0-A2 
	

	
	#if defined (JOYSTICKANALOG)
		PAD_CONFIG->PORT_1_CFG |= (0b11 << (2 * PIN_AXISX)); // аналоговый сигнал. порт A0=1.5
		PAD_CONFIG->PORT_1_CFG |= (0b11 << (2 * PIN_AXISY)); // аналоговый сигнал. порт A1=1.7
		GPIO_1->DIRECTION_IN = 1 << PIN_AXISX; // 
		GPIO_1->DIRECTION_IN = 1 << PIN_AXISY; //
		PAD_CONFIG->PORT_0_PUPD |=  (0b01 << (2 * B_BUTTON_BIT) | 0b01 << (2 * A_BUTTON_BIT));
		
	  	 //
		 GPIO_1->DIRECTION_OUT = (1 << RED_LED_BIT) | (1 << BLUE_LED_BIT) | (1 << GREEN_LED_BIT);
		 GPIO_1->CLEAR = (1 << RED_LED_BIT) | (1 << BLUE_LED_BIT) | (1 << GREEN_LED_BIT); //RGB LED off

	#elif defined (JOYSTICKDISCRETE)
		//JOYSTICKDISCRETE

		  // Задаем направление без "|=", т.к. для установки DIRECTION - только запись "1"
		  PAD_CONFIG->PORT_0_PUPD |=  (0b01 << (2 * LEFT_BUTTON_BIT) | 0b01 << (2 * RIGHT_BUTTON_BIT) | 0b01 << (2 * UP_BUTTON_BIT) | 0b01 << (2 * DOWN_BUTTON_BIT) );
		  PAD_CONFIG->PORT_1_PUPD |=  (0b01 << (2 * B_BUTTON_BIT) | 0b01 << (2 * A_BUTTON_BIT));
		  GPIO_0->DIRECTION_IN = _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT);
		  GPIO_1->DIRECTION_IN = _BV(A_BUTTON_BIT) | _BV(B_BUTTON_BIT);
		  
	  	 GPIO_1->DIRECTION_OUT = (1 << RED_LED_BIT) | (1 << BLUE_LED_BIT) | (1 << GREEN_LED_BIT);
		 GPIO_1->CLEAR = (1 << RED_LED_BIT) | (1 << BLUE_LED_BIT) | (1 << GREEN_LED_BIT); //RGB LED off
	
	#elif defined (SPIBEAR)
		//SPIBEAR
		  // Задаем направление без "|=", т.к. для установки DIRECTION - только запись "1"
		  // подтяжка к "-" pull down т.к. на аналоговых входах частично уже есть  внешняя подтяжка к "-"
		  PAD_CONFIG->PORT_0_PUPD |=  (0b10 << (2 * DOWN_BUTTON_BIT));
		  PAD_CONFIG->PORT_1_PUPD |=  (0b10 << (2 * B_BUTTON_BIT) | 0b10 << (2 * A_BUTTON_BIT) | 0b10 << (2 * LEFT_BUTTON_BIT) | 0b10 << (2 * UP_BUTTON_BIT) | 0b10 << (2 * RIGHT_BUTTON_BIT));
		  GPIO_0->DIRECTION_IN = _BV(DOWN_BUTTON_BIT);
		  GPIO_1->DIRECTION_IN = _BV(A_BUTTON_BIT) | _BV(B_BUTTON_BIT) | _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT);
		  
	  	 GPIO_1->DIRECTION_OUT = (1 << RED_LED_BIT) | (1 << BLUE_LED_BIT) | (1 << GREEN_LED_BIT);
		 GPIO_1->CLEAR = (1 << RED_LED_BIT) | (1 << BLUE_LED_BIT) | (1 << GREEN_LED_BIT); //RGB LED off

	#else // ECOSOLE KEYS
		  //PAD_CONFIG->PORT_0_PUPD &= ~ (0b11 << (2 * LEFT_BUTTON_BIT) | 0b11 << (2 * RIGHT_BUTTON_BIT) | 0b11 << (2 * UP_BUTTON_BIT) | 0b11 << (2 * DOWN_BUTTON_BIT) | 0b11 << (2 * A_BUTTON_BIT));
		  //PAD_CONFIG->PORT_1_PUPD &= ~ (0b11 << (2 * B_BUTTON_BIT));
		  PAD_CONFIG->PORT_0_PUPD |=  (0b01 << (2 * LEFT_BUTTON_BIT) | 0b01 << (2 * RIGHT_BUTTON_BIT) | 0b01 << (2 * UP_BUTTON_BIT) | 0b01 << (2 * DOWN_BUTTON_BIT) | 0b01 << (2 * A_BUTTON_BIT));
		  PAD_CONFIG->PORT_1_PUPD |=  (0b01 << (2 * B_BUTTON_BIT));
		  GPIO_0->DIRECTION_IN = _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) | _BV(A_BUTTON_BIT);
		  GPIO_1->DIRECTION_IN =  _BV(B_BUTTON_BIT);
		
		// задаются как выходы. Но не задействованы на плате
		//GPIO_0->DIRECTION_OUT = _BV(BLUE_LED_BIT);
		//GPIO_1->DIRECTION_OUT = _BV(GREEN_LED_BIT) | _BV(RED_LED_BIT);	
	#endif	

	
	ANALOG_REG->ADC_CONFIG = 0x3c00; // последовательность для инициализации ADC MIK32 из HAL
	//HAL_ADC_Enable(&hadc);
	ANALOG_REG->ADC_CONFIG = (ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_M)) |
                                 ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_M) |
                                 (1 << ADC_CONFIG_EN_S);
	//HAL_ADC_ResetEnable:
	ANALOG_REG->ADC_CONFIG = (ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_M)) |
                                 ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_M) |
                                 (1 << ADC_CONFIG_RESETN_S);
	myADC_SEL_CHANNEL (chan_selected);
	// HAL_ADC_ChannelSet
	ANALOG_REG->ADC_CONFIG |= (ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_M)) |
                                 ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_M) |
                                 (ADC_EXTREF_OFF << ADC_CONFIG_EXTREF_S) |   // Настройка источника опорного напряжения 
                                 (ADC_EXTCLB_ADCREF << ADC_CONFIG_EXTPAD_EN_S); // Выбор внешнего источника опорного напряжения 
	chan_converted=chan_selected;
	ANALOG_REG->ADC_SINGLE=1;  // считаем ерунду
	myADC_SEL_CHANNEL (chan_selected); // переключаем канал
	while (!ANALOG_REG->ADC_VALID) {};
	ANALOG_REG->ADC_SINGLE=1; // считаем канал 3
	while (!ANALOG_REG->ADC_VALID) {};



#else
  // Port B INPUT_PULLUP or HIGH
  PORTB = (0
         #ifndef MICROCADE
          | _BV(RED_LED_BIT) | _BV(BLUE_LED_BIT) //RGB LED off
         #endif
         #ifndef AB_ALTERNATE_WIRING
          | _BV(GREEN_LED_BIT)
         #endif
         #ifdef SUPPORT_XY_BUTTONS
          | _BV(X_BUTTON_BIT) | _BV(A_BUTTON_BIT)
         #else 
          | _BV(B_BUTTON_BIT)
         #endif
         #ifndef ARDUINO_AVR_MICRO
          | _BV(RX_LED_BIT) //RX LED off for Arduboy and non Micro based Arduino
         #endif          
  // Port B INPUT or LOW                    
          ) & ~(_BV(SPI_MISO_BIT) | _BV(SPI_MOSI_BIT) | _BV(SPI_SCK_BIT));

  // Port B outputs
  DDRB = (_BV(RED_LED_BIT)   | _BV(BLUE_LED_BIT)
        #ifndef AB_ALTERNATE_WIRING
         | _BV(GREEN_LED_BIT)
        #endif
         | _BV(SPI_MOSI_BIT) | _BV(SPI_SCK_BIT)  | _BV(RX_LED_BIT)) & ~(
  // Port B inputs
        #ifdef SUPPORT_XY_BUTTONS
         _BV(A_BUTTON_BIT) 
        #else
         _BV(B_BUTTON_BIT) 
        #endif
         | _BV(SPI_MISO_BIT)
         #ifdef SUPPORT_XY_BUTTONS
          | _BV(X_BUTTON_BIT)
         #endif
         );

  // Port C
  // Speaker: Not set here. Controlled by audio class
  // Port D INPUT_PULLUP or HIGH
  PORTD = (
         #if (defined(AB_ALTERNATE_WIRING) && !defined(MICROCADE))
          _BV(GREEN_LED_BIT) |
         #endif
         #if !(defined(ARDUINO_AVR_MICRO))
          _BV(TX_LED_BIT) | //TX LED off for Arduboy and non Micro based Arduino
         #endif          
          _BV(CART_BIT) | 
         #if !(defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C))
          _BV(DC_BIT) |
         #endif
          0) & ~( // Port D INPUTs or LOW outputs
         #if !(defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C))
          _BV(CS_BIT) |  // oled display enabled
          _BV(RST_BIT) | // reset active
         #endif
         #if defined(AB_ALTERNATE_WIRING)
          _BV(SPEAKER_2_BIT) |
         #endif
         #if defined(LCD_ST7565)
          _BV(POWER_LED_BIT) |
         #endif
         #if defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
          _BV(I2C_SCL) |
          _BV(I2C_SDA) |
         #endif
          0);

  // Port D outputs
  DDRD = (
        #if !(defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C))
         _BV(DC_BIT) | 
        #endif
        #if !(defined(AB_ALTERNATE_WIRING) && (CART_CS_SDA))
         _BV(RST_BIT) | 
         _BV(CS_BIT) |
        #endif
        #if defined(AB_ALTERNATE_WIRING)
         _BV(GREEN_LED_BIT) |
        #endif
        #if defined(LCD_ST7565)
         _BV(POWER_LED_BIT) |
        #endif
         _BV(CART_BIT) |
         _BV(TX_LED_BIT) |
         0) & ~(// Port D inputs
         #if defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
          _BV(I2C_SCL) | // SDA and SCL as inputs without pullups
          _BV(I2C_SDA) | // (both externally pulled up)
         #endif
         0);

  // Port E INPUT_PULLUP or HIGH
 #ifndef SUPPORT_XY_BUTTONS
  PORTE |= _BV(A_BUTTON_BIT);
  // Port E INPUT or LOW (none)
  // Port E inputs
  DDRE &= ~(_BV(A_BUTTON_BIT));
  // Port E outputs (none)
 #else
  PORTE |= _BV(B_BUTTON_BIT);
  DDRE &= ~(_BV(B_BUTTON_BIT));
 #endif

  // Port F INPUT_PULLUP or HIGH
  PORTF = (_BV(LEFT_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) |
          _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT)
         #ifdef SUPPORT_XY_BUTTONS
          | _BV(Y_BUTTON_BIT)  
         #endif
          ) &
  // Port F INPUT or LOW
          ~(_BV(RAND_SEED_IN_BIT));
  
  // Port F outputs (none) // в оригинальной версии здесь какая-то дичь. для DDRF выполняется умножением маски на "0"
  DDRF = 0 &
  // Port F inputs
         ~(_BV(LEFT_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) |
         _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) |
         #ifdef SUPPORT_XY_BUTTONS
          _BV(Y_BUTTON_BIT) | 
         #endif
         _BV(RAND_SEED_IN_BIT));

#endif
#if defined(AB_DEVKIT) 

  // Port B INPUT_PULLUP or HIGH
  PORTB |= _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) |
           _BV(BLUE_LED_BIT);
  // Port B INPUT or LOW (none)
  // Port B inputs
  DDRB &= ~(_BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) |
            _BV(SPI_MISO_BIT));
  // Port B outputs
  DDRB |= _BV(SPI_MOSI_BIT) | _BV(SPI_SCK_BIT) | _BV(BLUE_LED_BIT);

  // Port C INPUT_PULLUP or HIGH
  PORTC |= _BV(RIGHT_BUTTON_BIT);
  // Port C INPUT or LOW (none)
  // Port C inputs
  DDRC &= ~(_BV(RIGHT_BUTTON_BIT));
  // Port C outputs (none)

  // Port D INPUT_PULLUP or HIGH
  PORTD |= _BV(CS_BIT);
  // Port D INPUT or LOW
  PORTD &= ~(_BV(RST_BIT));
  // Port D inputs (none)
  // Port D outputs
  DDRD |= _BV(RST_BIT) | _BV(CS_BIT) | _BV(DC_BIT);

  // Port E (none)

  // Port F INPUT_PULLUP or HIGH
  PORTF |= _BV(A_BUTTON_BIT) | _BV(B_BUTTON_BIT);
  // Port F INPUT or LOW
  PORTF &= ~(_BV(RAND_SEED_IN_BIT));
  // Port F inputs
  DDRF &= ~(_BV(A_BUTTON_BIT) | _BV(B_BUTTON_BIT) | _BV(RAND_SEED_IN_BIT));
  // Port F outputs (none)
  // Speaker: Not set here. Controlled by audio class
#endif
#endif
}

void Arduboy2Core::bootOLED()
{


	
#if defined(GU12864_800B)
  bitSet(RST_PORT,RST_BIT);
  delayByte(10);
  displayEnable();
  for (uint8_t i = 0; i < sizeof(lcdBootProgram) + 8; i++)
  {
    if (i < 8)    
    {
      displayWrite(0x62); // set display area
      displayWrite(i);    // display area address
      LCDDataMode();     
      displayWrite(0xFF); // Graphic display
      LCDCommandMode();
    }
    else 
      displayWrite(pgm_read_byte(lcdBootProgram + i - 8));
  }
  displayDisable();
#elif   defined(OLED_SSD1306_SPI) && defined(OLED_SSD1306_I2C) 
  // reset the display
  uint8_t cmd;
  const uint8_t* ptr = lcdBootProgram;
  delayByte(5);                          //for a short active low reset pulse   
   const uint8_t* end = lcdBootProgram + sizeof(lcdBootProgram);	  
   delayByte(5);     
	 LCDCommandMode();
	while (ptr != end) {
		cmd = pgm_read_byte(ptr++);
        SPItransfer(cmd);
    }
    LCDDataMode();  // Переключиться в режим данных после отправки команд
  i2c_start(SSD1306_I2C_CMD);
  for (uint8_t i = 0; i < sizeof(lcdBootProgram); i++)
    i2c_sendByte(pgm_read_byte(lcdBootProgram + i));
  i2c_stop();	
#elif defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
  i2c_start(SSD1306_I2C_CMD);
  for (uint8_t i = 0; i < sizeof(lcdBootProgram); i++)
    i2c_sendByte(pgm_read_byte(lcdBootProgram + i));
  i2c_stop();
#else // варианты с SPI
	// reset the display
	  uint8_t cmd;
	  const uint8_t* ptr = lcdBootProgram;
	  delayByte(5);                          //for a short active low reset pulse
 #if !(defined(AB_ALTERNATE_WIRING) && defined(CART_CS_SDA)) && !defined(SPIBEAR)
  bitSet(RST_PORT, RST_BIT);             //deactivate reset
 #endif
  delayByte(5);
 #if defined(OLED_128X64_ON_96X96) || defined(OLED_128X64_ON_128X96) || defined(OLED_128X64_ON_128X128)|| defined(OLED_128X96_ON_128X128) || defined(OLED_96X96_ON_128X128) || defined(OLED_64X128_ON_128X128)
  for (uint16_t i = 0; i < 8192; i++) SPItransfer(0); //make sure all display ram is cleared
 #endif
  //bitClear(CS_PORT, CS_BIT);               // select the display as default SPI device, already cleared by boot pins)

 #if defined __AVR_ARCH__  
  LCDCommandMode(); 
  asm volatile
  (
    "3:  lpm  %[cmd], Z+             \n" 
    : [ptr] "+z" (ptr),
      [cmd] "=r" (cmd)
    : 
    :
  );    
  SPItransfer(cmd);                      
  asm volatile(
    "    cpi  r30, lo8(%[lbp_end])   \n" // check only LSB cause size < 256
    "    brne 3b                     \n"
    : 
    : [lbp_end] "" (lcdBootProgram + sizeof(lcdBootProgram))
    :
  );
  LCDDataMode();
 #else 	// только SPIBEAR
	#if defined OLED_SSD1306_SPI
	   

	   LCDCommandMode();
	   const uint8_t* end = lcdBootProgram + sizeof(lcdBootProgram);	  
		while (ptr != end) {

			cmd = pgm_read_byte(ptr++);
			SPItransfer(cmd);
		}
		LCDDataMode();  // Переключиться в режим данных после отправки команд	
	#elif defined (TFT_ST7735_BLK) || defined (TFT_IL9341)
	

	uint8_t numArgs;	
	uint8_t ms;
	const uint8_t* end = lcdBootProgram + sizeof(lcdBootProgram);
		#if defined (TFT_IL9341)
			sendLCDCommand(TFTCMD_SWRESET);
			delayByte(150);
		#endif

	while (ptr != end) {
		  cmd = pgm_read_byte(ptr++);       // Read command
		  numArgs = pgm_read_byte(ptr++);   // Number of args to follow
		  ms = numArgs & TFTCMD_DELAY;       // If hibit set, delay follows args
		  numArgs &= ~TFTCMD_DELAY;          // Mask out delay bit

		  sendTFTCommand(cmd,ptr,numArgs);
		  ptr += numArgs;
		  if (ms) {
			#if defined TFT_ST7735_BLK
				ms = pgm_read_byte(ptr++); // Read post-command delay time (ms)
				if (ms == 255) {delayByte(ms);delayByte(ms);} else {delayByte(ms);}
			#elif defined  TFT_IL9341
				delayByte(150);
			#endif
		  }	
	}
	

	blank();
	#endif
 #endif
#endif  


}

// Initialize the SPI interface for the display
void Arduboy2Core::bootSPI()
{
#ifndef BEARBOARD
// master, mode 0, MSB first, CPU clock / 2 (8MHz)
  SPCR = _BV(SPE) | _BV(MSTR);
  SPSR = _BV(SPI2X);
#else
	#ifdef SPIBEAR

	SPI_0->ENABLE &= ~SPI_ENABLE_M;

	
	SPI_0->ENABLE |= SPI_ENABLE_CLEAR_RX_FIFO_M;
	SPI_0->ENABLE |= SPI_ENABLE_CLEAR_TX_FIFO_M;
	

	
	volatile uint32_t unused = SPI_0->INT_STATUS; /* Очистка флагов ошибок чтением */
    (void) unused;
	
		volatile uint32_t dummy;
		while ((SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M) != 0)
		{
			dummy = SPI_0->RXDATA;
		}
		(void) dummy;
	#if defined (OLED_SSD1306_SPI) 
		SPI_0->CONFIG = SPI_CONFIG_MASTER_M | SPI_CONFIG_BAUD_RATE_DIV_8_M | SPI_CONFIG_MANUAL_CS_M | SPI_CONFIG_CS_NONE_M;  // мастер, деление на 8 т.е. 32/8=4 МГц. В оригинале 16/4=4 МГц. Ручное управление CS
	#elif defined (TFT_ST7735_BLK) || defined (TFT_IL9341)
		SPI_0->CONFIG = SPI_CONFIG_MASTER_M | SPI_CONFIG_BAUD_RATE_DIV_2_M | SPI_CONFIG_MANUAL_CS_M | SPI_CONFIG_CS_NONE_M;  // мастер, деление на 2 т.е. 32/2=16 МГц. В оригинале 16/4=4 МГц. Ручное управление CS
	#endif
	// Декодер - по умолчанию 0, фаза 0, полярность 0
	SPI_0->ENABLE = SPI_ENABLE_M;


		GPIO_1->CLEAR = (1 << CSOUT_BIT) ; // Включаем OLED обратно (выключали, чтобы не прилетал мусор)
	#endif
#endif  
}

// Write to the SPI bus (MOSI pin)






void Arduboy2Core::SPItransfer(uint8_t data)	

{
#ifndef BEARBOARD
  SPDR = data;
  /*
   * The following NOP introduces a small delay that can prevent the wait
   * loop from iterating when running at the maximum speed. This gives
   * about 10% more speed, even if it seems counter-intuitive. At lower
   * speeds it is unnoticed.
   */
  asm volatile("nop");
  while (!(SPSR & _BV(SPIF))) { } // wait
#else
	

#ifdef SPIBEAR
	SPI_0->TXDATA = data;
	while (!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M));
	(void) SPI_0->RXDATA;
#endif
	


#endif
}






#if defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
void Arduboy2Core::i2c_start(uint8_t mode)
{
  #if defined(BEARBOARD)
  I2C_SDA_LOW();       // disable posible internal pullup, ensure SDA low on enabling output
  I2C_SDA_AS_OUTPUT(); // SDA low before SCL for start condition
  I2C_SCL_LOW();
  I2C_SCL_AS_OUTPUT();
  #else
  I2C_SDA_LOW();       // disable posible internal pullup, ensure SDA low on enabling output
  I2C_SDA_AS_OUTPUT(); // SDA low before SCL for start condition
  I2C_SCL_LOW();
  I2C_SCL_AS_OUTPUT();
  #endif
  i2c_sendByte(SSD1306_I2C_ADDR << 1);
  i2c_sendByte(mode);
  
}

void Arduboy2Core::i2c_sendByte(uint8_t byte)
{
  #ifndef BEARBOARD
  uint8_t sda_clr = I2C_PORT & ~((1 << I2C_SDA) | (1 << I2C_SCL));
  uint8_t scl = 1 << I2C_SCL;
  uint8_t sda = 1 << I2C_SDA;
  uint8_t scl_bit = I2C_SCL;  
  asm volatile (    
    "    sec                    \n" // set carry for 8 shift counts
    "    rol  %[byte]           \n" // shift a bit out and count at the same time
    "1:                         \n"
    "    out  %[port], %[sda0]  \n" // preemtively clear SDA
    "    brcc 2f                \n" // skip if dealing with 0 bit
    "    out  %[pin], %[sda]    \n" 
    "2:                         \n" 
    "    out  %[pin], %[scl]    \n" // toggle SCL on
    "    lsl  %[byte]           \n" // next bit to carry (moved here for 1 extra cycle delay)
    "    out  %[pin], %[scl]    \n" // toggle SCL off
    "    brne 1b                \n" // initial set carry will be shifted out after 8 loops setting Z flag
    "                           \n" 
    "    out  %[port], %[sda0]  \n" // clear SDA for ACK
    "    nop                    \n" // extra delay
    "    sbi  %[port], %[sclb]  \n" // set SCL (extends ACK bit by 1 cycle)
    "    cbi  %[port], %[sclb]  \n" // clear SCL (extends SCL high by 1 cycle)
    :[byte] "+r" (byte)
    :[port] "i" (_SFR_IO_ADDR(I2C_PORT)),
     [pin]  "i" (_SFR_IO_ADDR(I2C_PIN)),
     [sda0] "r" (sda_clr),
     [scl]  "r" (scl),
     [sda]  "r" (sda),
     [sclb] "i" (scl_bit)
  );
  #else
	//I2C_SCL_LOW();
	//I2C_SDA_LOW();
	for (uint8_t i=0;i<8;i++)
		{
			if (byte & 0x80) 
			{ I2C_SDA_AS_INPUT();  // лог.1
			} else 
			{	I2C_SDA_AS_OUTPUT();
			}
			byte<<=1; // сдвигаем на 1 бит влево // 5NOP ok for 1306 & 1309
			__NOP();
			__NOP();
			//__5NOP();
			I2C_SCL_AS_INPUT();   // Записать его импульсом на SCL       // отпустить SCL (лог.1) // 5 NOP ok for sh1106 // 20 NOP ok for ssd1309
			//__10NOP();
			//__10NOP();
			//__NOP();
			__NOP();
			__NOP();
			__5NOP();
			I2C_SCL_AS_OUTPUT(); // притянуть SCL (лог.0) // 5 NOP ok for sh1106
			//__5NOP();

		}
		I2C_SDA_AS_INPUT(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK. В оригинальном тексте Arduboy2 тут выставляется лог.0. Вероятно, чтобы не дожидаться, пока это сделает ведомый?
		I2C_SCL_AS_INPUT(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK // 20 NOP ok for ssd1309
			//__10NOP(); 
			//__10NOP();
			__5NOP();
			__NOP();
			__NOP();
			__NOP();
		I2C_SCL_AS_OUTPUT(); // притянуть SCL (лог.0)  // приём ACK завершён // 1 nop for 1309
		__NOP();
  #endif
}
#endif

void Arduboy2Core::safeMode()
{

  if (buttonsState() == UP_BUTTON)
  {
    setRGBledRedOn();
#ifndef BEARBOARD
#ifndef ARDUBOY_CORE // for Arduboy core timer 0 should remain enabled
    // prevent the bootloader magic number from being overwritten by timer 0
    // when a timer variable overlaps the magic number location
    power_timer0_disable();
#endif
 #endif 
    while (true) { }
  }

}


/* Power Management */

void Arduboy2Core::idle()
{
#ifndef BEARBOARD
  SMCR = _BV(SE); // select idle mode and enable sleeping
  sleep_cpu();
  SMCR = 0; // disable sleeping
#endif
}

void Arduboy2Core::bootPowerSaving()
{
#ifndef BEARBOARD
  #if defined(PRR) && !defined(PRR0)
	#if defined (JOYSTICKANALOG)  // disable power saving for ADC
	PRR = _BV(PRTWI);
	#else
	PRR = _BV(PRTWI) | _BV(PRADC);
	#endif
  PRR |= _BV(PRUSART0);
  #else
  // disable Two Wire Interface (I2C) and the ADC
  // All other bits will be written with 0 so will be enabled
 PRR0 = _BV(PRTWI) | _BV(PRADC);
  // disable USART1
  PRR1 |= _BV(PRUSART1);
  #endif
#endif 
}

#if defined(GU12864_800B)
void Arduboy2Core::displayEnable()
{
  bitSet(CS_PORT,CS_BIT);
  SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA);
  //bitClear(CS_PORT,CS_BIT);
  LCDCommandMode();
}

void Arduboy2Core::displayDisable()
{
  //bitSet(CS_PORT,CS_BIT);
  SPCR = _BV(SPE) | _BV(MSTR);
}

void Arduboy2Core::displayWrite(uint8_t data)
{
  bitClear(CS_PORT,CS_BIT);
  SPItransfer(data);
  bitSet(CS_PORT,CS_BIT);
}
#endif



// Shut down the display
void Arduboy2Core::displayOff()
{
#if defined(GU12864_800B)
  displayEnable();
  displayWrite(0x20);
  displayWrite(0x00);
  displayDisable();
#elif defined(OLED_SSD1306_I2C) && defined(OLED_SSD1306_SPI)
  i2c_start(SSD1306_I2C_CMD);    
  i2c_sendByte(0xAE); // display off
  i2c_sendByte(0x8D); // charge pump:
  i2c_sendByte(0x10); //   disable
  i2c_stop();
  LCDCommandMode();
  SPItransfer(0xAE); // display off
  SPItransfer(0x8D); // charge pump:
  SPItransfer(0x10); //   disable
  LCDDataMode();
#elif defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
  i2c_start(SSD1306_I2C_CMD);    
  i2c_sendByte(0xAE); // display off
  i2c_sendByte(0x8D); // charge pump:
  i2c_sendByte(0x10); //   disable
  i2c_stop();
#elif defined (TFT_ST7735_BLK) || defined (TFT_IL9341)
    sendLCDCommand(TFTCMD_DISPOFF);
	sendLCDCommand(TFTCMD_SLPIN);
#else
  LCDCommandMode();
  SPItransfer(0xAE); // display off
  SPItransfer(0x8D); // charge pump:
  SPItransfer(0x10); //   disable
  LCDDataMode();
#endif  
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
  bootOLED();
}





/* Drawing */

void Arduboy2Core::paint8Pixels(uint8_t pixels)
{
#if defined(OLED_SSD1306_I2C) && defined(OLED_SSD1306_SPI)
  i2c_start(SSD1306_I2C_DATA);
  i2c_sendByte(pixels);
  i2c_stop();
  SPItransfer(pixels);
#elif defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
  i2c_start(SSD1306_I2C_DATA);
  i2c_sendByte(pixels);
  i2c_stop();
#else  
  SPItransfer(pixels);
#endif
}

void Arduboy2Core::paintScreen(const uint8_t *image)
{
#if defined(GU12864_800B) 
  displayEnable();
  for (uint8_t r = 0; r < (HEIGHT/8); r++)
  {
    LCDCommandMode();
    displayWrite(0x60);
    displayWrite(r);
    LCDDataMode();
    for (uint8_t c = 0; c < (WIDTH); c++)
    {
      bitClear(CS_PORT,CS_BIT);
      SPDR = pgm_read_byte(image++);
      while (!(SPSR & _BV(SPIF)));
      bitSet(CS_PORT,CS_BIT);
    }
  }
  displayDisable();
#elif defined(OLED_SSD1306_I2C) && defined(OLED_SSD1306_SPI)
  i2c_start(SSD1306_I2C_DATA);
  for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
  {
	i2c_sendByte(pgm_read_byte(image+i));
	SPItransfer(pgm_read_byte(image + i));
  }
  i2c_stop();

#elif defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX)
  i2c_start(SSD1306_I2C_DATA);
  for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
  {
	i2c_sendByte(pgm_read_byte(image+i));
	
  }
  i2c_stop();
#elif defined(TFT_ST7735_BLK) || defined(TFT_IL9341)
  GPIO_1->CLEAR = (1 << CSOUT_BIT);	
  LCDCommandMode();
	SPItransfer(TFTCMD_CASET);
  LCDDataMode();
  SPItransfer(0x00);SPItransfer(0);SPItransfer((WIDTH*SCALE-1)>>8);SPItransfer(WIDTH*SCALE-1); 
  LCDCommandMode();
	SPItransfer(TFTCMD_RASET);
  LCDDataMode();
  SPItransfer(0x00);SPItransfer(0);SPItransfer((HEIGHT*SCALE-1)>>8);SPItransfer(HEIGHT*SCALE-1);
  LCDCommandMode();
	SPItransfer(TFTCMD_RAMWR);
  LCDDataMode();	

    
	
/*
    // Предварительно разворачиваем страницы
    for (int page = 0; page < 8; page++) {
        int base_idx = page * WIDTH;
        
        // Для каждого бита в странице (8 строк)
        for (int bit = 0; bit < 8; bit++) {
            // Выводим все пиксели текущей строки
            for (int x = 0; x < WIDTH; x++) {
                uint8_t byte = image[base_idx + x];
                uint16_t color = (byte & (1 << bit)) ? MONOCHROME_ON : MONOCHROME_OFF;
                SPItransfer(color >> 8);
                SPItransfer(color & 0xFF);
            }
        }
    }
*/



	for (int page = 0; page < 8*SCALE; page++) {
        int base_idx = page * WIDTH*SCALE;
        for (int bit = 0; bit < 8; bit++) {
            for (int x = 0; x < WIDTH*SCALE; x++) {
                #if defined SCALED
					uint8_t byte = dstBuffer[base_idx + x];
				#else
					uint8_t byte = image[base_idx + x];
				#endif 
                color_buffer[x] = ((byte & (1 << bit)) ? MONOCHROME_ON : MONOCHROME_OFF) ;
            }
            //spi_transfer_block((uint8_t*)(color_buffer), WIDTH*2*SCALE);
			spi_transfer_block((uint8_t*)(color_buffer), WIDTH*SCALE);
        }
    }







/*
 // Формируем ПОЛНЫЙ буфер цвета перед отправкой
    for (int page = 0; page < 8; page++) {
        int base_idx = page * WIDTH;  // Смещение в исходном буфере SSD1306
        
        // Формируем 8 строк для текущей "страницы"
        for (int x = 0; x < WIDTH; x++) {
            uint8_t byte = image[base_idx + x];
            
            // Распаковываем 8 вертикальных пикселей в 8 горизонтальных строк
            for (int bit = 0; bit < 8; bit++) {
                // bit (0-7) становится строкой внутри блока из 8 строк
                row_buffer[bit * WIDTH + x] = (byte & (1 << bit)) ? 0xFFFF : 0x0000;
				//row_buffer[bit * WIDTH + x] = BIT_LUT[byte][bit];
            }
        }
        
        // Отправляем блок из 8 строк ОДНИМ вызовом SPI
        spi_transfer_block((uint8_t*)row_buffer, 8 * WIDTH * 2);
    }
*/
		
  GPIO_1->SET = (1 << CSOUT_BIT);
#elif  defined (OLED_SH1106_I2C) 
  for (int page = 0; page < HEIGHT/8; page++)
  {
    i2c_start(SSD1306_I2C_CMD);
    i2c_sendByte(OLED_SET_PAGE_ADDRESS + page); // set page
    i2c_sendByte(OLED_SET_COLUMN_ADDRESS_HI);   // only reset hi nibble to zero
    i2c_stop();
    const uint8_t *line = image + page*WIDTH;
    i2c_start(SSD1306_I2C_DATA);
    for (int i = 0; i < WIDTH; i++)
      i2c_sendByte(pgm_read_byte(line+i));
    i2c_stop();
  }
#elif defined(OLED_SH1106) || defined(LCD_ST7565)
  for (uint8_t i = 0; i < HEIGHT / 8; i++)
  {
    LCDCommandMode();
    SPDR = (OLED_SET_PAGE_ADDRESS + i);
    while (!(SPSR & _BV(SPIF)));
    SPDR = (OLED_SET_COLUMN_ADDRESS_HI); // only reset hi nibble to zero
    while (!(SPSR & _BV(SPIF)));
    LCDDataMode();
    for (uint8_t j = WIDTH; j > 0; j--)
      {
        SPDR = pgm_read_byte(image++);
        while (!(SPSR & _BV(SPIF)));
      }
  }
#elif defined(OLED_96X96) || defined(OLED_128X96) || defined(OLED_128X128) || defined(OLED_128X64_ON_96X96) || defined(OLED_128X64_ON_128X96) || defined(OLED_128X64_ON_128X128) || defined(OLED_128X96_ON_128X128) || defined(OLED_96X96_ON_128X128)
 #if defined(OLED_128X64_ON_96X96)
  uint16_t i = 16;
  for (uint8_t col = 0; col < 96 / 2; col++)
 #else     
  uint16_t i = 0;
  for (uint8_t col = 0; col < WIDTH / 2; col++)
 #endif     
  {
    for (uint8_t row = 0; row < HEIGHT / 8; row++)
    {
      uint8_t b1 = pgm_read_byte(image + i);
      uint8_t b2 = pgm_read_byte(image + i + 1);
      for (uint8_t shift = 0; shift < 8; shift++)
      {
        uint8_t c = 0xFF;
        if ((b1 & 1) == 0) c &= 0x0F;
        if ((b2 & 1) == 0) c &= 0xF0;
        SPDR = c;
        b1 = b1 >> 1;
        b2 = b2 >> 1;
        while (!(SPSR & _BV(SPIF)));
      }
      i += WIDTH;
    }
    i -= HEIGHT / 8 * WIDTH - 2;
  }
#elif defined(OLED_64X128_ON_128X128)
  uint16_t i = WIDTH-1;
  for (uint8_t col = 0; col < WIDTH ; col++)
  {
    for (uint8_t row = 0; row < HEIGHT / 8; row++)
    {
      uint8_t b = pgm_read_byte(image + i);
      for (uint8_t shift = 0; shift < 4; shift++)
      {
        uint8_t c = 0xFF;
        if ((b & _BV(0)) == 0) c &= 0x0F;
        if ((b & _BV(1)) == 0) c &= 0xF0;
        SPDR = c;
        b = b >> 2;
        while (!(SPSR & _BV(SPIF)));
      }
      i += WIDTH;
    }
    i -= HEIGHT / 8 * WIDTH  + 1;
  }
#else 
  //OLED SSD1306 and compatibles

  for (int i = 0; i < (HEIGHT*WIDTH)/8; i++)
  {
    SPItransfer(pgm_read_byte(image + i));

  }
#endif
}

// paint from a memory buffer, this should be FAST as it's likely what
// will be used by any buffer based subclass
void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{

#if defined(GU12864_800B) 
  displayEnable();
  for (uint8_t r = 0; r < (HEIGHT/8); r++)
  {
    LCDCommandMode();
    displayWrite(0x60);
    displayWrite(r);
    LCDDataMode();
    for (uint8_t c = 0; c < (WIDTH); c++)
    {
      bitClear(CS_PORT,CS_BIT);
      if (clear)
      {
        SPDR = *image; // set the first SPI data byte to get things started
        *(image++) = 0;  // clear the first image byte
      }
      else
        SPDR = *(image++);
      while (!(SPSR & _BV(SPIF)));
      bitSet(CS_PORT,CS_BIT);
    }
  }
  displayDisable();
#elif (defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX)) && !defined(BEARBOARD)
  uint16_t length = WIDTH * HEIGHT / 8;
  uint8_t sda_clr = I2C_PORT & ~((1 << I2C_SDA) | (1 << I2C_SCL));
  uint8_t scl = 1 << I2C_SCL;
  uint8_t sda = 1 << I2C_SDA;
  uint8_t scl_bit = I2C_SCL;
  i2c_start(SSD1306_I2C_DATA);
 #if defined (OLED_SSD1306_I2C)
  //bitbanging I2C ~2Mbps (8 cycles per bit / 78 cycles per byte)
  asm volatile (    
    "    dec  %[clear]          \n" //  get clear mask 0:0xFF, 1:0x00
    "    ld   r24, %a[ptr]      \n" // fetch display byte from buffer
    "1:                         \n"
    "    mov  r0, r24           \n" // move to shift register
    "    and  r24, %[clear]     \n" // apply clear mask
    "    st   %a[ptr]+, r24     \n" // update buffer
    "                           \n" 
    "    sec                    \n" // set carry for 8 shift counts
    "    rol  r0                \n" // shift a bit out and count at the same time
    "2:                         \n"
    "    out  %[port], %[sda0]  \n" // preemtively clear SDA
    "    brcc 3f                \n" // skip if dealing with 0 bit
    "    out  %[pin], %[sda]    \n" // toggle SDA on
    "3:                         \n" 
    "    out  %[pin], %[scl]    \n" // toggle SCL on
    "    lsl  r0                \n" // next bit to carry (moved here for 1 extra cycle delay)
    "    out  %[pin], %[scl]    \n" // toggle SCL off
    "    brne 2b                \n" // initial set carry will be shifted out after 8 loops setting Z flag
    "                           \n" 
    "    out  %[port], %[sda0]  \n" // clear SDA for ACK
    "    subi %A[len], 1        \n" // len-- part1 (moved here for 1 cycle delay)
    "    ld   r24, %a[ptr]      \n" // fetch display byte from buffer (and delay)
    "    out  %[pin], %[scl]    \n" // set SCL (2 cycles required)
    "    sbci %B[len], 0        \n" // len-- part2 (moved here for 1 cycle delay)
    "    out  %[pin], %[scl]    \n" // clear SCL (2 cycles required)
    "    brne 1b                \n"
    :[ptr]   "+e" (image),
     [len]   "+d" (length),
     [clear] "+r" (clear)
    :[port]  "i" (_SFR_IO_ADDR(I2C_PORT)),
     [pin]   "i" (_SFR_IO_ADDR(I2C_PIN)),
     [sda0]  "r" (sda_clr),
     [scl]   "r" (scl),
     [sda]   "r" (sda)
    :"r24"
  );
 #else
  //bitbanging I2C @ 2.66Mbps (6 cycles per bit / 56 cycles per byte)
  asm volatile (    
    "    dec  %[clear]          \n" //  get clear mask 0:0xFF, 1:0x00
    "    ld   r0, %a[ptr]       \n" // fetch display byte from buffer
    "1:                         \n"
    "    sbrc r0, 7             \n" // MSB first comes first
    "    out  %[pin], %[sda]    \n" // toggle SDA on for 1-bit
    "    out  %[pin], %[scl]    \n" // toggle SCL high
    "    mov  r24, r0           \n" // duplicate byte (also serves as extra clock cycle delay)
    "    out  %[pin], %[scl]    \n" // toggle SCL low
    "    out  %[port], %[sda0]  \n" // preemptively clear SDA for next bit
    "                           \n"    
    "    sbrc r0, 6             \n" // repeat of above but for bit 6
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" //    
    "    and  r24, %[clear]     \n" // apply clear mask (also serves as extra clock cycle delay)
    "    out  %[pin], %[scl]    \n" //    
    "    out  %[port], %[sda0]  \n" //    
    
    "    sbrc r0, 5             \n" // 
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" //    
    "    st   %a[ptr]+, r24     \n" // new buffer contents (also serves as extra clock cycle delay)
    "    out  %[pin], %[scl]    \n" //    
    "    out  %[port], %[sda0]  \n" //    

    "    sbrc r0, 4             \n" // 
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" // 
    "    cbi  %[port], %[sclb]  \n" // using cbi for extra extra clock cycle delay
    "    out  %[port], %[sda0]  \n" // 

    "    sbrc r0, 3             \n" // 
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" // 
    "    cbi  %[port], %[sclb]  \n" // using cbi for extra extra clock cycle delay
    "    out  %[port], %[sda0]  \n" // 
    
    "    sbrc r0, 2             \n" // 
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" // 
    "    cbi  %[port], %[sclb]  \n" // using cbi for extra extra clock cycle delay
    "    out  %[port], %[sda0]  \n" // 
    
    "    sbrc r0, 1             \n" // 
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" // 
    "    cbi  %[port], %[sclb]  \n" // using cbi for extra extra clock cycle delay
    "    out  %[port], %[sda0]  \n" // 
    
    "    sbrc r0, 0             \n" // 
    "    out  %[pin], %[sda]    \n" //    
    "    out  %[pin], %[scl]    \n" //    
    "    subi %A[len], 1        \n" // length-- part 1 (also serves as extra clock cycle delay)
    "    out  %[pin], %[scl]    \n" //    
    "    out  %[port], %[sda0]  \n" // SDA low for ACK   
    
    "    sbci %B[len], 0        \n" // length-- part 2 (also serves as extra clock cycle delay)
    "    out  %[pin], %[scl]    \n" // // clock ACK bit
    "    ld   r0, %a[ptr]       \n" // fetch next buffer byte (also serves as clock delay)
    "    out  %[pin], %[scl]    \n" // 
    "    brne 1b                \n" // length != 0 do next byte
    :[ptr]   "+e" (image),
     [len]   "+d" (length),
     [clear] "+r" (clear)
    :[port]  "i" (_SFR_IO_ADDR(I2C_PORT)),
     [pin]   "i" (_SFR_IO_ADDR(I2C_PIN)),
     [sda0]  "r" (sda_clr),
     [scl]   "r" (scl),
     [sda]   "r" (sda),
     [sclb]  "i" (scl_bit)
    :"r24"
  );
 #endif
  i2c_stop();
#elif defined(BEARBOARD) 
	 #if  ( defined(OLED_SSD1306_I2C) && defined(OLED_SSD1306_SPI))
	  i2c_start(SSD1306_I2C_DATA);
	  if (clear)
	  {
		for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++){
			SPItransfer(*(image));			
			i2c_sendByte(*(image));
			*(image++) = 0;
		}
	  } else {
		for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
		{
			SPItransfer(*(image++));
			i2c_sendByte(*(image));
		}
	  }
	  i2c_stop();

		 
	 #elif ( defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) ) 
	  i2c_start(SSD1306_I2C_DATA);
	  if (clear)
	  {
		for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++){
			i2c_sendByte(*(image));
			*(image++) = 0;
		}
	  } else {
		for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
		{
			i2c_sendByte(*(image++));
		}
	  }
	  i2c_stop();
	 #elif defined(OLED_SSD1306_SPI) // OLED_SSD1306



	  if (clear)
	  {

		for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++){
			SPItransfer(*(image));
			*(image++) = 0;
		}

	  } else {
	  for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
		{
			SPItransfer(*(image++));
			//GPIO_SPI_transfer(*(image++));
		  }
	  }


	#elif defined(TFT_ST7735_BLK) || defined (TFT_IL9341)

  GPIO_1->CLEAR = (1 << CSOUT_BIT);	
  LCDCommandMode();
	SPItransfer(TFTCMD_CASET);
  LCDDataMode();
  SPItransfer(0x00);SPItransfer(0);SPItransfer((WIDTH*SCALE-1)>>8);SPItransfer(WIDTH*SCALE-1); 
  LCDCommandMode();
	SPItransfer(TFTCMD_RASET);
  LCDDataMode();
  SPItransfer(0x00);SPItransfer(0);SPItransfer((HEIGHT*SCALE-1)>>8);SPItransfer(HEIGHT*SCALE-1);
  LCDCommandMode();
	SPItransfer(TFTCMD_RAMWR);
  LCDDataMode();	



 
/*
   // Предварительно разворачиваем страницы
    for (int page = 0; page < 8; page++) {
        int base_idx = page * WIDTH;
        
        // Для каждого бита в странице (8 строк)
        for (int bit = 0; bit < 8; bit++) {
            // Выводим все пиксели текущей строки
            for (int x = 0; x < WIDTH; x++) {
                uint8_t byte = image[base_idx + x];
                uint16_t color = (byte & (1 << bit)) ? MONOCHROME_ON : MONOCHROME_OFF;
                SPItransfer(color >> 8);
                SPItransfer(color & 0xFF);
            }
        }
    }
  */  
  
  /*
    for (int page = 0; page < 8; page++) {
        int base_idx = page * WIDTH;
        
        for (int bit = 0; bit < 8; bit++) {
            for (int x = 0; x < WIDTH; x++) {
                uint8_t byte = image[base_idx + x];
                color_buffer[x] = ((byte & (1 << bit)) ? 0xFFFF : 0x0000) ;
            }
            spi_transfer_block((uint8_t*)(color_buffer), WIDTH*2);
        }
    }
*/

/*
      for (int page = 0; page < 8; page++) {
        int base_idx = page * WIDTH;
        
        for (int bit = 0; bit < 8; bit++) {
            for (int x = 0; x < WIDTH; x++) {
                uint8_t byte = image[base_idx + x];
                color_buffer[x] = ((byte & (1 << bit)) ? MONOCHROME_ON : MONOCHROME_OFF) ;
            }
            spi_transfer_block((uint8_t*)(color_buffer), WIDTH*2);
        }
    }
*/

	#if defined SCALED
	//scaleBuffer2x(image,dstBuffer);
		
		
	
//	memset(dstBuffer, 0, 4096);

// 2. Рисуем точки НАПРЯМУЮ в dstBuffer
// Формат: 16 страниц × 256 байт, 1 байт = 8 вертикальных пикселей

// Точка А: Страница 0, байт 0, бит 0 → строки 0-1, столбец 0-1
//dstBuffer[0 * 256 + 0] = 0x03;  // 0b00000011 = биты 0 и 1 установлены
//dstBuffer[0 * 256 + 1] = 0x03;  // Дублируем для горизонтального ×2

// Точка Б: Страница 4, байт 10, бит 0 → строки 64-65, столбец 80-81
//dstBuffer[4 * 256 + 10] = 0x03;
//dstBuffer[4 * 256 + 11] = 0x03;

// Точка В: Страница 8, байт 50, бит 0 → строки 128-129 (если экран позволяет)
//dstBuffer[8 * 256 + 50] = 0x03;
//dstBuffer[8 * 256 + 51] = 0x03;
	//Serial.println("тут2");
		
		
		
		
	#endif
		for (int page = 0; page < 8; page++) {
			int base_idx = page * WIDTH;
			
			#if defined SCALED
				scaleBuffer2x(image+base_idx,dstBuffer);
				//memset(dstBuffer,0xFF,256);
			#endif
			for (int bit = 0; bit < 8; bit++) {
				for (int x = 0; x < WIDTH*SCALE; x++) {
					#if defined SCALED
						uint8_t byte = dstBuffer[x];
					#else
						uint8_t byte = image[base_idx + x];
					#endif 
					color_buffer[x] = ((byte & (1 << bit)) ? MONOCHROME_ON : MONOCHROME_OFF) ;
				}
				#if defined SCALED
					//spi_transfer_block((uint8_t*)(color_buffer), WIDTH*2*SCALE);
					//spi_transfer_block((uint8_t*)(color_buffer), WIDTH*2*SCALE);
					spi_transfer_block((uint8_t*)(color_buffer), WIDTH*SCALE);
					spi_transfer_block((uint8_t*)(color_buffer), WIDTH*SCALE);
				#else
					//spi_transfer_block((uint8_t*)(color_buffer), WIDTH*2*SCALE);
					spi_transfer_block((uint8_t*)(color_buffer), WIDTH*SCALE);
				#endif
			}
		}


		if (clear) {
			memset(image, 0, WIDTH * 8); // 8 страниц по 128 байт = 1024 байта
		}

	GPIO_1->SET = (1 << CSOUT_BIT);			
	 #endif
	
#elif  defined (OLED_SH1106_I2C)

  for (int page = 0; page < HEIGHT/8; page++)
  {
    i2c_start(SSD1306_I2C_CMD);
    i2c_sendByte(OLED_SET_PAGE_ADDRESS + page); // set page
    i2c_sendByte(OLED_SET_COLUMN_ADDRESS_HI);
    i2c_stop();
    i2c_start(SSD1306_I2C_DATA);
    if (clear)
    {
      for (int i = 0; i < WIDTH; i++)
      {
        i2c_sendByte(*image);
        *(image++) = 0;
      }
    } else
    {
      for (int i = 0; i < WIDTH; i++)
        i2c_sendByte(*(image++));
    }
    i2c_stop();
  }

  
#elif (defined(OLED_SH1106) || defined(LCD_ST7565)) && !defined(BEARBOARD)
  //Assembly optimized page mode display code with clear support.
  //Each byte transfer takes 18 cycles
  asm volatile (
    "     ldi  r19, %[page_cmd]                     \n\t"
    "1:                                             \n\t"
    "     ldi  r18, %[col_cmd]        ;1            \n\t"
    "     ldi  r20, 6                 ;1            \n\t"
    "     cbi  %[dc_port], %[dc_bit]  ;2 cmd mode   \n\t"         
    "                                               \n\t"
    "     out  %[spdr], r19           ;1            \n\t"         
    "2:   dec  r20                    ;6*3-1 : 17   \n\t"         
    "     brne 2b                                   \n\t"         
    "     out  %[spdr], r18           ;1            \n\t"        
    
    "     ldi  r18, %[width]          ;1            \n\t"         
    "     inc  r18                    ;1            \n\t"              
    "     rjmp 5f                     ;2            \n\t"              
    "4:                                             \n\t"
    "     lpm  r20, Z                 ;3 delay      \n\t"
    "     ld   r20, Z                 ;2            \n\t"
    "     sbi  %[dc_port], %[dc_bit]  ;2 data mode  \n\t"
    "     out  %[spdr], r20           ;1            \n\t" 
    "     cpse %[clear], __zero_reg__ ;1/2          \n\t" 
    "     mov  r20, __zero_reg__      ;1            \n\t" 
    "     st   Z+, r20                ;2            \n\t"
    "5:                                             \n\t"
    "     lpm  r20, Z                 ;3 delay      \n\t"
    "     dec  r18                    ;1            \n\t"
    "     brne 4b                     ;1/2          \n\t"
    "     inc  r19                    ;1            \n\t"
    "     cpi  r19,%[page_end]        ;1            \n\t"
    "     brne 1b                     ;1/2          \n\t"
    "     lpm  r20, Z                 ;3 delay      \n\t"
    "     in    __tmp_reg__, %[spsr]                \n\t" //read SPSR to clear SPIF
    : [ptr]      "+&z" (image)
    : 
      [page_cmd] "M" (OLED_SET_PAGE_ADDRESS),
      [page_end] "M" (OLED_SET_PAGE_ADDRESS + (HEIGHT / 8)),
      [dc_port]  "I" (_SFR_IO_ADDR(DC_PORT)),
      [dc_bit]   "I" (DC_BIT),
      [spdr]     "I" (_SFR_IO_ADDR(SPDR)),
      [spsr]    "I"   (_SFR_IO_ADDR(SPSR)),
      [col_cmd]  "M" (OLED_SET_COLUMN_ADDRESS_HI),
      [width]    "M" (WIDTH),
      [clear]    "r" (clear)
    : "r18", "r19", "r20"
  );
#elif defined(OLED_96X96) || defined(OLED_128X96) || defined(OLED_128X128)|| defined(OLED_128X64_ON_96X96) || defined(OLED_128X64_ON_128X96) || defined(OLED_128X64_ON_128X128)|| defined(OLED_128X96_ON_128X128) || defined(OLED_96X96_ON_128X128)
  // 1 bit to 4-bit expander display code with clear support.
  // Each transfer takes 18 cycles with additional 4 cycles for a column change.
  asm volatile(
   #if defined(OLED_128X64_ON_96X96)
    "  adiw   r30, 16                           \n\t"          
   #endif
    "  ldi   r25, %[col]                        \n\t"          
    ".lcolumn:                                  \n\t"         
    "   ldi  r24, %[row]            ;1          \n\t"
    ".lrow:                                     \n\t"
    "   ldi  r21, 7                 ;1          \n\t"
    "   ld   r22, z                 ;2          \n\t"
    "   ldd  r23, z+1               ;2          \n\t"
    ".lshiftstart:                              \n\t"
    "   ldi  r20, 0xFF              ;1          \n\t"
    "   sbrs r22, 0                 ;1          \n\t"
    "   andi r20, 0x0f              ;1          \n\t"
    "   sbrs r23, 0                 ;1          \n\t"
    "   andi r20,0xf0               ;1          \n\t"
    "   out  %[spdr], r20           ;1          \n\t"
    "                                           \n\t"
    "   cp   %[clear], __zero_reg__ ;1          \n\t"
    "   brne .lclear1               ;1/2        \n\t"
    ".lshiftothers:                             \n\t"
    "   movw r18, %A[ptr]           ;1          \n\t"
    "   rjmp .+0                    ;2          \n\t"
    "   rjmp .lshiftnext            ;2          \n\t"
    ".lclear1:                                  \n\t"
    "   st   z, __zero_reg__        ;2          \n\t" 
    "   std  z+1, __zero_reg__      ;2          \n\t"
    ".lshiftnext:                               \n\t"
    "                                           \n\t"
    "   lsr  r22                    ;1          \n\t"
    "   lsr  r23                    ;1          \n\t"
    "                                           \n\t"
    "   ldi  r20, 0xFF              ;1          \n\t"
    "   sbrs r22, 0                 ;1/2        \n\t"
    "   andi r20, 0x0f              ;1          \n\t"
    "   sbrs r23, 0                 ;1/2        \n\t"
    "   andi r20,0xf0               ;1          \n\t"
    "                                           \n\t"
    "   subi r18, %[top_lsb]        ;1          \n\t" //image - (WIDTH * ((HEIGHT / 8) - 1) - 2)
    "   sbci r19, %[top_msb]        ;1          \n\t"
    "   subi r21, 1                 ;1          \n\t"
    "   out  %[spdr], r20           ;1          \n\t"
    "   brne .lshiftothers          ;1/2        \n\t"
    "                                           \n\t"
    "   nop                         ;1          \n\t"
    "   subi %A[ptr], %[width]      ;1          \n\t" //image + width (negated addition)
    "   sbci %B[ptr], -1            ;1          \n\t"
    "   subi r24, 1                 ;1          \n\t"
    "   brne .lrow                  ;1/2        \n\t"
    "                                           \n\t"
    "   movw %A[ptr], r18           ;1          \n\t"
    "   subi r25, 1                 ;1          \n\t"
    "   brne .lcolumn               ;1/2        \n\t"
    "   in    __tmp_reg__, %[spsr]              \n\t" //read SPSR to clear SPIF
    : [ptr]     "+&z" (image)
    : [spdr]    "I" (_SFR_IO_ADDR(SPDR)),
      [spsr]    "I"   (_SFR_IO_ADDR(SPSR)),
      [row]     "M" (HEIGHT / 8),
     #if defined(OLED_128X64_ON_96X96)
      [col]     "M" (96 / 2),
     #else
      [col]     "M" (WIDTH / 2),
     #endif
      [width]   "M" (256 - WIDTH),
      [top_lsb] "M" ((WIDTH * ((HEIGHT / 8) - 1) - 2) & 0xFF),
      [top_msb] "M" ((WIDTH * ((HEIGHT / 8) - 1) - 2) >> 8),
      [clear]   "r" (clear)
    : "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25"
  );
#elif defined(OLED_64X128_ON_128X128)
  uint16_t i = WIDTH-1;
  for (uint8_t col = 0; col < WIDTH ; col++)
  {
    for (uint8_t row = 0; row < HEIGHT / 8; row++)
    {
      uint8_t b = *(image + i);
      if (clear) *(image + i) = 0;
      for (uint8_t shift = 0; shift < 4; shift++)
      {
        uint8_t c = 0xFF;
        if ((b & _BV(0)) == 0) c &= 0x0F;
        if ((b & _BV(1)) == 0) c &= 0xF0;
        SPDR = c;
        b = b >> 2;
        while (!(SPSR & _BV(SPIF)));
      }
      i += WIDTH;
    }
    i -= HEIGHT / 8 * WIDTH  + 1;
  }
#else
  //OLED SSD1306 and compatibles
  //data only transfer with clear support at 18 cycles per transfer
  uint16_t count;

  asm volatile (
    "   ldi   %A[count], %[len_lsb]               \n\t" //for (len = WIDTH * HEIGHT / 8)
    "   ldi   %B[count], %[len_msb]               \n\t"
    "1: ld    __tmp_reg__, %a[ptr]      ;2        \n\t" //tmp = *(image)
    "   out   %[spdr], __tmp_reg__      ;1        \n\t" //SPDR = tmp
    "   cpse  %[clear], __zero_reg__    ;1/2      \n\t" //if (clear) tmp = 0;
    "   mov   __tmp_reg__, __zero_reg__ ;1        \n\t"
    "2: sbiw  %A[count], 1              ;2        \n\t" //len --
    "   sbrc  %A[count], 0              ;1/2      \n\t" //loop twice for cheap delay
    "   rjmp  2b                        ;2        \n\t"
    "   st    %a[ptr]+, __tmp_reg__     ;2        \n\t" //*(image++) = tmp
    "   brne  1b                        ;1/2 :18  \n\t" //len > 0
    "   in    __tmp_reg__, %[spsr]                \n\t" //read SPSR to clear SPIF
    : [ptr]     "+&e" (image),
      [count]   "=&w" (count)
    : [spdr]    "I"   (_SFR_IO_ADDR(SPDR)),
      [spsr]    "I"   (_SFR_IO_ADDR(SPSR)),
      [len_msb] "M"   (WIDTH * (HEIGHT / 8 * 2) >> 8),   // 8: pixels per byte
      [len_lsb] "M"   (WIDTH * (HEIGHT / 8 * 2) & 0xFF), // 2: for delay loop multiplier
      [clear]   "r"   (clear)
  );
  #endif  

}
#if 0
// For reference, this is the "closed loop" C++ version of paintScreen()
// used prior to the above version.
void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{
  uint8_t c;
  int i = 0;

  if (clear)
  {
    SPDR = image[i]; // set the first SPI data byte to get things started
    image[i++] = 0;  // clear the first image byte
  }
  else
    SPDR = image[i++];

  // the code to iterate the loop and get the next byte from the buffer is
  // executed while the previous byte is being sent out by the SPI controller
  while (i < (HEIGHT * WIDTH) / 8)
  {
    // get the next byte. It's put in a local variable so it can be sent as
    // as soon as possible after the sending of the previous byte has completed
    if (clear)
    {
      c = image[i];
      // clear the byte in the image buffer
      image[i++] = 0;
    }
    else
      c = image[i++];

    while (!(SPSR & _BV(SPIF))) { } // wait for the previous byte to be sent

    // put the next byte in the SPI data register. The SPI controller will
    // clock it out while the loop continues and gets the next byte ready
    SPDR = c;
  }
  while (!(SPSR & _BV(SPIF))) { } // wait for the last byte to be sent
}
#endif

void Arduboy2Core::blank()
{
#if defined(OLED_SSD1306_I2C) && defined(OLED_SSD1306_SPI)
  i2c_start(SSD1306_I2C_DATA);
  for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++) {
    SPItransfer(0x00);
	i2c_sendByte(0);
  }
  i2c_stop();
#elif defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX)
  i2c_start(SSD1306_I2C_DATA);
  for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
    i2c_sendByte(0);
  i2c_stop();
#elif  defined (OLED_SH1106_I2C)
  for (int page = 0; page < HEIGHT/8; page++)
  {
    i2c_start(SSD1306_I2C_CMD);
    i2c_sendByte(OLED_SET_PAGE_ADDRESS + page); // set page
    i2c_sendByte(OLED_SET_COLUMN_ADDRESS_HI);   // only reset hi nibble to zero
    i2c_stop();
    i2c_start(SSD1306_I2C_DATA);
    for (int i = 0; i < WIDTH; i++)
      i2c_sendByte(0);
    i2c_stop();
  }
#elif defined(TFT_ST7735_BLK) || defined(TFT_IL9341)

  GPIO_1->CLEAR = (1 << CSOUT_BIT);	
 LCDCommandMode();
	SPItransfer(TFTCMD_CASET);
  LCDDataMode();
  SPItransfer(0x00);SPItransfer(0);SPItransfer((WIDTH*SCALE-1)>>8);SPItransfer(WIDTH*SCALE-1); 
  LCDCommandMode();
	SPItransfer(TFTCMD_RASET);
  LCDDataMode();
  SPItransfer(0x00);SPItransfer(0);SPItransfer((HEIGHT*SCALE-1)>>8);SPItransfer(HEIGHT*SCALE-1);
  LCDCommandMode();
	SPItransfer(TFTCMD_RAMWR);
  LCDDataMode();
    // Отправка 8192 пикселей (черных)

	
	for (uint32_t i = 0; i < (WIDTH*SCALE+HEIGHT*SCALE); i++) {
        SPItransfer(0x00);
        SPItransfer(0x00);
    }



	GPIO_1->SET = (1 << CSOUT_BIT);
#else
 #if defined (OLED_SH1106)
  for (int i = 0; i < (HEIGHT * 132) / 8; i++)
 #elif defined(OLED_96X96) || defined(OLED_128X96) || defined(OLED_128X128)|| defined(OLED_128X64_ON_96X96) || defined(OLED_128X64_ON_128X96) || defined(OLED_128X64_ON_128X128)|| defined(OLED_128X96_ON_128X128) || defined(OLED_96X96_ON_128X128) || defined(OLED_64X128_ON_128X128)
  for (int i = 0; i < (HEIGHT * WIDTH) / 2; i++)
 #else //OLED SSD1306 and compatibles
  for (int i = 0; i < (HEIGHT * WIDTH) / 8; i++)
 #endif
    SPItransfer(0x00);
#endif
}

void Arduboy2Core::sendLCDCommand(uint8_t command)
{
#if defined(OLED_SSD1306_I2C) && defined(OLED_SSD1306_SPI) 
  LCDCommandMode();
  SPItransfer(command);
  LCDDataMode();
  i2c_start(SSD1306_I2C_CMD);
  i2c_sendByte(command);
  i2c_stop();
#elif defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
  i2c_start(SSD1306_I2C_CMD);
  i2c_sendByte(command);
  i2c_stop();
#elif defined (TFT_ST7735_BLK) || defined (TFT_IL9341)
	//LCDCommandMode();
	GPIO_1->CLEAR = (1 << CSOUT_BIT);
		LCDCommandMode();
    SPItransfer(command);
		LCDDataMode();
	GPIO_1->SET = (1 << CSOUT_BIT);
    //LCDDataMode();
#elif !defined GU12864_800B 
  LCDCommandMode();
  SPItransfer(command);
  LCDDataMode();
#endif
}

#if defined (TFT_ST7735_BLK) || defined (TFT_IL9341)


	#if defined SCALED
	void Arduboy2Core::scaleBuffer2x (const uint8_t* src, uint8_t* dst) {
			for (uint8_t x = 0; x < WIDTH; x++) {
			  dst[x * 2]     = src[x];
			  dst[x * 2 + 1] = src[x];
		}
	}
	#endif


void Arduboy2Core::sendTFTCommand(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes)
{
	GPIO_1->CLEAR = (1 << CSOUT_BIT);	
	LCDCommandMode();
    SPItransfer(commandByte); // Send the command byte
	LCDDataMode();
		  for (int i = 0; i < numDataBytes; i++) {
			   SPItransfer(*dataBytes); // Send the data bytes
			  dataBytes++;
			}
	GPIO_1->SET = (1 << CSOUT_BIT);

}

void Arduboy2Core::sendTFTCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes)
{
	GPIO_1->CLEAR = (1 << CSOUT_BIT);	
	LCDCommandMode();
    SPItransfer(commandByte); // Send the command byte
	LCDDataMode();
		  for (int i = 0; i < numDataBytes; i++) {
			   SPItransfer(*dataBytes); // Send the data bytes
			  dataBytes++;
			}
	GPIO_1->SET = (1 << CSOUT_BIT);

}

void Arduboy2Core::spi_transfer_block(uint8_t *data, int len) {
	uint8_t* end = data + len;
    while (data< end) {

	#if defined (TFT_ST7735_BLK) || defined (TFT_IL9341)
//uint8_t current_byte = *data++;

		SPI_0->TXDATA = 0;
		__NOP();
		__NOP();
		__NOP();
		(void) SPI_0->RXDATA;
		__NOP();
		__NOP();
		__NOP();
		SPI_0->TXDATA = *data++;
		__NOP();
		__NOP();
		__NOP();		
		(void) SPI_0->RXDATA;
		//while (!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M)); 
		//while (!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M)) {(void) SPI_0->RXDATA;};
	#else // такого варианта нет....
		while (!(SPI_0->INT_STATUS & SPI_INT_STATUS_RX_FIFO_NOT_EMPTY_M));
		(void) SPI_0->RXDATA;		
	#endif
    }
}

#endif






// invert the display or set to normal
// when inverted, a pixel set to 0 will be on
void Arduboy2Core::invert(bool inverse)
{
 #if defined(GU12864_800B)
  displayEnable();
  displayWrite(0x24);
  if (inverse) displayWrite(0x50);
  else displayWrite(0x40);
  LCDDataMode();
  displayDisable();
 #elif defined(TFT_ST7735_BLK) | defined (TFT_IL9341)
    sendLCDCommand(inverse ? TFTCMD_INVON : TFTCMD_INVOFF);
 #else
  sendLCDCommand(inverse ? OLED_PIXELS_INVERTED : OLED_PIXELS_NORMAL);
 #endif
}

// turn all display pixels on, ignoring buffer contents
// or set to normal buffer display
void Arduboy2Core::allPixelsOn(bool on)
{
 #if defined(GU12864_800B)
  displayEnable();
  if (on) 
  {
    displayWrite(0x20);
    displayWrite(0x50);
  }
  else 
    displayWrite(0x24);
    displayWrite(0x40);
  LCDDataMode();
  displayDisable();
 #else
  sendLCDCommand(on ? OLED_ALL_PIXELS_ON : OLED_PIXELS_FROM_RAM);
 #endif  
}

// flip the display vertically or set to normal
void Arduboy2Core::flipVertical(bool flipped)
{
 #ifdef GU12864_800B 
  //not available
 #else
  sendLCDCommand(flipped ? OLED_VERTICAL_FLIPPED : OLED_VERTICAL_NORMAL);
 #endif
}

// flip the display horizontally or set to normal
void Arduboy2Core::flipHorizontal(bool flipped)
{
 #ifdef GU12864_800B 
  //not available
 #else
  sendLCDCommand(flipped ? OLED_HORIZ_FLIPPED : OLED_HORIZ_NORMAL);
 #endif
}

/* RGB LED */

void Arduboy2Core::setRGBled(uint8_t red, uint8_t green, uint8_t blue)
{
#if defined (LCD_ST7565) || (MICROCADE)
  if ((red | green | blue) == 0) //prevent backlight off 
  {
    red   = 255;
    green = 255;
    blue  = 255;
  }
#endif


#ifndef BEARBOARD
	#if defined(ECONSOLE) 
  // only blue on DevKit, which is not PWM capable
  (void)red;    // parameter unused
  (void)green;  // parameter unused
   (void)blue;  // parameter unused
#elif ARDUBOY_10 // RGB, all the pretty colors
  // timer 0: Fast PWM, OC0A clear on compare / set at top
  // We must stay in Fast PWM mode because timer 0 is used for system timing.
  // We can't use "inverted" mode because it won't allow full shut off.
 #ifndef AB_ALTERNATE_WIRING
  TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
  #ifndef LCD_ST7565
   OCR0A = 255 - green;
  #else
   OCR0A = green;
  #endif
 #else
  TCCR0A = _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
  #ifndef LCD_ST7565
   OCR0B = 255 - green;
  #else
   OCR0B = green;
  #endif
 #endif
  // timer 1: Phase correct PWM 8 bit
  // OC1A and OC1B set on up-counting / clear on down-counting (inverted). This
  // allows the value to be directly loaded into the OCR with common anode LED.
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0) | _BV(WGM10);
 #ifndef LCD_ST7565
  OCR1AL = blue;
  OCR1BL = red;
 #else
  OCR1AL = 255 - blue;
  OCR1BL = 255 - red;
 #endif
#elif defined(AB_DEVKIT)
  // only blue on DevKit, which is not PWM capable
  (void)red;    // parameter unused
  (void)green;  // parameter unused
  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, blue ? RGB_ON : RGB_OFF);
#endif
#else

	PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_TIMER32_2_M | PM_CLOCK_APB_P_GPIO_1_M;
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;

	PAD_CONFIG->PORT_1_CFG |= (0b10 << (2 * RED_LED)) | (0b10 << (2 * GREEN_LED)) | (0b10 << (2 * BLUE_LED)) ; // установка вывода в режим 0xb10. Timer Connect!

	TIMER32_2->CHANNELS[0].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
	TIMER32_2->CHANNELS[2].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
	TIMER32_2->CHANNELS[1].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
	
	TIMER32_2->CHANNELS[0].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
	TIMER32_2->CHANNELS[2].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
	TIMER32_2->CHANNELS[1].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
	
	TIMER32_2->PRESCALER =  0; //Divide by 1 clock prescale
	TIMER32_2->INT_MASK =  0;
	TIMER32_2->INT_CLEAR =   0xFFFFFFFF;
	
	TIMER32_2->CHANNELS[0].OCR = 0;
	TIMER32_2->CHANNELS[2].OCR = 0;
	TIMER32_2->CHANNELS[1].OCR = 0;
	
	TIMER32_2->CHANNELS[0].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
	TIMER32_2->CHANNELS[2].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
	TIMER32_2->CHANNELS[1].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;

	TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_CLR_M | ~(TIMER32_ENABLE_TIM_EN_M); // без  этого таймер временно "зависает" при быстрой смене TOP/OCR
	TIMER32_2->TOP = (255); // максимальное значение 255
	TIMER32_2->CHANNELS[2].OCR = 255- red; // D13 - PORT_1_2 - Timer32_2_ch3 R
	TIMER32_2->CHANNELS[0].OCR = 255- green; // D12 - PORT_1_0 - Timer32_2_ch1 G
	TIMER32_2->CHANNELS[1].OCR = 255- blue; // D11 - PORT_1_1 - Timer32_2_ch2 B
	TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_CLR_M | TIMER32_ENABLE_TIM_EN_M;
#endif
}

void Arduboy2Core::setRGBled(uint8_t color, uint8_t val)
{
#ifndef BEARBOARD
	#if defined(ECONSOLE) 
	   //(void)blue;  // parameter unused
	#elif defined (ARDUBOY_10)
	  if (color == RED_LED)
	  {
	   #ifdef LCD_ST7565
		OCR1BL = 255 - val;
	   #else
		OCR1BL = val;
	   #endif
	  }
	  else if (color == GREEN_LED)
	  {
	   #ifndef AB_ALTERNATE_WIRING
		OCR0A = 255 - val;
	   #else
		#ifdef LCD_ST7565
		OCR0B = val;            
		#else
		OCR0B = 255 - val;            
		#endif
	   #endif
	  }
	  else if (color == BLUE_LED)
	  {
	   #ifdef LCD_ST7565
		OCR1AL = 255 - val;
	   #else
		OCR1AL = val;
	   #endif
	  }
	#elif defined(AB_DEVKIT)
	  // only blue on DevKit, which is not PWM capable
	  if (color == BLUE_LED)
	  {
		bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, val ? RGB_ON : RGB_OFF);
	  }
	#endif
#else
	  if (color == RED_LED)
	  {
		TIMER32_2->CHANNELS[2].OCR = 255- val;
	  }
	  else if (color == GREEN_LED)
	  {
		TIMER32_2->CHANNELS[0].OCR = 255- val;
	  }
	  else if (color == BLUE_LED)
	  {
		TIMER32_2->CHANNELS[1].OCR = 255- val;
	  }
#endif
}

void Arduboy2Core::freeRGBled()
{
#ifndef BEARBOARD
	#ifdef ARDUBOY_10
		  // clear the COM bits to return the pins to normal I/O mode
		  TCCR0A = _BV(WGM01) | _BV(WGM00);
		  TCCR1A = _BV(WGM10);
	#endif  
#else
	PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * RED_LED)); // установка вывода в режим 0xb00.  Timer Disconnect!
    PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * GREEN_LED)); // установка вывода в режим 0xb00.  Timer Disconnect!
	PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * BLUE_LED)); // установка вывода в режим 0xb00.  Timer Disconnect!
#endif
}

void Arduboy2Core::digitalWriteRGB(uint8_t red, uint8_t green, uint8_t blue)
{
#ifndef BEARBOARD
	#if defined (LCD_ST7565) || (MICROCADE)
	  if ((red & green & blue) == RGB_OFF) //prevent backlight off 
	  {
		red   = RGB_ON;
		green = RGB_ON;
		blue  = RGB_ON;
	  }
	  bitWrite(RED_LED_PORT, RED_LED_BIT, !red);
	  bitWrite(GREEN_LED_PORT, GREEN_LED_BIT, !green);
	  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, !blue);
	#else
	 #ifdef ARDUBOY_10
	  bitWrite(RED_LED_PORT, RED_LED_BIT, red);
	  bitWrite(GREEN_LED_PORT, GREEN_LED_BIT, green);
	  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, blue);
	 #elif defined(AB_DEVKIT)
	  // only blue on DevKit
	  (void)red;    // parameter unused
	  (void)green;  // parameter unused
	  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, blue);
	  #elif defined(ECONSOLE)
	  // only blue on DevKit, which is not PWM capable
	  (void)red;    // parameter unused
	  (void)green;  // parameter unused
	   (void)blue;  // parameter unused
	 #endif
	#endif
#else
	  if (red) {GPIO_1->SET = (1 << RED_LED_BIT);} else {GPIO_1->CLEAR = (1 << RED_LED_BIT); }
	  if (green) {GPIO_1->SET = (1 << GREEN_LED_BIT);} else {GPIO_1->CLEAR = (1 << GREEN_LED_BIT); }
	  if (blue) {GPIO_1->SET = (1 << BLUE_LED_BIT);} else {GPIO_1->CLEAR = (1 << BLUE_LED_BIT); }
#endif
}

void Arduboy2Core::digitalWriteRGB(uint8_t color, uint8_t val)
{
#ifndef BEARBOARD
	#ifdef ARDUBOY_10
	  if (color == RED_LED)
	  {
		bitWrite(RED_LED_PORT, RED_LED_BIT, val);
	  }
	  else if (color == GREEN_LED)
	  {
		bitWrite(GREEN_LED_PORT, GREEN_LED_BIT, val);
	  }
	  else if (color == BLUE_LED)
	  {
		bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, val);
	  }
	#elif defined(AB_DEVKIT)
	  // only blue on DevKit
	  if (color == BLUE_LED)
	  {
		bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, val);
	  }
	#endif
#else
	  if (color == RED_LED)
	  {
		if (val) {GPIO_1->SET = (1 << RED_LED_BIT);} else {GPIO_1->CLEAR = (1 << RED_LED_BIT); }
	  }
	  else if (color == GREEN_LED)
	  {
		if (val) {GPIO_1->SET = (1 << GREEN_LED_BIT);} else {GPIO_1->CLEAR = (1 << GREEN_LED_BIT); }
	  }
	  else if (color == BLUE_LED)
	  {
		if (val) {GPIO_1->SET = (1 << BLUE_LED_BIT);} else {GPIO_1->CLEAR = (1 << BLUE_LED_BIT); }
	  }
#endif
}

/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  uint8_t buttons;

#ifdef ARDUBOY_10
  #if defined (ECONSOLE)
	#ifndef JOYSTICKANALOG
	buttons = 0;
	if (bitRead(UP_BUTTON_PORTIN, UP_BUTTON_BIT) == 0) { buttons |= UP_BUTTON; }
	if (bitRead(DOWN_BUTTON_PORTIN, DOWN_BUTTON_BIT) == 0) { buttons |= DOWN_BUTTON; }
	if (bitRead(LEFT_BUTTON_PORTIN, LEFT_BUTTON_BIT) == 0) { buttons |= LEFT_BUTTON; }
	if (bitRead(RIGHT_BUTTON_PORTIN, RIGHT_BUTTON_BIT) == 0) { buttons |= RIGHT_BUTTON; }
	#else
    // JOYSTICKANALOG
	buttons = 0;  
    //buttons &= ~(A_BUTTON| B_BUTTON);

	if (ADCSRA & (1 << ADIF)) {
		unsigned int ADCdata=(ADCL|ADCH << 8);

    if ((ADMUX & 0b00001111) ==0 ) { // if the conversion at the AC0 input is complete
		ADCJoystickState &= ~(RIGHT_BUTTON | LEFT_BUTTON);
		if (JoystickXZero>1024) {JoystickXZero=ADCdata;} // if first run
		if (ADCdata > JoystickXZero+JOYSENSX) {ADCJoystickState |= RIGHT_BUTTON;} else if (ADCdata < JoystickXZero-JOYSENSX) {ADCJoystickState |= LEFT_BUTTON;} // we determine the direction along the X axis
		ADMUX = YAXIS_IN_ADMUX; // we will measure the signal at the AC1 input; REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=0, MUX2=0, MUX1=0, MUX0=1;  
		//ADMUX =  0b01000001;
		ADCSRA |= (1 << ADSC);  // start conversionе
	} else if ((ADMUX & 0b00001111) ==1)   // if the conversion at the AC1 input is complete 
		{ 
			ADCJoystickState &= ~(UP_BUTTON | DOWN_BUTTON);
			if (JoystickYZero>1024) {JoystickYZero=ADCdata;} // if first run
			if (ADCdata > JoystickYZero+JOYSENSY) {ADCJoystickState |= UP_BUTTON;} else if (ADCdata < JoystickYZero-JOYSENSY) {ADCJoystickState |= DOWN_BUTTON;} // we determine the direction along the Y axis
			ADMUX =  XAXIS_IN_ADMUX; // we will measure the signal at the AC0 input; REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=0, MUX2=0, MUX1=0, MUX0=0;  
			//ADMUX =  0b01000000;
			ADCSRA |= (1 << ADSC);  // start conversionе
		} else 
		{
			ADMUX =  XAXIS_IN_ADMUX; // снимать сигнал будем с входа AC0; REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=0, MUX2=0, MUX1=0, MUX0=0; 
			//ADMUX =  0b01000000;
			ADCSRA |= (1 << ADSC); 
		}
	} 
	buttons |= ADCJoystickState;
	#endif
	if (bitRead(A_BUTTON_PORTIN, A_BUTTON_BIT) == 0) { buttons |= A_BUTTON; }
	if (bitRead(B_BUTTON_PORTIN, B_BUTTON_BIT) == 0) { buttons |= B_BUTTON; }


  #elif defined(BEARBOARD)
	buttons = 0;
	#ifndef JOYSTICKANALOG
		#ifndef SPIBEAR
			if (bitRead(UP_BUTTON_PORTIN, UP_BUTTON_BIT) == 0) { buttons |= UP_BUTTON; }
			if (bitRead(DOWN_BUTTON_PORTIN, DOWN_BUTTON_BIT) == 0) { buttons |= DOWN_BUTTON; }
			if (bitRead(LEFT_BUTTON_PORTIN, LEFT_BUTTON_BIT) == 0) { buttons |= LEFT_BUTTON; }
			if (bitRead(RIGHT_BUTTON_PORTIN, RIGHT_BUTTON_BIT) == 0) { buttons |= RIGHT_BUTTON; }
		#else
			if (bitRead(UP_BUTTON_PORTIN, UP_BUTTON_BIT) == 1) { buttons |= UP_BUTTON; }
			if (bitRead(DOWN_BUTTON_PORTIN, DOWN_BUTTON_BIT) == 1) { buttons |= DOWN_BUTTON; }
			if (bitRead(LEFT_BUTTON_PORTIN, LEFT_BUTTON_BIT) == 1) { buttons |= LEFT_BUTTON; }
			if (bitRead(RIGHT_BUTTON_PORTIN, RIGHT_BUTTON_BIT) == 1) { buttons |= RIGHT_BUTTON; }		
		#endif
	#else
		if (ANALOG_REG->ADC_VALID) {
			if ((chan_converted == CHAN_AXISX | chan_converted==CHAN_AXISY ) & (chan_selected==CHAN_AXISY|chan_selected==CHAN_AXISX))  
	   
			{  // последний заданный канал и рассчитанный канал- один из наших
				unsigned int ADCdata=ANALOG_REG->ADC_VALUE; // данные от прошлого расчета, возможно на канале за пределами используемых
				if (chan_converted ==CHAN_AXISX ) { // if the conversion at the AC0 input is complete
					ADCJoystickState &= ~(RIGHT_BUTTON | LEFT_BUTTON);
					if (JoystickXZero>4096) {JoystickXZero=ADCdata;} // if first run
					if (ADCdata > JoystickXZero+JOYSENSX) {ADCJoystickState |= RIGHT_BUTTON;} else if (ADCdata < JoystickXZero-JOYSENSX) {ADCJoystickState |= LEFT_BUTTON;} // we determine the direction along the X axis
					chan_selected=CHAN_AXISX;
					chan_converted=CHAN_AXISY;
				} else if (chan_converted ==CHAN_AXISY)   // if the conversion at the AC1 input is complete 
				{ 
					ADCJoystickState &= ~(UP_BUTTON | DOWN_BUTTON);
					if (JoystickYZero>4096) {JoystickYZero=ADCdata;} // if first run
					if (ADCdata > JoystickYZero+JOYSENSY) {ADCJoystickState |= UP_BUTTON;} else if (ADCdata < JoystickYZero-JOYSENSY) {ADCJoystickState |= DOWN_BUTTON;} // we determine the direction along the Y axis
					chan_selected=CHAN_AXISY;
					chan_converted=CHAN_AXISX;
				} 
				myADC_SEL_CHANNEL (chan_selected); //можно попробовать исключить для ускорения
				ANALOG_REG->ADC_SINGLE=1;
				myADC_SEL_CHANNEL (chan_selected);
			} else // если не наш канал (канал рандомизатора )
			{

				chan_converted=chan_selected;
				chan_selected=CHAN_AXISY;
				myADC_SEL_CHANNEL (chan_selected); // необходимая строка. без нее переключение с канала рандомайзера происходит с задержкой на несколько циклов вычисления. Вероятно эта строка нужна при переключении одного GPIO на другой.
				ANALOG_REG->ADC_SINGLE=1;
				myADC_SEL_CHANNEL (chan_selected);

			}
			

		}
		buttons |= ADCJoystickState;
	#endif
		#ifndef SPIBEAR
			if (bitRead(A_BUTTON_PORTIN, A_BUTTON_BIT) == 0) { buttons |= A_BUTTON; }
			if (bitRead(B_BUTTON_PORTIN, B_BUTTON_BIT) == 0) { buttons |= B_BUTTON; }
		#else
			if (bitRead(A_BUTTON_PORTIN, A_BUTTON_BIT) == 1) { buttons |= A_BUTTON; }
			if (bitRead(B_BUTTON_PORTIN, B_BUTTON_BIT) == 1) { buttons |= B_BUTTON; }
		#endif
  #else
  // up, right, left, down
  buttons = ((~PINF) &
              (_BV(UP_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) |
               _BV(LEFT_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) |
              #ifdef SUPPORT_XY_BUTTONS
               _BV(Y_BUTTON_BIT) |
              #endif              
               0));
  // A
  if (bitRead(A_BUTTON_PORTIN, A_BUTTON_BIT) == 0) { buttons |= A_BUTTON; }
  // B
  if (bitRead(B_BUTTON_PORTIN, B_BUTTON_BIT) == 0) { buttons |= B_BUTTON; }
 #ifdef SUPPORT_XY_BUTTONS
  // Y 
  if (bitRead(X_BUTTON_PORTIN, X_BUTTON_BIT) == 0) { buttons |= X_BUTTON; }
 #endif
#endif
#elif defined(AB_DEVKIT)
  // down, left, up
  buttons = ((~PINB) &
              (_BV(DOWN_BUTTON_BIT) | _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT)));
  // right
  if (bitRead(RIGHT_BUTTON_PORTIN, RIGHT_BUTTON_BIT) == 0) { buttons |= RIGHT_BUTTON; }
  // A
  if (bitRead(A_BUTTON_PORTIN, A_BUTTON_BIT) == 0) { buttons |= A_BUTTON; }
  // B
  if (bitRead(B_BUTTON_PORTIN, B_BUTTON_BIT) == 0) { buttons |= B_BUTTON; }
#endif
//Serial.println(buttons,BIN);
  return buttons;
}

unsigned long Arduboy2Core::generateRandomSeed()
{
  unsigned long seed;

#ifndef BEARBOARD // classic or ECONSOLE
  
	#ifdef JOYSTICKANALOG

	  // ожидаем окончания счета
	  while ((ADCSRA >> ADSC) & 1);

		// измеряем внутренний аналоговый канал 1.1В на основе внутреннего опорного 1.1В
	  ADMUX = RAND_START_IN_ADMUX;
	  ADCSRA |= _BV(ADSC); // 
	  while ((ADCSRA >> ADSC) & 1);   // wait for conversion complete

		// переключаем опорное напряжение на внутренний источник 5В. Канал A2. Считается что переключение опорного напряжения вызывает большие погрешности при первых расчетах
	  ADMUX = REF_BACK_IN_ADMUX;
	  ADCSRA |= _BV(ADSC); 
	  while ((ADCSRA >> ADSC) & 1);  // wait for conversion complete
		seed = ((unsigned long)ADC << 16) + micros();
	
	  
	#else // стандартный расчет на основе RAND_SEED_IN_ADMUX
		
	  power_adc_enable(); // ADC on

	  // do an ADC read from an unconnected input pin
	  ADCSRA |= _BV(ADSC); // start conversion (ADMUX has been pre-set in boot())
	  while (bit_is_set(ADCSRA, ADSC)) { } // wait for conversion complete

	  seed = ((unsigned long)ADC << 16) + micros();
	  

	  power_adc_disable(); // ADC off
	  
	  
	#endif
#else
	//chan_selected=CHAN_RANDOM; // канал рандомизации
	
    myADC_SEL_CHANNEL(CHAN_RANDOM); //переключаемся на канал для рандомизации
	ANALOG_REG->ADC_SINGLE=1;
    myADC_SEL_CHANNEL(CHAN_RANDOM); //переключаемся на канал для рандомизации
	while (!ANALOG_REG->ADC_VALID) {};
    ANALOG_REG->ADC_SINGLE=1; //считаем новый рандом
	while (!ANALOG_REG->ADC_VALID) {};
	
	seed = ((unsigned long) ANALOG_REG->ADC_VALUE << 16) + micros();
	//seed = ((unsigned long) ANALOG_REG->ADC_VALUE << 16) ;
	
#endif
 
 return seed;
}

// delay in ms with 16 bit duration
void Arduboy2Core::delayShort(uint16_t ms)
{
 #ifndef ARDUBOY_CORE
  delay((unsigned long) ms);
 #else
  ::delayShort(ms);
 #endif
}

void Arduboy2Core::delayByte(uint8_t ms)
{
  delayShort(ms);
}

#ifdef BEARBOARD

/*
void inline Arduboy2Core::Delay_us (uint32_t us) //Функция задержки в микросекундах us
{

//        uint32_t i;
//      for (i=0;i<us;i++)
//      {
//       i++;
//       i--;
//      }

//    __NOP();
//    __NOP();
//    __NOP();
//    __NOP();
//    __NOP();
//    __NOP();
//    __NOP();


   //us *=(F_CPU/1000000000);
	while(us--){__NOP();};
   
}
*/
#define myEEPROM_TIMEOUT 100000 


#define myEEPROM_PAGE_WORDS 32      // words number per page
#define myEEPROM_PAGE_COUNT 8       // user EEPROM pages number
#define myEEPROM_START_ADDR 0x1C00  // user EEPROM start address
#define myEEPROM_WORD_SIZE  4       // word takes 4 bytes
#define myEEPROM_PAGE_SIZE  ( myEEPROM_PAGE_WORDS * myEEPROM_WORD_SIZE )   // page takes 32*4 = 128 bytes
#define myEEPROM_END        0x1FFF
#define myEEPROM_LENGHT     (myEEPROM_PAGE_SIZE * myEEPROM_PAGE_COUNT)

#define EEPROM_START_word_ADDR (myEEPROM_START_ADDR / myEEPROM_WORD_SIZE ) // 0x700

uint8_t Arduboy2Core::read_eeprom_byte(uint16_t idx) {
    if (idx >= myEEPROM_LENGHT)
    {
        //idx = idx % myEEPROM_LENGHT;   
		return 0x00;
    }

	// выбираем слово  //uint8_t word_idx = idx >> 2; // делим на 4
    EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx; // считаем, что биты 0,1 просто проигнорируются после чего в регистре останется адрес 4х байтового слова
	// ожидаем готовность
	uint32_t timeout=myEEPROM_TIMEOUT;
	while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
	// читаем
	uint32_t word_data = EEPROM_REGS->EEDAT;
	// выбираем байт
	uint8_t byte_offset = idx % myEEPROM_WORD_SIZE;
	// меняем порядок байт в слове
	uint32_t word_order= ((word_data & 0xFF)<<24) | ((word_data & (0xFF<<8))<<8) | ((word_data & (0xFF<<16))>>8) | ((word_data & (0xFF<<24))>>24);
	return (word_order >> (byte_offset * 8)) & 0xFF;
}
/*
void Arduboy2Core::update_eeprom_1st_page_byte(uint16_t idx, uint8_t val){
  
  if (idx<128) {
	// выбираем слово
    uint8_t word_idx = idx >> 2;  // делим на 4
    EEPROM_REGS->EEA  = (EEPROM_START_word_ADDR + word_idx)<<2;
	// ожидаем готовность
	uint32_t timeout=myEEPROM_TIMEOUT;
	while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
	// читаем существующее слово в обратном порядке
	uint32_t exist_order= EEPROM_REGS->EEDAT;
	//выбираем байт
	uint8_t byte_offset = idx % 4;
	// прямой порядок существующего слова
	uint32_t exist_redro= ((exist_order & 0xFF)<<24) | ((exist_order & (0xFF<<8))<<8) | ((exist_order & (0xFF<<16))>>8) | ((exist_order & (0xFF<<24))>>24);
	// добавляем наш байт в прямой порядок
	uint32_t word_data=exist_redro & (~(0xFF << (byte_offset*8))) |  ((uint32_t)val << (byte_offset * 8));
	// делаем обратный порядок измененного слова
	uint32_t word_order=((word_data & 0xFF)<<24) | ((word_data & (0xFF<<8))<<8) | ((word_data & (0xFF<<16))>>8) | ((word_data & (0xFF<<24))>>24);
		 
      //uint32_t a32=EEPROM_REGS->EEDAT;
      //uint8_t a = (uint8_t)a32;

      if (exist_order != word_order) {
        //erase
        
		EEPROM_REGS->EECON |= EEPROM_EECON_BWE_M;
		// При заполнении буфера записи адрес слова внутри буфера определяется разрядами EEA[6:2]
        EEPROM_REGS->EEA  = (EEPROM_START_word_ADDR + word_idx)<<2;
        EEPROM_REGS->EEDAT= exist_order;
        EEPROM_REGS->EECON |= EEPROM_EECON_OP(EEPROM_EECON_OP_ER) | EEPROM_EECON_EX_M;
		timeout=myEEPROM_TIMEOUT;		
		while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
        //update

        EEPROM_REGS->EECON |= EEPROM_EECON_BWE_M;
        EEPROM_REGS->EEA  = (EEPROM_START_word_ADDR + word_idx)<<2;
        EEPROM_REGS->EEDAT= word_order;
        EEPROM_REGS->EECON |= EEPROM_EECON_OP(EEPROM_EECON_OP_PR) | EEPROM_EECON_EX_M;
		timeout=myEEPROM_TIMEOUT;
		while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
      }
  } 
}
*/



void Arduboy2Core::update_eeprom_byte(uint16_t idx, uint8_t val){
  
    if (idx >= myEEPROM_LENGHT)
    {
        //idx = idx % myEEPROM_LENGHT;   
		return;
    }
	// выбираем слово
	EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx;
	// ожидаем готовность
	uint32_t timeout=myEEPROM_TIMEOUT;
	while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
	// читаем существующее слово в обратном порядке
	uint32_t exist_order= EEPROM_REGS->EEDAT;
	//выбираем байт
	uint8_t byte_offset = idx % myEEPROM_WORD_SIZE;
	// прямой порядок существующего слова
	uint32_t exist_redro= ((exist_order & 0xFF)<<24) | ((exist_order & (0xFF<<8))<<8) | ((exist_order & (0xFF<<16))>>8) | ((exist_order & (0xFF<<24))>>24);
	// добавляем наш байт в прямой порядок
	uint32_t word_data=exist_redro & (~(0xFF << (byte_offset*8))) |  ((uint32_t)val << (byte_offset * 8));
	// делаем обратный порядок измененного слова
	uint32_t word_order=((word_data & 0xFF)<<24) | ((word_data & (0xFF<<8))<<8) | ((word_data & (0xFF<<16))>>8) | ((word_data & (0xFF<<24))>>24);
		 
      //uint32_t a32=EEPROM_REGS->EEDAT;
      //uint8_t a = (uint8_t)a32;

      if (exist_order != word_order) {
        //erase
 		EEPROM_REGS->EECON |= EEPROM_EECON_BWE_M;
		// При заполнении буфера записи адрес слова внутри буфера определяется разрядами EEA[6:2]
        EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx;
        EEPROM_REGS->EEDAT= exist_order;
        EEPROM_REGS->EECON |= EEPROM_EECON_OP(EEPROM_EECON_OP_ER) | EEPROM_EECON_EX_M;
		timeout=myEEPROM_TIMEOUT;		
		while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
        //update

        EEPROM_REGS->EECON |= EEPROM_EECON_BWE_M;
        EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx;
        EEPROM_REGS->EEDAT= word_order;
        EEPROM_REGS->EECON |= EEPROM_EECON_OP(EEPROM_EECON_OP_PR) | EEPROM_EECON_EX_M;
		timeout=myEEPROM_TIMEOUT;
		while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
      }
  } 


uint8_t eeprom_read_byte(void *__src) {
    uint16_t idx = (uint16_t)( (uintptr_t)__src);
	if (idx >= myEEPROM_LENGHT)
    {
        //idx = idx % myEEPROM_LENGHT;   
		return 0x00;
    }

	// выбираем слово  //uint8_t word_idx = idx >> 2; // делим на 4
    EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx; // считаем, что биты 0,1 просто проигнорируются после чего в регистре останется адрес 4х байтового слова
	// ожидаем готовность
	uint32_t timeout=myEEPROM_TIMEOUT;
	while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
	// читаем
	uint32_t word_data = EEPROM_REGS->EEDAT;
	// выбираем байт
	uint8_t byte_offset = idx % myEEPROM_WORD_SIZE;
	// меняем порядок байт в слове
	uint32_t word_order= ((word_data & 0xFF)<<24) | ((word_data & (0xFF<<8))<<8) | ((word_data & (0xFF<<16))>>8) | ((word_data & (0xFF<<24))>>24);
	return (word_order >> (byte_offset * 8)) & 0xFF;
}


void eeprom_write_block(const void *__src, void *__dst, size_t __n)
{
    // Преобразуем адреса
    uint16_t dst_idx = (uint16_t) (uintptr_t)(__dst);
    const uint8_t *src_ptr = (const uint8_t *)__src;
    
    // Проверка границ
    if (dst_idx >= myEEPROM_LENGHT || dst_idx + __n > myEEPROM_LENGHT)
    {
        // Выход за пределы EEPROM - ничего не пишем
        return;
    }
    
    // Пишем побайтово, используя существующую функцию
    for (size_t i = 0; i < __n; i++)
    {
        eeprom_update_byte((void*) (uintptr_t)(dst_idx + i ),src_ptr[i]);
    }
}  

void eeprom_read_block(void *__dst, const void *__src, size_t __n)
{
    // Преобразуем адреса
    uint16_t src_idx = (uint16_t)(uintptr_t)(__src);
    uint8_t *dst_ptr = (uint8_t *)__dst;
    
    // Проверка границ
    if (src_idx >= myEEPROM_LENGHT || src_idx + __n > myEEPROM_LENGHT)
    {
        return;
    }
    
    // Читаем побайтово, используя существующую функцию
    for (size_t i = 0; i < __n; i++)
    {
        dst_ptr[i]=eeprom_read_byte((void*)(uintptr_t)(src_idx + i ));
    }
}
  
  
void eeprom_update_byte(void *__dst, uint8_t val){
	uint16_t idx = (uint16_t)(uintptr_t)(__dst);
    if (idx >= myEEPROM_LENGHT)
    {
        //idx = idx % myEEPROM_LENGHT;   
		return;
    }
	// выбираем слово
	EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx;
	// ожидаем готовность
	uint32_t timeout=myEEPROM_TIMEOUT;
	while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
	// читаем существующее слово в обратном порядке
	uint32_t exist_order= EEPROM_REGS->EEDAT;
	//выбираем байт
	uint8_t byte_offset = idx % myEEPROM_WORD_SIZE;
	// прямой порядок существующего слова
	uint32_t exist_redro= ((exist_order & 0xFF)<<24) | ((exist_order & (0xFF<<8))<<8) | ((exist_order & (0xFF<<16))>>8) | ((exist_order & (0xFF<<24))>>24);
	// добавляем наш байт в прямой порядок
	uint32_t word_data=exist_redro & (~(0xFF << (byte_offset*8))) |  ((uint32_t)val << (byte_offset * 8));
	// делаем обратный порядок измененного слова
	uint32_t word_order=((word_data & 0xFF)<<24) | ((word_data & (0xFF<<8))<<8) | ((word_data & (0xFF<<16))>>8) | ((word_data & (0xFF<<24))>>24);
		 
      //uint32_t a32=EEPROM_REGS->EEDAT;
      //uint8_t a = (uint8_t)a32;

      if (exist_order != word_order) {
        //erase
 		EEPROM_REGS->EECON |= EEPROM_EECON_BWE_M;
		// При заполнении буфера записи адрес слова внутри буфера определяется разрядами EEA[6:2]
        EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx;
        EEPROM_REGS->EEDAT= exist_order;
        EEPROM_REGS->EECON |= EEPROM_EECON_OP(EEPROM_EECON_OP_ER) | EEPROM_EECON_EX_M;
		timeout=myEEPROM_TIMEOUT;		
		while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
        //update

        EEPROM_REGS->EECON |= EEPROM_EECON_BWE_M;
        EEPROM_REGS->EEA  = myEEPROM_START_ADDR + idx;
        EEPROM_REGS->EEDAT= word_order;
        EEPROM_REGS->EECON |= EEPROM_EECON_OP(EEPROM_EECON_OP_PR) | EEPROM_EECON_EX_M;
		timeout=myEEPROM_TIMEOUT;
		while (timeout-- && (EEPROM_REGS->EESTA & EEPROM_EESTA_BSY_M));
      }
  }   
#endif

void Arduboy2Core::exitToBootloader()
{
#if !defined  (BEARBOARD) 
  cli();
 #ifdef ARDUBOY_CORE
  asm volatile 
  (
    "jmp exit_to_bootloader \n" // resuse ISR exit to bootloader code
  );
 #else
#if !defined  (ECONSOLE) 
// set bootloader magic key
  // storing two uint8_t instead of one uint16_t saves an instruction
  //  when high and low bytes of the magic key are the same
  *(uint8_t *)MAGIC_KEY_POS = lowByte(MAGIC_KEY);
  *(uint8_t *)(MAGIC_KEY_POS + 1) = highByte(MAGIC_KEY);
  // enable watchdog timer reset, with 16ms timeout
#endif
  wdt_reset();
  WDTCSR = (_BV(WDCE) | _BV(WDE));
  WDTCSR = _BV(WDE);
  while (true) { }
 #endif
#else
	//Serial.println("WDT");
	PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_WDT_M; //PM_CLOCK_WDT_M;
	PM->WDT_CLK_MUX = 0x01 & PM_WDT_CLK_MUX_M;  // 0x01 1 – внутренний HSI32M;
	//WDT->KEY=0x71;
	WDT->KEY = WDT_KEY_UNLOCK; // разблокировка  сторожевого таймера  0x1E
	WDT->CON = WDT_CON_PRESCALE_4096_M | WDT_CON_PRELOAD(0x000); // Делитель входной частоты на 4096   Начальное значение таймера при запуске или перезапуске (таймер считает в сторону увеличения значений)
	WDT->KEY = WDT_KEY_UNLOCK; // разблокировка  сторожевого таймера 0x1E
	WDT->KEY = WDT_KEY_START; // запуск сторожевого таймера 0x71
#endif
}

// Replacement main() that eliminates the USB stack code.
// Used by the ARDUBOY_NO_USB macro. This should not be called
// directly from a sketch.

//=========================================
//========== class Arduboy2NoUSB ==========
//=========================================

void Arduboy2NoUSB::mainNoUSB()
{
#if !defined  (ECONSOLE) && !defined  (BEARBOARD)
  // disable USB
  UDCON = _BV(DETACH);
  UDIEN = 0;
  UDINT = 0;
  USBCON = _BV(FRZCLK);
  UHWCON = 0;
  power_usb_disable();

  
  init();

  // This would normally be done in the USB code that uses the TX and RX LEDs
  //TX_RX_LED_INIT; // configured by bootpins

 #ifndef ARDUBOY_CORE // (Arduboy  core supports UP + DOWN to enter bootloader)
  // Set the DOWN button pin for INPUT_PULLUP
  bitSet(DOWN_BUTTON_PORT, DOWN_BUTTON_BIT);
  bitClear(DOWN_BUTTON_DDR, DOWN_BUTTON_BIT);

  // Delay to give time for the pin to be pulled high if it was floating
  Arduboy2Core::delayByte(10);

  // if the DOWN button is pressed
  if (bitRead(DOWN_BUTTON_PORTIN, DOWN_BUTTON_BIT) == 0) {
    Arduboy2Core::exitToBootloader();
  }
 #endif
  // The remainder is a copy of the Arduino main() function with the
  // USB code and other unneeded code commented out.
  // init() was called above.
  // The call to function initVariant() is commented out to fix compiler
  // error: "multiple definition of 'main'".
  // The return statement is removed since this function is type void.

//  init();

//  initVariant();

//#if defined(USBCON)
//  USBDevice.attach();
//#endif
#endif
  setup();

  for (;;) {
    loop();
//    if (serialEventRun) serialEventRun();
  }

//  return 0;
}

