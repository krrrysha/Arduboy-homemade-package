/**
 * @file Arduboy2Beep.cpp
 * \brief
 * Classes to generate simple square wave tones on the Arduboy speaker pins.
 */

#include <Arduino.h>
#include "Arduboy2Beep.h"

#ifndef AB_DEVKIT

// Speaker pin 1, Timer 3A, Port C bit 6, Arduino pin 5

uint8_t BeepPin1::duration = 0;

void BeepPin1::begin()
{
#ifndef ELBEARBOY 
#ifdef ECONSOLE
  TCCR1A = 0;
  TCCR1B = (bit(WGM12) | bit(CS11)); // CTC mode. Divide by 8 clock prescale
#else 
  TCCR3A = 0;
  TCCR3B = (bit(WGM32) | bit(CS31)); // CTC mode. Divide by 8 clock prescale
#endif
#else
	// Timer32_1_ch4, D9= PORT 0.3 
	PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_1_M | PM_CLOCK_APB_P_GPIO_0_M;
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
	TIMER32_1->CHANNELS[3].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
	TIMER32_1->CHANNELS[3].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
	TIMER32_1->PRESCALER =  0; //Divide by 16 clock prescale
	TIMER32_1->INT_MASK =  0;
	TIMER32_1->INT_CLEAR =   0xFFFFFFFF;
	TIMER32_1->CHANNELS[3].OCR = 0;
	TIMER32_1->CHANNELS[3].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
#endif
}

void BeepPin1::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
  duration = dur;
#ifndef ELBEARBOY 
#ifdef ECONSOLE
  TCCR1A = bit(COM1A0); // set toggle on compare mode (which connects the pin)
  OCR1A = count; // load the count (16 bits), which determines the frequency
#else  
  TCCR3A = bit(COM3A0); // set toggle on compare mode (which connects the pin)
  OCR3A = count; // load the count (16 bits), which determines the frequency
#endif
#else
	TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M | ~(TIMER32_ENABLE_TIM_EN_M); // без  этого таймер временно "зависает" при быстрой смене TOP/OCR
	TIMER32_1->TOP = (count*32);
	TIMER32_1->CHANNELS[3].OCR = TIMER32_1->TOP>>1;
	TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M | TIMER32_ENABLE_TIM_EN_M;
	//TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_EN_M;
#endif	
}

void BeepPin1::timer()
{
  if (duration && (--duration == 0)) {
#ifndef ELBEARBOY
  #ifdef ECONSOLE
    TCCR1A = 0; // set normal mode (which disconnects the pin)	  
  #else
    TCCR3A = 0; // set normal mode (which disconnects the pin)
 #endif
#else
	TIMER32_1->CHANNELS[3].OCR = 0;
#endif
  }
}

void BeepPin1::noTone()
{
  duration = 0;
#ifndef ELBEARBOY
  #ifdef ECONSOLE
  TCCR1A = 0; // set normal mode (which disconnects the pin)	  
  #else  
  TCCR3A = 0; // set normal mode (which disconnects the pin)
  #endif
#else
	TIMER32_1->CHANNELS[3].OCR = 0;
#endif
}


// Speaker pin 2, Timer 4A, Port C bit 7, Arduino pin 13 or Port D bit 7, Arduino pin 6 for alternate wiring

uint8_t BeepPin2::duration = 0;

void BeepPin2::begin()
{
#ifndef ELBEARBOY 
#ifdef ECONSOLE
  TCCR2A = 0; // normal mode. Disable PWM
  TCCR2B = bit(CS22) | bit(CS20); // divide by 128 clock prescale
  OCR2A = 0; //  "
#else
  TCCR4A = 0; // normal mode. Disable PWM
  TCCR4B = bit(CS43); // divide by 128 clock prescale
 #ifdef AB_ALTERNATE_WIRING
  TCCR4C = 0; // normal mode
 #endif
  TCCR4D = 0; // normal mode
  TC4H = 0;  // toggle pin at count = 0
  OCR4A = 0; //  "
#endif
#else
	// // Timer32_2_ch2, D11= PORT 1.1 
	PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_1_M | PM_CLOCK_APB_P_GPIO_1_M;
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
	TIMER32_2->CHANNELS[1].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
	TIMER32_2->CHANNELS[1].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
	TIMER32_2->PRESCALER =  0; //Divide by 16 clock prescale
	TIMER32_2->INT_MASK =  0;
	TIMER32_2->INT_CLEAR =   0xFFFFFFFF;
	TIMER32_2->CHANNELS[1].OCR = 0;
	TIMER32_2->CHANNELS[1].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
#endif
}

void BeepPin2::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin2::tone(uint16_t count, uint8_t dur)
{

  duration = dur;
#ifndef ELBEARBOY
 #ifdef ECONSOLE
  TCCR2A = bit(WGM21) | bit(COM2A0); // CTC mode, toggle on compare mode (which connects the pin)
  OCR2A = lowByte(count); //  which determines the frequency
#else
 #ifdef AB_ALTERNATE_WIRING
  TCCR4C = bit(COM4D0); // set toggle on compare mode (which connects pin 6)
 #else
  TCCR4A = bit(COM4A0); // set toggle on compare mode (which connects pin 13)
 #endif
 TC4H = highByte(count); // load the count (10 bits),
  OCR4C = lowByte(count); //  which determines the frequency
#endif 
#else
	TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_CLR_M | ~(TIMER32_ENABLE_TIM_EN_M);
	TIMER32_2->TOP = (count*32);
	TIMER32_2->CHANNELS[1].OCR = TIMER32_2->TOP>>1;
	TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_CLR_M | TIMER32_ENABLE_TIM_EN_M;
	//TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_EN_M;
#endif	
}

void BeepPin2::timer()
{
  if (duration && (--duration == 0)) {
    noTone();
  }
}

void BeepPin2::noTone()
{
  duration = 0;
#ifndef ELBEARBOY
#ifdef ECONSOLE
  TCCR2A = 0; // set normal mode (which disconnects the pin)
#else     
 #ifdef AB_ALTERNATE_WIRING
  TCCR4C = 0; // set normal mode (which disconnects the pin)
 #else
  TCCR4A = 0; // set normal mode (which disconnects the pin)
 #endif
#endif 
#else
	TIMER32_2->CHANNELS[1].OCR = 0;
#endif
}


#else /* AB_DEVKIT */

// *** The pins used for the speaker on the DevKit cannot be directly
// controlled by a timer/counter. The following "dummy" functions will
// compile and operate properly but no sound will be produced

uint8_t BeepPin1::duration = 0;

void BeepPin1::begin()
{
}

void BeepPin1::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
  (void) count; // parameter not used

  duration = dur;
}

void BeepPin1::timer()
{
  if (duration) {
    --duration;
  }
}

void BeepPin1::noTone()
{
  duration = 0;
}


uint8_t BeepPin2::duration = 0;

void BeepPin2::begin()
{
}

void BeepPin2::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin2::tone(uint16_t count, uint8_t dur)
{
  (void) count; // parameter not used

  duration = dur;
}

void BeepPin2::timer()
{
  if (duration) {
    --duration;
  }
}

void BeepPin2::noTone()
{
  duration = 0;
}

#endif
