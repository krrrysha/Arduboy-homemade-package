/**
 * @file Arduboy2Core.h
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#ifndef ARDUBOY2_CORE_H
#define ARDUBOY2_CORE_H

#ifdef MCU_MIK32_Amur
	#define OLED_SSD1306_I2C // define OLED_SSD1306_I2C or define OLED_SH1106_I2C 
	//define OLED_SH1106_I2C
	//#define JOYSTICKANALOG // undef or JOYSTICKANALOG or JOYSTICKDISCRETE.  JOYSTICKANALOG - when using the Joystick Shield analog stick; 
	//#define JOYSTICKDISCRETE
	#define ELBEARBOY
	#warning ELBEARBOY!
#endif

#ifdef __AVR_ATmega328P__
	#define OLED_SSD1306_I2C
	//#define OLED_SH1106_I2C
	//#define JOYSTICKDISCRETE // undef or JOYSTICKANALOG or JOYSTICKDISCRETE  JOYSTICKANALOG - when using the Joystick Shield analog stick; 
	//#define JOYSTICKANALOG
	#define ECONSOLE
	#warning ECONSOLE!
#endif


#include <Arduino.h>
#if !defined(ELBEARBOY)
	#include <avr/power.h>
	#include <avr/sleep.h>
#else
	#include "analog_reg.h"
	//#include <EEPROM.h>
	#include <eeprom_def.h>
#endif

#ifdef JOYSTICKANALOG
	#if !defined(ELBEARBOY) // ECONSOLE:
		#define JOYSENSX 200 //Joystick sensitivity. X axis
		#define JOYSENSY 200 //Joystick sensitivity. Y axis
	#else // ELBEARBOY:
		#define JOYSENSX 1000 //Joystick sensitivity. X axis
		#define JOYSENSY 1000 //Joystick sensitivity. Y axis
	#endif
#endif

// main hardware compile flags

#if !defined(ARDUBOY_10) && !defined(AB_DEVKIT)
/* defaults to Arduboy Release 1.0 if not using a boards.txt file
 *
 * we default to Arduboy Release 1.0 if a compile flag has not been
 * passed to us from a boards.txt file
 *
 * if you wish to compile for the devkit without using a boards.txt
 * file simply comment out the ARDUBOY_10 define and uncomment
 * the AB_DEVKIT define like this:
 *
 *     // #define ARDUBOY_10
 *     #define AB_DEVKIT
 */
#define ARDUBOY_10   //< compile for the production Arduboy v1.0
// #define AB_DEVKIT    //< compile for the official dev kit
#endif

#define RGB_ON LOW   /**< For digitially setting an RGB LED on using digitalWriteRGB() */
#define RGB_OFF HIGH /**< For digitially setting an RGB LED off using digitalWriteRGB() */

// ----- Arduboy pins -----
#ifdef ARDUBOY_10

#ifndef ELBEARBOY 
	#if defined(ECONSOLE) 
		#define PIN_CS 12       // Display CS Arduino pin number
		#define CS_PORT PORTD   // Display CS port
		#define CS_BIT PORTD6   // Display CS physical bit number

		#define PIN_DC 4        // Display D/C Arduino pin number
		#define DC_PORT PORTD   // Display D/C port
		#define DC_BIT PORTD4   // Display D/C physical bit number

		#define PIN_RST 6       // Display reset Arduino pin number
		#define RST_PORT PORTD  // Display reset port
		#define RST_BIT PORTD7  // Display reset physical bit number
	#elif defined (AB_ALTERNATE_WIRING)
		  #define PIN_CS 1        // Pro Micro alternative display CS pin (pin 12 not not available)
		  #define CS_PORT PORTD
		  #define CS_BIT PORTD3

		  #define PIN_RST 2       // Pro Micro alternative display RST pin (pin 6 favoured for 2nd speaker pin)
		  #define RST_PORT PORTD
		  #define RST_BIT PORTD1
	#else
		  #define PIN_CS 12       // Display CS Arduino pin number
		  #define CS_PORT PORTD   // Display CS port
		  #define CS_BIT PORTD6   // Display CS physical bit number

		  #define PIN_RST 6       // Display reset Arduino pin number
		  #define RST_PORT PORTD  // Display reset port
		  #define RST_BIT PORTD7  // Display reset physical bit number
	#endif

 #define PIN_DC 4        // Display D/C Arduino pin number
 #define DC_PORT PORTD   // Display D/C port
 #define DC_BIT PORTD4   // Display D/C physical bit number

#ifdef CART_CS_SDA
 #ifdef AB_ALTERNATE_WIRING
  #error SDA can not be used as flash chip select when using Pro Micro alternate wiring. Use RX instead.
 #endif
 #define PIN_CART 2        // SDA as alternative flash cart chip select
 #define CART_BIT PORTD1
#else
 #define PIN_CART 0        // RX as default flash cart chip select
 #define CART_BIT PORTD2
#endif
#define CART_PORT PORTD

#define SPI_MOSI_PORT PORTB
#define SPI_MOSI_BIT PORTB2

#define SPI_SCK_PORT PORTB
#define SPI_SCK_BIT PORTB1

#endif // end !ELBEARBOY



#if defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
	#if defined (ECONSOLE)
		#warning ECONSOLE!!!!!
		#define I2C_PORT  PORTC
		#define I2C_DDR   DDRC
		#define I2C_PIN   PINC
		#define I2C_SCL PORTC5
		#define I2C_SDA PORTC4
		//port states
		#define I2C_SDA_HIGH() I2C_PORT |=  (1 << I2C_SDA)
		#define I2C_SCL_HIGH() I2C_PORT |=  (1 << I2C_SCL)
		#define I2C_SDA_LOW()  I2C_PORT &= ~(1 << I2C_SDA)
		#define I2C_SCL_LOW()  I2C_PORT &= ~(1 << I2C_SCL)

		//port directions
		#define I2C_SDA_AS_INPUT()  I2C_DDR &= ~(1 << I2C_SDA)
		#define I2C_SCL_AS_INPUT()  I2C_DDR &= ~(1 << I2C_SCL)
		#define I2C_SDA_AS_OUTPUT() I2C_DDR |= (1 << I2C_SDA)
		#define I2C_SCL_AS_OUTPUT() I2C_DDR |= (1 << I2C_SCL)

		// display address, commands
		#define SSD1306_I2C_ADDR 0x3c //0x3c:default, 0x3d: alternative)
		#define SSD1306_I2C_CMD  0x00
		#define SSD1306_I2C_DATA 0x40
	#elif defined (ELBEARBOY)
			#define I2C_PORT  GPIO_1->STATE
			#define I2C_SCL 13 // D19  PORT 1.13
			#define I2C_SDA 12 // D18  PORT 1.12 
			
			#define SELA_A_PIN    15 // ANAMUX: SELA_A PORT 1.15
			#define SELA_B_PIN    11 // ANAMUX: SELA_B PORT 1.11

			//port states
			// INIT I2C as serial: PAD_CONFIG->PORT_1_CFG |= (0b01 << (2 * I2C_SCL));
			//						PAD_CONFIG->PORT_1_CFG |= (0b01 << (2 * I2C_SDA));
			//#define I2C_SDA_HIGH()  GPIO_1->SET =   (1 << I2C_SDA)
			//#define I2C_SCL_HIGH()  GPIO_1->SET =   (1 << I2C_SCL)
			
			//port directions
			#define I2C_SDA_AS_INPUT()  GPIO_1->DIRECTION_IN =  (1 << I2C_SDA)	// установка SDA в 1
			#define I2C_SCL_AS_INPUT()  GPIO_1->DIRECTION_IN =  (1 << I2C_SCL) // установка SCL в 1

			// установка SDA в 0
			#define I2C_SDA_AS_OUTPUT() GPIO_1->DIRECTION_OUT = (1 << I2C_SDA) 
			#define I2C_SDA_LOW()   GPIO_1->CLEAR = (1 << I2C_SDA) 

			// установка SCL в 0
			#define I2C_SCL_AS_OUTPUT()  GPIO_1->DIRECTION_OUT = (1 << I2C_SCL) 
			#define I2C_SCL_LOW()    GPIO_1->CLEAR = (1 << I2C_SCL) 
		
			#define SSD1306_I2C_ADDR 0x3c //0x3c:default, 0x3d: alternative)
			#define SSD1306_I2C_CMD  0x00
			#define SSD1306_I2C_DATA 0x40
	#else
		 #define I2C_PORT  PORTD
		 #define I2C_DDR   DDRD
		 #define I2C_PIN   PIND
		 #ifdef AB_ALTERNATE_WIRING
			#define I2C_SCL PORTD3
		 #else
			#define I2C_SCL PORTD7
		 #endif
		 #define I2C_SDA PORTD4
		 //port states
		 #define I2C_SDA_HIGH() I2C_PORT |=  (1 << I2C_SDA)
		 #define I2C_SCL_HIGH() I2C_PORT |=  (1 << I2C_SCL)
		 #define I2C_SDA_LOW()  I2C_PORT &= ~(1 << I2C_SDA)
		 #define I2C_SCL_LOW()  I2C_PORT &= ~(1 << I2C_SCL)

		 //port directions
		 #define I2C_SDA_AS_INPUT()  I2C_DDR &= ~(1 << I2C_SDA)
		 #define I2C_SCL_AS_INPUT()  I2C_DDR &= ~(1 << I2C_SCL)
		 #define I2C_SDA_AS_OUTPUT() I2C_DDR |= (1 << I2C_SDA)
		 #define I2C_SCL_AS_OUTPUT() I2C_DDR |= (1 << I2C_SCL)

		 // display address, commands
		 #define SSD1306_I2C_ADDR 0x3c //0x3c:default, 0x3d: alternative)
		 #define SSD1306_I2C_CMD  0x00
		 #define SSD1306_I2C_DATA 0x40
	#endif
#endif
#ifndef ELBEARBOY
	#if defined (ECONSOLE)
		#define RED_LED 13   /**< The pin number for the red color in the RGB LED. */
		#define GREEN_LED 17 /**< The pin number for the greem color in the RGB LED. */
		#define BLUE_LED 16   /**< The pin number for the blue color in the RGB LED. */

		#define RED_LED_PORT PORTB  
		#define RED_LED_BIT PORTB5

		#define GREEN_LED_PORT PORTC
		#define GREEN_LED_BIT PORTC3

		#define BLUE_LED_PORT PORTC
		#define BLUE_LED_BIT PORTC2

	#else
		#define RED_LED 10   /**< The pin number for the red color in the RGB LED. */
			#ifdef AB_ALTERNATE_WIRING
			  #define GREEN_LED 3 // Pro Micro alternative green LED pin
			#else
			  #define GREEN_LED 11 /**< The pin number for the green color in the RGB LED. */
			#endif
		#define BLUE_LED 9   /**< The pin number for the blue color in the RGB LED. */

		#define RED_LED_PORT PORTB
		#define RED_LED_BIT PORTB6

			#ifdef AB_ALTERNATE_WIRING
			  #define GREEN_LED_PORT PORTD // Pro Micro alternative green LED port
			  #define GREEN_LED_BIT PORTD0
			#else
			  #define GREEN_LED_PORT PORTB
			  #define GREEN_LED_BIT PORTB7
			#endif

		#define BLUE_LED_PORT PORTB
		#define BLUE_LED_BIT PORTB5
	#endif

	#define TX_LED_PORT PORTD 
	#define TX_LED_BIT PORTD5 // D5

	#define RX_LED_PORT PORTB
	#define RX_LED_BIT PORTB0 // D8

	#ifdef LCD_ST7565
	  #define POWER_LED_PORT PORTD
	  #define POWER_LED_BIT PORTD0
	#endif
#else // ELBEARBOY

	#if defined (JOYSTICKDISCRETE) || defined (JOYSTICKANALOG)
		#define RED_LED 2  // D13 port 1.2
		#define GREEN_LED 2 // D17 port 0.2
		#define BLUE_LED 4  // D16 port 0.4

		#define RED_LED_PORT GPIO_1->STATE
		#define RED_LED_BIT  2 //

		#define GREEN_LED_PORT GPIO_0->STATE
		#define GREEN_LED_BIT 2

		#define BLUE_LED_PORT GPIO_0->STATE
		#define BLUE_LED_BIT 4
	#else // ECONSOLE KEYS
		#define RED_LED 2  // D13 port 1.2
		#define GREEN_LED 9 // D8/9 port_1_9
		#define BLUE_LED 4  // D16 port 0.4

		#define RED_LED_PORT GPIO_1->STATE
		#define RED_LED_BIT  2 

		#define GREEN_LED_PORT GPIO_1->STATE
		#define GREEN_LED_BIT 9

		#define BLUE_LED_PORT GPIO_0->STATE
		#define BLUE_LED_BIT 4
	#endif

	#define TX_LED_PORT PORTD
	#define TX_LED_BIT PORTD5 // D5

	#define RX_LED_PORT PORTB
	#define RX_LED_BIT PORTB0 //d8
#endif
// bit values for button states
// these are determined by the buttonsState() function
#if defined (MICROCADE)
#define LEFT_BUTTON _BV(6)  // joystick is rotated 180 degrees
#define RIGHT_BUTTON _BV(5)
#define UP_BUTTON _BV(4)   
#define DOWN_BUTTON _BV(7) 
#else
#define LEFT_BUTTON _BV(5)  /**< The Left button value for functions requiring a bitmask */
#define RIGHT_BUTTON _BV(6) /**< The Right button value for functions requiring a bitmask */
#define UP_BUTTON _BV(7)    /**< The Up button value for functions requiring a bitmask */
#define DOWN_BUTTON _BV(4)  /**< The Down button value for functions requiring a bitmask */
#endif
#define A_BUTTON _BV(3)     /**< The A button value for functions requiring a bitmask */
#define B_BUTTON _BV(2)     /**< The B button value for functions requiring a bitmask */
#ifdef SUPPORT_XY_BUTTONS
 #define X_BUTTON _BV(0)
 #define Y_BUTTON _BV(1)
#endif

#if defined (ECONSOLE)
	#if defined (JOYSTICKANALOG)
		#define PIN_Y_AXIS A1 //3v3 341..342=0, 0..200=down, 400..674=up; 5V: 518..519=0, 0..200=down, 600..1023=up
		#define X_AXIS_PORT PORTC
		#define X_AXIS_PORTIN PINC
		#define X_AXIS_DDR DDRC
		#define X_AXIS_BIT PORTC0
		
		#define PIN_X_AXIS A0  //3v3 340..341=0, 0..124=left, 540..674=right; 5V 530=0, 0..200=left, 900..1023=rigth
		#define Y_AXIS_PORT PORTC
		#define Y_AXIS_PORTIN PINC
		#define Y_AXIS_DDR DDRC
		#define Y_AXIS_BIT PORTC1

		#define PIN_A_BUTTON 4
		#define A_BUTTON_PORT PORTD
		#define A_BUTTON_PORTIN PIND
		#define A_BUTTON_DDR DDRD
		#define A_BUTTON_BIT PORTD4

		#define PIN_B_BUTTON 3
		#define B_BUTTON_PORT PORTD
		#define B_BUTTON_PORTIN PIND
		#define B_BUTTON_DDR DDRD
		#define B_BUTTON_BIT PORTD3
	#elif defined (JOYSTICKDISCRETE) 
		// JOYSTICKDISCRETE KEYS 
		#define PIN_LEFT_BUTTON 5
		#define LEFT_BUTTON_PORT PORTD
		#define LEFT_BUTTON_PORTIN PIND
		#define LEFT_BUTTON_DDR DDRD
		#define LEFT_BUTTON_BIT PORTD5

		#define PIN_RIGHT_BUTTON 3
		#define RIGHT_BUTTON_PORT PORTD
		#define RIGHT_BUTTON_PORTIN PIND
		#define RIGHT_BUTTON_DDR DDRD
		#define RIGHT_BUTTON_BIT PORTD3

		#define PIN_UP_BUTTON 2
		#define UP_BUTTON_PORT PORTD
		#define UP_BUTTON_PORTIN PIND
		#define UP_BUTTON_DDR DDRD
		#define UP_BUTTON_BIT PORTD2

		#define PIN_DOWN_BUTTON 4
		#define DOWN_BUTTON_PORT PORTD
		#define DOWN_BUTTON_PORTIN PIND
		#define DOWN_BUTTON_DDR DDRD
		#define DOWN_BUTTON_BIT PORTD4

		#define PIN_A_BUTTON 8
		#define A_BUTTON_PORT PORTB
		#define A_BUTTON_PORTIN PINB
		#define A_BUTTON_DDR DDRB
		#define A_BUTTON_BIT PORTB0

		#define PIN_B_BUTTON 7
		#define B_BUTTON_PORT PORTD
		#define B_BUTTON_PORTIN PIND
		#define B_BUTTON_DDR DDRD
		#define B_BUTTON_BIT PORTD7
	#else // ECONSOLE KEYS
		#define PIN_LEFT_BUTTON 2
		#define LEFT_BUTTON_PORT PORTD
		#define LEFT_BUTTON_PORTIN PIND
		#define LEFT_BUTTON_DDR DDRD
		#define LEFT_BUTTON_BIT PORTD2

		#define PIN_RIGHT_BUTTON 6
		#define RIGHT_BUTTON_PORT PORTD
		#define RIGHT_BUTTON_PORTIN PIND
		#define RIGHT_BUTTON_DDR DDRD
		#define RIGHT_BUTTON_BIT PORTD6

		#define PIN_UP_BUTTON 3
		#define UP_BUTTON_PORT PORTD
		#define UP_BUTTON_PORTIN PIND
		#define UP_BUTTON_DDR DDRD
		#define UP_BUTTON_BIT PORTD3

		#define PIN_DOWN_BUTTON 5
		#define DOWN_BUTTON_PORT PORTD
		#define DOWN_BUTTON_PORTIN PIND
		#define DOWN_BUTTON_DDR DDRD
		#define DOWN_BUTTON_BIT PORTD5

		#define PIN_A_BUTTON 4
		#define A_BUTTON_PORT PORTD
		#define A_BUTTON_PORTIN PIND
		#define A_BUTTON_DDR DDRD
		#define A_BUTTON_BIT PORTD4

		#define PIN_B_BUTTON 7
		#define B_BUTTON_PORT PORTD
		#define B_BUTTON_PORTIN PIND
		#define B_BUTTON_DDR DDRD
		#define B_BUTTON_BIT PORTD7
	#endif // END KEYS
#elif defined (ELBEARBOY)

	//define ADC_CONFIG_SAH_TIME_MY          (0x3F << ADC_CONFIG_SAH_TIME_S) //Время выборки очередного отсчета в тактах АЦП - используем значение по-умолчанию
	// нумерация каналов ADC - сквозная 0,1,2:
    //define myADC_SEL_CHANNEL(channel_selection) (ANALOG_REG->ADC_CONFIG = ((ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_MY)) & (~ADC_CONFIG_SEL_M)) | ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_MY) | ((channel_selection) << ADC_CONFIG_SEL_S))
	#define myADC_SEL_CHANNEL(channel_selection) (ANALOG_REG->ADC_CONFIG = ((ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_M)) & (~ADC_CONFIG_SEL_M)) | ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_M) | ((channel_selection) << ADC_CONFIG_SEL_S))
	
	#define ADC_EXTREF_OFF      0       /* Встроенный источник опорного напряжения 1,2 В */
	//define ADC_EXTREF_ON       1       /* Внешний источник опорного напряжения */
	//define ADC_EXTCLB_CLBREF      0       /* Настраиваемый ОИН */
	#define ADC_EXTCLB_ADCREF      1       /* Внешний вывод */
	//moved to RANDOM config saction:
	//define PIN_RANDOM 4 // ADC3 port_0_4
	//define CHAN_RANDOM 3 // ADC3 port_0_4

	#if defined (JOYSTICKANALOG)

		#define B_BUTTON_BIT 0 // D3/0 port_0_0
		#define B_BUTTON_PORTIN GPIO_0->STATE
		#define A_BUTTON_BIT 8 // D4/8 port_0_8
		#define A_BUTTON_PORTIN GPIO_0->STATE
		
		#define PIN_AXISX 5 // Ось X port_1_5
		#define PIN_AXISY 7 // Ось Y port_1_7

		#define CHAN_AXISX 0 // Ось X port_1_5
		#define CHAN_AXISY 1 // Ось Y port_1_7
	#elif defined (JOYSTICKDISCRETE) // JOYSTICKDISCRETE KEYS 
		#define B_BUTTON_BIT 8 // D7/8 port_1_8
		#define B_BUTTON_PORTIN GPIO_1->STATE
			
		#define A_BUTTON_BIT 9 // D8/9 port_1_9
		#define A_BUTTON_PORTIN GPIO_1->STATE

		#define LEFT_BUTTON_BIT 1 // D5/1 port_0_1
		#define LEFT_BUTTON_PORTIN GPIO_0->STATE

		#define RIGHT_BUTTON_BIT 0 // D3/0 port_0_0
		#define RIGHT_BUTTON_PORTIN GPIO_0->STATE

		#define UP_BUTTON_BIT 10 // D2/10 port_0_10
		#define UP_BUTTON_PORTIN GPIO_0->STATE

		#define DOWN_BUTTON_BIT 8 // D4/8 port_0_8
		#define DOWN_BUTTON_PORTIN GPIO_0->STATE
	
	#else // ECOSOLE KEYS
		
		#define B_BUTTON_BIT 8 // D7/8 port_1_8
		#define B_BUTTON_PORTIN GPIO_1->STATE
			
		#define A_BUTTON_BIT 8 // D4/8 port_0_8
		#define A_BUTTON_PORTIN GPIO_0->STATE

		#define LEFT_BUTTON_BIT 10 // D2/10 port_0_10
		#define LEFT_BUTTON_PORTIN GPIO_0->STATE

		#define RIGHT_BUTTON_BIT 2 // D6/0 port_0_2
		#define RIGHT_BUTTON_PORTIN GPIO_0->STATE

		#define UP_BUTTON_BIT 0 // D3/0 port_0_0
		#define UP_BUTTON_PORTIN GPIO_0->STATE

		#define DOWN_BUTTON_BIT 1 // D5/1 port_0_1
		#define DOWN_BUTTON_PORTIN GPIO_0->STATE
	
	#endif // END KEYS
#else
#if defined (MICROCADE)
#define PIN_LEFT_BUTTON A1
#define LEFT_BUTTON_PORT PORTF
#define LEFT_BUTTON_PORTIN PINF
#define LEFT_BUTTON_DDR DDRF
#define LEFT_BUTTON_BIT PORTF6

#define PIN_RIGHT_BUTTON A2
#define RIGHT_BUTTON_PORT PORTF
#define RIGHT_BUTTON_PORTIN PINF
#define RIGHT_BUTTON_DDR DDRF
#define RIGHT_BUTTON_BIT PORTF5

#define PIN_UP_BUTTON A3
#define UP_BUTTON_PORT PORTF
#define UP_BUTTON_PORTIN PINF
#define UP_BUTTON_DDR DDRF
#define UP_BUTTON_BIT PORTF4

#define PIN_DOWN_BUTTON A0
#define DOWN_BUTTON_PORT PORTF
#define DOWN_BUTTON_PORTIN PINF
#define DOWN_BUTTON_DDR DDRF
#define DOWN_BUTTON_BIT PORTF7
#else
#define PIN_LEFT_BUTTON A2
#define LEFT_BUTTON_PORT PORTF
#define LEFT_BUTTON_PORTIN PINF
#define LEFT_BUTTON_DDR DDRF
#define LEFT_BUTTON_BIT PORTF5

#define PIN_RIGHT_BUTTON A1
#define RIGHT_BUTTON_PORT PORTF
#define RIGHT_BUTTON_PORTIN PINF
#define RIGHT_BUTTON_DDR DDRF
#define RIGHT_BUTTON_BIT PORTF6

#define PIN_UP_BUTTON A0
#define UP_BUTTON_PORT PORTF
#define UP_BUTTON_PORTIN PINF
#define UP_BUTTON_DDR DDRF
#define UP_BUTTON_BIT PORTF7

#define PIN_DOWN_BUTTON A3
#define DOWN_BUTTON_PORT PORTF
#define DOWN_BUTTON_PORTIN PINF
#define DOWN_BUTTON_DDR DDRF
#define DOWN_BUTTON_BIT PORTF4
#endif

#ifndef SUPPORT_XY_BUTTONS
 #define PIN_A_BUTTON 7
 #define A_BUTTON_PORT PORTE
 #define A_BUTTON_PORTIN PINE
 #define A_BUTTON_DDR DDRE
 #define A_BUTTON_BIT PORTE6
#else
 #define PIN_A_BUTTON 8
 #define A_BUTTON_PORT PORTB
 #define A_BUTTON_PORTIN PINB
 #define A_BUTTON_DDR DDRB
 #define A_BUTTON_BIT PORTB4
#endif
#ifndef SUPPORT_XY_BUTTONS
 #define PIN_B_BUTTON 8
 #define B_BUTTON_PORT PORTB
 #define B_BUTTON_PORTIN PINB
 #define B_BUTTON_DDR DDRB
 #define B_BUTTON_BIT PORTB4
#else
 #define PIN_B_BUTTON 7
 #define B_BUTTON_PORT PORTE
 #define B_BUTTON_PORTIN PINE
 #define B_BUTTON_DDR DDRE
 #define B_BUTTON_BIT PORTE6
 #define PIN_X_BUTTON 11
 #define X_BUTTON_PORT PORTB
 #define X_BUTTON_PORTIN PINB
 #define X_BUTTON_DDR DDRB
 #define X_BUTTON_BIT PORTB7
 #define PIN_Y_BUTTON A4
 #define Y_BUTTON_PORT PORTF
 #define Y_BUTTON_PORTIN PINF
 #define Y_BUTTON_DDR DDRF
 #define Y_BUTTON_BIT PORTF1
#endif
#endif


#if defined (ECONSOLE)
	#define PIN_SPEAKER_1 9  /**< The pin number of the first lead of the speaker */
	#define PIN_SPEAKER_2 11 /**< The pin number of the second lead of the speaker. Don't used in ECONSOLE? */ 

	#define SPEAKER_1_PORT PORTB
	#define SPEAKER_1_DDR DDRB
	#define SPEAKER_1_BIT PORTB1

	#define SPEAKER_2_PORT PORTB
	#define SPEAKER_2_DDR DDRB
	#define SPEAKER_2_BIT PORTB3
#elif defined (ELBEARBOY)
	#define PIN_SPEAKER_1 3  // D9/3 port_0_3
	#define SPEAKER_1_PORT GPIO_0->STATE
	#define SPEAKER_1_BIT 3

	#define PIN_SPEAKER_2 1 //D11/1 port_1_1
	#define SPEAKER_2_PORT GPIO_1->STATE
	#define SPEAKER_2_BIT 1
#else
	#define PIN_SPEAKER_1 5  /**< The pin number of the first lead of the speaker */

	#define SPEAKER_1_PORT PORTC
	#define SPEAKER_1_DDR DDRC
	#define SPEAKER_1_BIT PORTC6

	#ifdef AB_ALTERNATE_WIRING
	  #define PIN_SPEAKER_2 6      //Pro Micro alternative for 2nd speaker pin
	  #define SPEAKER_2_PORT PORTD
	  #define SPEAKER_2_DDR DDRD
	  #define SPEAKER_2_BIT PORTD7
	#else
	  #define PIN_SPEAKER_2 13 /**< The pin number of the second lead of the speaker */
	  #define SPEAKER_2_PORT PORTC
	  #define SPEAKER_2_DDR DDRC
	  #define SPEAKER_2_BIT PORTC7
	#endif
#endif
// -----------------------

// ----- DevKit pins -----
#elif defined(AB_DEVKIT)

#define PIN_CS 6        // Display CS Arduino pin number
#define CS_PORT PORTD   // Display CS port
#define CS_BIT PORTD7   // Display CS physical bit number

#define PIN_DC 4        // Display D/C Arduino pin number
#define DC_PORT PORTD   // Display D/C port
#define DC_BIT PORTD4   // Display D/C physical bit number

#define PIN_RST 12      // Display reset Arduino pin number
#define RST_PORT PORTD  // Display reset port
#define RST_BIT PORTD6  // Display reset physical bit number

#define SPI_MOSI_PORT PORTB
#define SPI_MOSI_BIT PORTB2

#define SPI_SCK_PORT PORTB
#define SPI_SCK_BIT PORTB1

// map all LEDs to the single TX LED on DEVKIT
#define RED_LED 17
#define GREEN_LED 17
#define BLUE_LED 17

#define RX_LED_BIT PORTB0
#define BLUE_LED_PORT PORTB
#define BLUE_LED_BIT PORTB0

// bit values for button states
// these are determined by the buttonsState() function
#define LEFT_BUTTON _BV(5)
#define RIGHT_BUTTON _BV(2)
#define UP_BUTTON _BV(4)
#define DOWN_BUTTON _BV(6)
#define A_BUTTON _BV(1)
#define B_BUTTON _BV(0)

// pin values for buttons, probably shouldn't use these
#define PIN_LEFT_BUTTON 9
#define LEFT_BUTTON_PORT PORTB
#define LEFT_BUTTON_PORTIN PINB
#define LEFT_BUTTON_DDR DDRB
#define LEFT_BUTTON_BIT PORTB5

#define PIN_RIGHT_BUTTON 5
#define RIGHT_BUTTON_PORT PORTC
#define RIGHT_BUTTON_PORTIN PINC
#define RIGHT_BUTTON_DDR DDRC
#define RIGHT_BUTTON_BIT PORTC6

#define PIN_UP_BUTTON 8
#define UP_BUTTON_PORT PORTB
#define UP_BUTTON_PORTIN PINB
#define UP_BUTTON_DDR DDRB
#define UP_BUTTON_BIT PORTB4

#define PIN_DOWN_BUTTON 10
#define DOWN_BUTTON_PORT PORTB
#define DOWN_BUTTON_PORTIN PINB
#define DOWN_BUTTON_DDR DDRB
#define DOWN_BUTTON_BIT PORTB6

#define PIN_A_BUTTON A0
#define A_BUTTON_PORT PORTF
#define A_BUTTON_PORTIN PINF
#define A_BUTTON_DDR DDRF
#define A_BUTTON_BIT PORTF7

#define PIN_B_BUTTON A1
#define B_BUTTON_PORT PORTF
#define B_BUTTON_PORTIN PINF
#define B_BUTTON_DDR DDRF
#define B_BUTTON_BIT PORTF6

#define PIN_SPEAKER_1 A2
#define SPEAKER_1_PORT PORTF
#define SPEAKER_1_DDR DDRF
#define SPEAKER_1_BIT PORTF5
// SPEAKER_2 is purposely not defined for DEVKIT as it could potentially
// be dangerous and fry your hardware (because of the devkit wiring).
//
// Reference: https://github.com/Arduboy/Arduboy/issues/108
#endif
// --------------------

// ----- Pins common on Arduboy and DevKit -----

// Unconnected analog input used for noise by initRandomSeed()
#ifndef SUPPORT_XY_BUTTONS
	#ifdef ECONSOLE // A2=ADC2=PC2=D16; для SLIMBOY есть ошибка - там PORTF, хотя его нет в Atmega328p
		#define RAND_SEED_IN A2
		#define RAND_SEED_IN_PORT PORTC
		#define RAND_SEED_IN_BIT PORTC2
		

		#define RAND_SEED_IN_ADMUX (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1)  ) // ADC:b_1110=1.1V; "0100 1110"; REF: 0100-> Vcc with external capacitor at AREF pin;  REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=1, MUX2=1, MUX1=1, MUX0=0;
		#define RAND_START_IN_ADMUX (_BV(REFS1) | _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1)  ) // ADC:b_1110=1.1V; "1100 1110"; REF: 1100-> Internal 1.1V voltage reference with external capacitor at AREF pin;  REFS1=1, REFS0=1, ADLAR=0, MUX4=0, MUX3=1, MUX2=1, MUX1=1, MUX0=0;
		#define REF_BACK_IN_ADMUX (_BV(REFS0) | _BV(MUX1) ) // ADC:b_0010=AC2; "0100 0010"; REF: 0100-> Vcc with external capacitor at AREF pin;  REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=0, MUX2=0, MUX1=1, MUX0=0;
		
		#define XAXIS_IN_ADMUX (_BV(REFS0) ) // ADC:b_0000=AC0; "0100 0000"; REF: 0100-> Vcc with external capacitor at AREF pin;  REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=0, MUX2=0, MUX1=0, MUX0=0;
		#define YAXIS_IN_ADMUX (_BV(REFS0) | _BV(MUX0) ) // ADC:b_0001=AC1; "0100 0001"; REF: 0100-> Vcc with external capacitor at AREF pin; REFS1=0, REFS0=1, ADLAR=0, MUX4=0, MUX3=0, MUX2=0, MUX1=0, MUX0=1;
		 
	#elif defined (ELBEARBOY) // MIK32: A2=ADC3=port_0_4
		#define PIN_RANDOM 4 // ADC3 port_0_4
		#define CHAN_RANDOM 3 // ADC3 port_0_4
		//define CNAN_ADCINIT_NANO 3 // канал инициализации ACE-NANO?
	#else // ATmega32U4: A4=ADC1=PORTF1
		#define RAND_SEED_IN A4
		#define RAND_SEED_IN_PORT PORTF
		#define RAND_SEED_IN_BIT PORTF1
		// Value for ADMUX to read the random seed pin: 2.56V reference, ADC1
		#define RAND_SEED_IN_ADMUX (_BV(REFS0) | _BV(REFS1) | _BV(MUX0))
	#endif
#else
	#define RAND_SEED_IN A5
	#define RAND_SEED_IN_PORT PORTF
	#define RAND_SEED_IN_BIT PORTF0
	// Value for ADMUX to read the random seed pin: 2.56V reference, ADC1
	#define RAND_SEED_IN_ADMUX (_BV(REFS0) | _BV(REFS1))
#endif

// SPI interface
#define SPI_MISO_PORT PORTB
#define SPI_MISO_BIT PORTB3

#define SPI_MOSI_PORT PORTB
#define SPI_MOSI_BIT PORTB2

#define SPI_SCK_PORT PORTB
#define SPI_SCK_BIT PORTB1

#define SPI_SS_PORT PORTB
#define SPI_SS_BIT PORTB0 //Note SPI SS pin is the same pin as for RXLED

// --------------------

// OLED hardware (SSD1306,SSD1309,SH1106,OLED_96X96,OLED_128x96)

#define OLED_PIXELS_INVERTED 0xA7 // All pixels inverted
#define OLED_PIXELS_NORMAL 0xA6 // All pixels normal

#define OLED_ALL_PIXELS_ON 0xA5 // all pixels on
#define OLED_PIXELS_FROM_RAM 0xA4 // pixels mapped to display RAM contents

#define OLED_VERTICAL_FLIPPED 0xC0 // reversed COM scan direction
#define OLED_VERTICAL_NORMAL 0xC8 // normal COM scan direction

#define OLED_HORIZ_FLIPPED 0xA0 // reversed segment re-map
#define OLED_HORIZ_NORMAL 0xA1 // normal segment re-map

#define OLED_SET_PAGE_ADDRESS      0xB0
#if defined(OLED_SH1106) || defined(OLED_SH1106_I2C)
  #define OLED_SET_COLUMN_ADDRESS_LO 0x02 //SH1106 only: 1st pixel starts on column 2
#else
  #define OLED_SET_COLUMN_ADDRESS_LO 0x00
#endif
#define OLED_SET_COLUMN_ADDRESS_HI 0x10

// -----
#if defined (OLED_96X96) || (OLED_96X96_ON_128X128)
  #define WIDTH 96
#else
  #define WIDTH 128 //The standard width of the display in pixels
#endif
#if defined(OLED_128X128)
  #define HEIGHT 128
#elif defined(OLED_96X96) || defined(OLED_128X96) || defined(OLED_96X96_ON_128X128) || defined(OLED_128X96_ON_128X128)
  #define HEIGHT 96
#else
  #define HEIGHT 64 //The standard height of the display in pixels
#endif

#define COLUMN_ADDRESS_END (WIDTH - 1) & 127   // 128 pixels wide
#define PAGE_ADDRESS_END ((HEIGHT/8)-1) & 7    // 8 pages high

/** \brief
 * Eliminate the USB stack to free up code space.
 *
 * \warning
 * Removing the USB code will make it impossible for sketch uploader
 * programs to automatically force a reset into the bootloader!
 * This means that a user will manually have to invoke a reset in order to
 * upload a new sketch, after one without USB has be been installed.
 * Be aware that the timing for the point that a reset must be initiated can
 * be tricky, which could lead to some frustration on the user's part.
 *
 * \details
 * \parblock
 * This macro will cause the USB code, normally included in the sketch as part
 * of the standard Arduino environment, to be eliminated. This will free up a
 * fair amount of program space, and some RAM space as well, at the expense of
 * disabling all USB functionality within the sketch (except as power input).
 *
 * The macro should be placed before the `setup()` function definition:
 *
 * \code{.cpp}
 * #include <Arduboy2.h>
 *
 * Arduboy2 arduboy;
 *
 * // (Other variable declarations, etc.)
 *
 * // Eliminate the USB stack
 * ARDUBOY_NO_USB
 *
 * void setup() {
 *   arduboy.begin();
 *   // any additional setup code
 * }
 * \endcode
 *
 * As stated in the warning above, without the USB code an uploader program
 * will be unable to automatically force a reset into the bootloader to upload
 * a new sketch. The user will have to manually invoke a reset. In addition to
 * eliminating the USB code, this macro will check if the DOWN button is held
 * when the sketch first starts and, if so, will call `exitToBootloader()` to
 * start the bootloader for uploading. This makes it easier for the user than
 * having to press the reset button.
 *
 * However, to make it even more convenient for a user to invoke the bootloader
 * it is highly recommended that a sketch using this macro include a menu or
 * prompt that allows the user to press the DOWN button within the sketch,
 * which should cause `exitToBootloader()` to be called.
 *
 * At a minimum, the documentation for the sketch should clearly state that a
 * manual reset will be required, and give detailed instructions on what the
 * user must do to upload a new sketch.
 * \endparblock
 *
 * \see Arduboy2Core::exitToBootloader()
 */
#define ARDUBOY_NO_USB int main() __attribute__ ((OS_main)); \
int main() { \
  Arduboy2NoUSB::mainNoUSB(); \
  return 0; \
}

// A replacement for the Arduino main() function that eliminates the USB code.
// Used by the ARDUBOY_NO_USB macro.
class Arduboy2NoUSB
{
  friend int main();

  private:
    static void mainNoUSB();
};


/** \brief
 * Lower level functions generally dealing directly with the hardware.
 *
 * \details
 * This class is inherited by Arduboy2Base and thus also Arduboy2, so wouldn't
 * normally be used directly by a sketch.
 *
 * \note
 * A friend class named _Arduboy2Ex_ is declared by this class. The intention
 * is to allow a sketch to create an _Arduboy2Ex_ class which would have access
 * to the private and protected members of the Arduboy2Core class. It is hoped
 * that this may eliminate the need to create an entire local copy of the
 * library, in order to extend the functionality, in most circumstances.
 */
class Arduboy2Core : public Arduboy2NoUSB
{
  friend class Arduboy2Ex;

  public:

    /** \brief
     * Idle the CPU to save power.
     *
     * \details
     * This puts the CPU in _idle_ sleep mode. You should call this as often
     * as you can for the best power savings. The timer 0 overflow interrupt
     * will wake up the chip every 1ms, so even at 60 FPS a well written
     * app should be able to sleep maybe half the time in between rendering
     * it's own frames.
     */
    static void idle();

    /** \brief
     * Put the display into data mode.
     *
     * \details
     * When placed in data mode, data that is sent to the display will be
     * considered as data to be displayed.
     *
     * \note
     * This is a low level function that is not intended for general use in a
     * sketch. It has been made public and documented for use by derived
     * classes.
     *
     * \see LCDCommandMode() SPItransfer()
     */
    static void inline LCDDataMode() __attribute__((always_inline))
    {
     #ifndef ELBEARBOY
		 #if defined(GU12864_800B)
		  bitClear(DC_PORT, DC_BIT);
		 #else
		  bitSet(DC_PORT, DC_BIT);
		 #endif
	#endif
    }
    /** \brief
     * Put the display into command mode.
     *
     * \details
     * When placed in command mode, data that is sent to the display will be
     * treated as commands.
     *
     * See the SSD1306 controller and OLED display documents for available
     * commands and command sequences.
     *
     * Links:
     *
     * - https://www.adafruit.com/datasheets/SSD1306.pdf
     * - http://www.buydisplay.com/download/manual/ER-OLED013-1_Series_Datasheet.pdf
     *
     * \note
     * This is a low level function that is not intended for general use in a
     * sketch. It has been made public and documented for use by derived
     * classes.
     *
     * \see LCDDataMode() sendLCDCommand() SPItransfer()
     */
    static void inline LCDCommandMode() __attribute__((always_inline))
    {
     #ifndef ELBEARBOY
		 #ifdef GU12864_800B
		  bitSet(DC_PORT, DC_BIT);
		 #else
		  bitClear(DC_PORT, DC_BIT);
		 #endif
	 #endif
    }
    /** \brief
     * Transfer a byte to the display.
     *
     * \param data The byte to be sent to the display.
     *
     * \details
     * Transfer one byte to the display over the SPI port and wait for the
     * transfer to complete. The byte will either be interpreted as a command
     * or as data to be placed on the screen, depending on the command/data
     * mode.
     *
     * \see LCDDataMode() LCDCommandMode() sendLCDCommand() SPItransferAndRead()
     */
    static void SPItransfer(uint8_t data);

    /** \brief
     * Transfer a byte to, and read a byte from, the SPI bus.
     *
     * \param data The byte to be sent.
     *
     * \return The byte that was received.
     *
     * \details
     * This function does the same as the `SPItransfer()` function but also
     * reads and returns the byte of data that was received during the
     * transfer.
     *
     * This function is of no use for a standard Arduboy, since only the
     * display is connected to the SPI bus and data cannot be received from
     * the display. It has been provided for use with homemade or expanded
     * units that have had additional peripherals added to the SPI bus that
     * are capable of sending data.
     *
     * \see SPItransfer()
     */
    static uint8_t SPItransferAndRead(uint8_t data);

#if defined(OLED_SSD1306_I2C) || defined(OLED_SSD1306_I2CX) || defined(OLED_SH1106_I2C)
    static void i2c_start(uint8_t mode);
	#if defined(ELBEARBOY)
		/* small delay for i2c
	*/
		#define __NOP() __asm volatile ("ADDI x0, x0, 0")
		#define __10NOP() __asm volatile ("ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n  ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n")
		#define __5NOP() __asm volatile ("ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n  ADDI x0, x0, 0 \n ADDI x0, x0, 0 \n")
		//static void  Delay_us (uint32_t us);
		
    #endif
	static void inline i2c_stop() __attribute__((always_inline))
    {
      // SDA and SCL both are already low, from writing ACK bit no need to change state
      I2C_SDA_AS_INPUT(); // switch to input so SDA is pulled up externally first for stop condition
      I2C_SCL_AS_INPUT(); // pull up SCL externally
    }

    static void i2c_sendByte(uint8_t byte);
#endif

//#endif

    /** \brief
     * Turn the display off.
     *
     * \details
     * The display will clear and be put into a low power mode. This can be
     * used to extend battery life when a game is paused or when a sketch
     * doesn't require anything to be displayed for a relatively long period
     * of time.
     *
     * \see displayOn()
     */
    static void displayOff();

    /** \brief
     * Turn the display on.
     *
     * \details
     * Used to power up and reinitialize the display after calling
     * `displayOff()`.
     *
     * \note
     * The previous call to `displayOff()` will have caused the display's
     * buffer contents to be lost. The display will have to be re-painted,
     * which is usually done by calling `display()`.
     *
     * \see displayOff()
     */
    static void displayOn();

    /** \brief
     * Get the width of the display in pixels.
     *
     * \return The width of the display in pixels.
     */
    static constexpr uint8_t width() { return WIDTH; }

    /** \brief
     * Get the height of the display in pixels.
     *
     * \return The height of the display in pixels.
     */
    static constexpr uint8_t height() { return HEIGHT; }

    /** \brief
     * Get the current state of all buttons as a bitmask.
     *
     * \return A bitmask of the state of all the buttons.
     *
     * \details
     * The returned mask contains a bit for each button. For any pressed button,
     * its bit will be 1. For released buttons their associated bits will be 0.
     *
     * The following defined mask values should be used for the buttons:
     *
     * LEFT_BUTTON, RIGHT_BUTTON, UP_BUTTON, DOWN_BUTTON, A_BUTTON, B_BUTTON
     */
    static uint8_t buttonsState();

    /** \brief
     * Paint 8 pixels vertically to the display.
     *
     * \param pixels A byte whose bits specify a vertical column of 8 pixels.
     *
     * \details
     * A byte representing a vertical column of 8 pixels is written to the
     * display at the current page and column address. The address is then
     * incremented. The page/column address will wrap to the start of the
     * display (the top left) when it increments past the end (lower right).
     *
     * The least significant bit represents the top pixel in the column.
     * A bit set to 1 is lit, 0 is unlit.
     *
     * Example:
     *
     *     X = lit pixels, . = unlit pixels
     *
     *     blank()                          paint8Pixels() 0xFF, 0, 0xF0, 0, 0x0F
     *     v TOP LEFT corner (8x9)          v TOP LEFT corner
     *     . . . . . . . . (page 1)         X . . . X . . . (page 1)
     *     . . . . . . . .                  X . . . X . . .
     *     . . . . . . . .                  X . . . X . . .
     *     . . . . . . . .                  X . . . X . . .
     *     . . . . . . . .                  X . X . . . . .
     *     . . . . . . . .                  X . X . . . . .
     *     . . . . . . . .                  X . X . . . . .
     *     . . . . . . . . (end of page 1)  X . X . . . . . (end of page 1)
     *     . . . . . . . . (page 2)         . . . . . . . . (page 2)
     */
    static void paint8Pixels(uint8_t pixels);

    /** \brief
     * Paints an entire image directly to the display from program memory.
     *
     * \param image A byte array in program memory representing the entire
     * contents of the display.
     *
     * \details
     * The contents of the specified array in program memory is written to the
     * display. Each byte in the array represents a vertical column of 8 pixels
     * with the least significant bit at the top. The bytes are written starting
     * at the top left, progressing horizontally and wrapping at the end of each
     * row, to the bottom right. The size of the array must exactly match the
     * number of pixels in the entire display.
     *
     * \see paint8Pixels()
     */
    static void paintScreen(const uint8_t *image);

    /** \brief
     * Paints an entire image directly to the display from an array in RAM.
     *
     * \param image A byte array in RAM representing the entire contents of
     * the display.
     * \param clear If `true` the array in RAM will be cleared to zeros upon
     * return from this function. If `false` the RAM buffer will remain
     * unchanged. (optional; defaults to `false`)
     *
     * \details
     * The contents of the specified array in RAM is written to the display.
     * Each byte in the array represents a vertical column of 8 pixels with
     * the least significant bit at the top. The bytes are written starting
     * at the top left, progressing horizontally and wrapping at the end of
     * each row, to the bottom right. The size of the array must exactly
     * match the number of pixels in the entire display.
     *
     * If parameter `clear` is set to `true` the RAM array will be cleared to
     * zeros after its contents are written to the display.
     *
     * \see paint8Pixels()
     */
    static void paintScreen(uint8_t image[], bool clear = false);

    /** \brief
     * Blank the display screen by setting all pixels off.
     *
     * \details
     * All pixels on the screen will be written with a value of 0 to turn
     * them off.
     */
    static void blank();

    /** \brief
     * Invert the entire display or set it back to normal.
     *
     * \param inverse `true` will invert the display. `false` will set the
     * display to no-inverted.
     *
     * \details
     * Calling this function with a value of `true` will set the display to
     * inverted mode. A pixel with a value of 0 will be on and a pixel set to 1
     * will be off.
     *
     * Once in inverted mode, the display will remain this way
     * until it is set back to non-inverted mode by calling this function with
     * `false`.
     */
    static void invert(bool inverse);

    /** \brief
     * Turn all display pixels on or display the buffer contents.
     *
     * \param on `true` turns all pixels on. `false` displays the contents
     * of the hardware display buffer.
     *
     * \details
     * Calling this function with a value of `true` will override the contents
     * of the hardware display buffer and turn all pixels on. The contents of
     * the hardware buffer will remain unchanged.
     *
     * Calling this function with a value of `false` will set the normal state
     * of displaying the contents of the hardware display buffer.
     *
     * \note
     * All pixels will be lit even if the display is in inverted mode.
     *
     * \see invert()
     */
    static void allPixelsOn(bool on);

    /** \brief
     * Flip the display vertically or set it back to normal.
     *
     * \param flipped `true` will set vertical flip mode. `false` will set
     * normal vertical orientation.
     *
     * \details
     * Calling this function with a value of `true` will cause the Y coordinate
     * to start at the bottom edge of the display instead of the top,
     * effectively flipping the display vertically.
     *
     * Once in vertical flip mode, it will remain this way until normal
     * vertical mode is set by calling this function with a value of `false`.
     *
     * \see flipHorizontal()
     */
    static void flipVertical(bool flipped);

    /** \brief
     * Flip the display horizontally or set it back to normal.
     *
     * \param flipped `true` will set horizontal flip mode. `false` will set
     * normal horizontal orientation.
     *
     * \details
     * Calling this function with a value of `true` will cause the X coordinate
     * to start at the left edge of the display instead of the right,
     * effectively flipping the display horizontally.
     *
     * Once in horizontal flip mode, it will remain this way until normal
     * horizontal mode is set by calling this function with a value of `false`.
     *
     * \see flipVertical()
     */
    static void flipHorizontal(bool flipped);

    /** \brief
     * Send a single command byte to the display.
     *
     * \param command The command byte to send to the display.
     *
     * \details
     * The display will be set to command mode then the specified command
     * byte will be sent. The display will then be set to data mode.
     * Multi-byte commands can be sent by calling this function multiple times.
     *
     * \note
     * Sending improper commands to the display can place it into invalid or
     * unexpected states, possibly even causing physical damage.
     */
    static void sendLCDCommand(uint8_t command);

    static void inline setRGBledRedOn()__attribute__((always_inline))
    {
     #ifndef LCD_ST7565
      bitClear(RED_LED_PORT, RED_LED_BIT); // Red on
     #else
      bitSet(RED_LED_PORT, RED_LED_BIT); // Red on
     #endif
    }

    static void inline setRGBledRedOff()__attribute__((always_inline))
    {
     #ifndef LCD_ST7565
      bitSet(RED_LED_PORT, RED_LED_BIT); // Red off
     #else
      bitClear(RED_LED_PORT, RED_LED_BIT); // Red off
     #endif
    }

    static void inline setRGBledGreenOn()__attribute__((always_inline))
    {
     #ifndef LCD_ST7565
      bitClear(GREEN_LED_PORT, GREEN_LED_BIT); // Green on
     #else
      bitSet(GREEN_LED_PORT, GREEN_LED_BIT); // Green on
     #endif
    }

    static void inline setRGBledGreenOff()__attribute__((always_inline))
    {
     #ifndef LCD_ST7565
      bitSet(GREEN_LED_PORT, GREEN_LED_BIT); // Green off
     #else
      bitClear(GREEN_LED_PORT, GREEN_LED_BIT); // Green off
     #endif
    }

    static void inline setRGBledBlueOn()__attribute__((always_inline))
    {
     #ifndef LCD_ST7565
      bitClear(BLUE_LED_PORT, BLUE_LED_BIT); // Blue on
     #else
      bitSet(BLUE_LED_PORT, BLUE_LED_BIT); // Blue on
     #endif
    }

    static void inline setRGBledBlueOff()__attribute__((always_inline))
    {
     #ifndef LCD_ST7565
      bitSet(BLUE_LED_PORT, BLUE_LED_BIT); // Blue off
     #else
      bitClear(BLUE_LED_PORT, BLUE_LED_BIT); // Blue off
     #endif
    }

    /** \brief
     * Set the light output of the RGB LED.
     *
     * \param red,green,blue The brightness value for each LED.
     *
     * \details
     * The RGB LED is actually individual red, green and blue LEDs placed
     * very close together in a single package. By setting the brightness of
     * each LED, the RGB LED can show various colors and intensities.
     * The brightness of each LED can be set to a value from 0 (fully off)
     * to 255 (fully on).
     *
     * \note
     * \parblock
     * Certain libraries that take control of the hardware timers may interfere
     * with the ability of this function to properly control the RGB LED.
     *_ArduboyPlaytune_ is one such library known to do this.
     * The digitalWriteRGB() function will still work properly in this case.
     * \endparblock
     *
     * \note
     * \parblock
     * Many of the Kickstarter Arduboys were accidentally shipped with the
     * RGB LED installed incorrectly. For these units, the green LED cannot be
     * lit. As long as the green led is set to off, setting the red LED will
     * actually control the blue LED and setting the blue LED will actually
     * control the red LED. If the green LED is turned fully on, none of the
     * LEDs will light.
     * \endparblock
     *
     * \see setRGBled(uint8_t, uint8_t) digitalWriteRGB() freeRGBled()
     */
    static void setRGBled(uint8_t red, uint8_t green, uint8_t blue);

    /** \brief
     * Set the brightness of one of the RGB LEDs without affecting the others.
     *
     * \param color The name of the LED to set. The value given should be one
     * of RED_LED, GREEN_LED or BLUE_LED.
     *
     * \param val The brightness value for the LED, from 0 to 255.
     *
     * \note
     * In order to use this function, the 3 parameter version must first be
     * called at least once, in order to initialize the hardware.
     *
     * \details
     * This 2 parameter version of the function will set the brightness of a
     * single LED within the RGB LED without affecting the current brightness
     * of the other two. See the description of the 3 parameter version of this
     * function for more details on the RGB LED.
     *
     * \see setRGBled(uint8_t, uint8_t, uint8_t) digitalWriteRGB() freeRGBled()
     */
    static void setRGBled(uint8_t color, uint8_t val);


    /** \brief
     * Relinquish analog control of the RGB LED.
     *
     * \details
     * Using the RGB LED in analog mode prevents further use of the LED in
     * digital mode. This function will restore the pins used for the LED, so
     * it can be used in digital mode.
     *
     * \see digitalWriteRGB() setRGBled()
     */
    static void freeRGBled();

    /** \brief
     * Set the RGB LEDs digitally, to either fully on or fully off.
     *
     * \param red,green,blue Use value RGB_ON or RGB_OFF to set each LED.
     *
     * \details
     * The RGB LED is actually individual red, green and blue LEDs placed
     * very close together in a single package. This 3 parameter version of the
     * function will set each LED either on or off, to set the RGB LED to
     * 7 different colors at their highest brightness or turn it off.
     *
     * The colors are as follows:
     *
     * | RED LED | GREEN LED | BLUE LED |  COLOR  |
     * | ------- | --------- | -------- | :-----: |
     * | RGB_OFF |  RGB_OFF  | RGB_OFF  | OFF     |
     * | RGB_OFF |  RGB_OFF  | RGB_ON   | Blue    |
     * | RGB_OFF |  RGB_ON   | RGB_OFF  | Green   |
     * | RGB_OFF |  RGB_ON   | RGB_ON   | Cyan    |
     * | RGB_ON  |  RGB_OFF  | RGB_OFF  | Red     |
     * | RGB_ON  |  RGB_OFF  | RGB_ON   | Magenta |
     * | RGB_ON  |  RGB_ON   | RGB_OFF  | Yellow  |
     * | RGB_ON  |  RGB_ON   | RGB_ON   | White   |
     *
     * \note
     * \parblock
     * Using the RGB LED in analog mode will prevent digital control of the
     * LED. To restore the ability to control the LED digitally, use the
     * `freeRGBled()` function.
     * \endparblock
     *
     * \note
     * \parblock
     * Many of the Kickstarter Arduboys were accidentally shipped with the
     * RGB LED installed incorrectly. For these units, the green LED cannot be
     * lit. As long as the green led is set to off, turning on the red LED will
     * actually light the blue LED and turning on the blue LED will actually
     * light the red LED. If the green LED is turned on, none of the LEDs
     * will light.
     * \endparblock
     *
     * \see digitalWriteRGB(uint8_t, uint8_t) setRGBled() freeRGBled()
     */
    static void digitalWriteRGB(uint8_t red, uint8_t green, uint8_t blue);

    /** \brief
     * Set one of the RGB LEDs digitally, to either fully on or fully off.
     *
     * \param color The name of the LED to set. The value given should be one
     * of RED_LED, GREEN_LED or BLUE_LED.
     *
     * \param val Indicates whether to turn the specified LED on or off.
     * The value given should be RGB_ON or RGB_OFF.
     *
     * \details
     * This 2 parameter version of the function will set a single LED within
     * the RGB LED either fully on or fully off. See the description of the
     * 3 parameter version of this function for more details on the RGB LED.
     *
     * \see digitalWriteRGB(uint8_t, uint8_t, uint8_t) setRGBled() freeRGBled()
     */
    static void digitalWriteRGB(uint8_t color, uint8_t val);

    /** \brief
     * Initialize the Arduboy's hardware.
     *
     * \details
     * This function initializes the display, buttons, etc.
     *
     * This function is called by `begin()` so isn't normally called within a
     * sketch. However, in order to free up some code space, by eliminating
     * some of the start up features, it can be called in place of begin().
     * The functions that `begin()` would call after `boot()` can then be
     * called to add back in some of the start up features as space permits.
     *
     * See the README file or main page, in section
     * _Substitute or remove boot up features_, for more details.
     *
     * \warning
     * If this function is used, it is recommended that at least `flashlight()`
     * or `safeMode()` be called after it to provide a means to upload a new
     * sketch if the bootloader "magic number" problem is encountered.
     *
     * \see Arduboy2::begin() Arduboy2Base::flashlight() safeMode()
     */
    static void boot();

    /** \brief
     * Allow upload when the bootloader "magic number" could be corrupted.
     *
     * \details
     * If the UP button is held when this function is entered, the RGB LED
     * will be lit and timer 0 will be disabled, then the sketch will remain
     * in a tight loop. This is to address a problem with uploading a new
     * sketch, for sketches that interfere with the bootloader "magic number".
     * The problem occurs with certain sketches that use large amounts of RAM.
     *
     * This function should be called after `boot()` in sketches that don't
     * call `flashlight()`.
     *
     * It is intended to replace the `flashlight()` function when more
     * program space is required. If possible, it is more desirable to use
     * `flashlight()`, so that the actual flashlight feature isn't lost.
     *
     * \see Arduboy2Base::flashlight() boot()
     */
    static void safeMode();

    /** \brief
     * Create a seed suitable for use with a pseudorandom number generator.
     *
     * \return A random value that can be used to seed a
     * pseudorandom number generator.
     *
     * \details
     * The returned value will be a random value derived from entropy from an
     * ADC reading of a floating pin combined with the microseconds since boot.
     *
     * \note
     * This function will be more effective if called after a semi-random time,
     * such as after waiting for the user to press a button to start a game, or
     * another event that takes a variable amount of time after boot.
     *
     * \see Arduboy2Base::initRandomSeed()
     */
    static unsigned long generateRandomSeed();

    /** \brief
     * Delay for the number of milliseconds, specified as a 16 bit value.
     *
     * \param ms The delay in milliseconds.
     *
     * \details
     * This function works the same as the Arduino `delay()` function except
     * the provided value is 16 bits long, so the maximum delay allowed is
     * 65535 milliseconds (about 65.5 seconds). Using this function instead
     * of Arduino `delay()` will save a few bytes of code.
     */
   #ifndef ARDUBOY_CORE
    static void delayShort(uint16_t ms) __attribute__ ((noinline));
   #else
    static void delayShort(uint16_t ms);
   #endif
    static void delayByte(uint8_t ms) __attribute__ ((noinline));

    /** \brief
     * Exit the sketch and start the bootloader
     *
     * \details
     * The sketch will exit and the bootloader will be started in command mode.
     * The effect will be similar to pressing the reset button.
     *
     * This function is intended to be used to allow uploading a new sketch,
     * when the USB code has been removed to gain more code space.
     * Ideally, the sketch would present a "New Sketch Upload" menu or prompt
     * telling the user to "Press and hold the DOWN button when the procedure
     * to upload a new sketch has been initiated". The sketch would then wait
     * for the DOWN button to be pressed and then call this function.
     *
     * \see ARDUBOY_NO_USB
     */
    static void exitToBootloader();
	#if defined (ELBEARBOY)
	static	uint8_t chan_converted;
	static	uint8_t chan_selected;
	

	/* read 256 eerprom bytes from number 1792 to 2047
	*/
	static uint8_t read_eeprom_byte(uint16_t idx);
	/* update 32 (one page) eerprom bytes from number 1792 to 1823
	*/
	static void update_eeprom_1st_page_byte(uint16_t idx, uint8_t val);

	
	#endif
	#if defined (JOYSTICKANALOG)
	 static uint8_t ADCJoystickState;
	 static unsigned int JoystickXZero;
	 static unsigned int JoystickYZero;
	#endif
	
  protected:
    // internals
    static void setCPUSpeed8MHz();
    static void bootSPI();
    static void bootOLED();
    static void bootPins();
    static void bootPowerSaving();

    static const PROGMEM uint8_t lcdBootProgram[];
#if defined(GU12864_800B)
    static void displayWrite(uint8_t d);
    static void displayEnable();
    static void displayDisable();
#endif
};

#endif
