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

#ifndef BEARBOARD 
	#ifdef ECONSOLE
	  TCCR1A = 0;
	  TCCR1B = (bit(WGM12) | bit(CS11)); // CTC mode. Divide by 8 clock prescale
	#else 
	  TCCR3A = 0;
	  TCCR3B = (bit(WGM32) | bit(CS31)); // CTC mode. Divide by 8 clock prescale
	#endif
#else // BEARBOARD
	#ifndef	SPIBEAR
		// Timer32_1_ch4, D9= PORT 0.3 
		if (~(GPIO_0->DIRECTION_IN & (1 << BEEPER_1_BIT))) {PAD_CONFIG->PORT_0_CFG |= (0b10 << (2 * BEEPER_1_BIT));} // установка вывода 3 порта 0 (в режим 0xb10). Timer Connect!

		PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_1_M | PM_CLOCK_APB_P_GPIO_0_M;
		PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
		// по умолчанию SOURCE=0 - APB_P
		TIMER32_1->CHANNELS[3].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M; // выключение канала
		//кроме того прямой ШИМ по-умолчанию
		TIMER32_1->CHANNELS[3].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // режим ШИМ MODE=0b11 
		TIMER32_1->PRESCALER =  0; //Divide by 1 clock prescale
		TIMER32_1->INT_MASK =  0; // прерывания таймера 1 отключены
		TIMER32_1->INT_CLEAR =   0xFFFFFFFF; 
		TIMER32_1->CHANNELS[3].OCR = 0; // значение сравнения
		TIMER32_1->CHANNELS[3].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M; // включение канала
	#else
		// Пробуем Timer16_1_out PORT_0_10 D2
		if (~(GPIO_0->DIRECTION_IN & (1 << BEEPER_1_BIT))) {PAD_CONFIG->PORT_0_CFG |= (0b10 << (2 * BEEPER_1_BIT));} // установка вывода 10 порта 0 (в режим 0xb10). Timer Connect!
		PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER16_1_M | PM_CLOCK_APB_P_GPIO_0_M;
		PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
		TIMER16_1->CR &= ~TIMER16_CR_ENABLE_M;
		// Регистры CFGR и IER должны быть изменены только тогда, когда TIMER16 отключен.
		TIMER16_1->CFGR = (TIMER16_1->CFGR & ~TIMER16_CFGR_PRESC_M) | (0b101 << TIMER16_CFGR_PRESC_S);  // делитель /32
		TIMER16_1->CFGR &= ~TIMER16_CFGR_WAVE_M; //Настроить таймер с ШИМ сигналом

		TIMER16_1->ARR = 0; //При отключении таймера и повторном включении необходимо заново проинициализировать значение в регистре ARR.
		TIMER16_1->CMP = 0;	
		//TIMER16_1->ICR = 0; 
		TIMER16_1->ICR = 0xFFFFFFFF;
		TIMER16_1->CR |= TIMER16_CR_ENABLE_M;
	#endif	
#endif
}

void BeepPin1::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin1::tone(uint16_t count, uint8_t dur)
{
  duration = dur;
#ifndef BEARBOARD 
	#ifdef ECONSOLE
	  TCCR1A = bit(COM1A0); // set toggle on compare mode (which connects the pin)
	  OCR1A = count; // load the count (16 bits), which determines the frequency
	#else  
	  TCCR3A = bit(COM3A0); // set toggle on compare mode (which connects the pin)
	  OCR3A = count; // load the count (16 bits), which determines the frequency
	#endif
#else // BEARBOARD
	#ifndef	SPIBEAR
		TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M | ~(TIMER32_ENABLE_TIM_EN_M); // без  этого таймер временно "зависает" при быстрой смене TOP/OCR
		TIMER32_1->TOP = (count*32); // максимальное значение. вычисляется в *.h (F_CPU / 16 / 2) + (hz / 2)) / hz
		TIMER32_1->CHANNELS[3].OCR = TIMER32_1->TOP>>1; // значение сравнения OCR=TOP/2
		TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M | TIMER32_ENABLE_TIM_EN_M;
		//TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_EN_M;
	#else
		//TIMER16_1->CR |= TIMER16_CR_ENABLE_M;
		TIMER16_1->CR |=  TIMER16_CR_CNTSTRT_M; //запустить таймер непрерывно
		TIMER16_1->ARR = count;  // максимальное значение. вычисляется в *.h (F_CPU / 16 / 2) + (hz / 2)) / hz. 
		//При отключении таймера и повторном включении необходимо заново проинициализировать значение в регистре ARR.
		TIMER16_1->CMP = count >>1;	
		//while (!( (TIMER16_1->ISR & TIMER16_ISR_ARR_OK_M) & (TIMER16_1->ISR & TIMER16_ISR_CMP_OK_M)));
		//TIMER16_1->ICR = 0; 
		TIMER16_1->ICR = 0xFFFFFFFF;
	#endif
#endif	
}

void BeepPin1::timer()
{
  if (duration && (--duration == 0)) {
#ifndef BEARBOARD
  #ifdef ECONSOLE
    TCCR1A = 0; // set normal mode (which disconnects the pin)	  
  #else
    TCCR3A = 0; // set normal mode (which disconnects the pin)
 #endif
#else // BEARBOARD
		#ifndef	SPIBEAR
			TIMER32_1->CHANNELS[3].OCR = 0;
		#else
			TIMER16_1->CMP = 0;
		#endif
#endif
  }
}

void BeepPin1::noTone()
{
  duration = 0;
#ifndef BEARBOARD
  #ifdef ECONSOLE
  TCCR1A = 0; // set normal mode (which disconnects the pin)	  
  #else  
  TCCR3A = 0; // set normal mode (which disconnects the pin)
  #endif
#else // BEARBOARD
		#ifndef	SPIBEAR
			TIMER32_1->CHANNELS[3].OCR = 0;
		#else
			TIMER16_1->CMP = 0;
		#endif
#endif
}


// Speaker pin 2, Timer 4A, Port C bit 7, Arduino pin 13 or Port D bit 7, Arduino pin 6 for alternate wiring

uint8_t BeepPin2::duration = 0;

void BeepPin2::begin()
{
#ifndef BEARBOARD 
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
#else // BEARBOARD

	/*
	// // Timer32_2_ch2, D11= PORT 1.1 
	if (~(GPIO_1->DIRECTION_IN & (1 << BEEPER_2_BIT))) {PAD_CONFIG->PORT_1_CFG |= (0b10 << (2 * BEEPER_2_BIT));} // установка вывода 1 порта 1 (в режим 0xb10). Timer Connect!
	
	PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_2_M | PM_CLOCK_APB_P_GPIO_1_M;
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
	TIMER32_2->CHANNELS[1].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
	TIMER32_2->CHANNELS[1].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
	TIMER32_2->PRESCALER =  0; //Divide by 1 clock prescale
	TIMER32_2->INT_MASK =  0;
	TIMER32_2->INT_CLEAR =   0xFFFFFFFF;
	TIMER32_2->CHANNELS[1].OCR = 0;
	TIMER32_2->CHANNELS[1].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
	*/
	

	// Пробуем Timer16_0_out PORT_0_7 D17
	if (~(GPIO_0->DIRECTION_IN & (1 << BEEPER_2_BIT))) {PAD_CONFIG->PORT_0_CFG |= (0b10 << (2 * BEEPER_2_BIT));} // установка вывода 7 порта 0 (в режим 0xb10). Timer Connect!
 	PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER16_0_M | PM_CLOCK_APB_P_GPIO_0_M;
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;

	TIMER16_0->CR &= ~TIMER16_CR_ENABLE_M;

	// Регистры CFGR и IER должны быть изменены только тогда, когда TIMER16 отключен.
	TIMER16_0->CFGR = (TIMER16_0->CFGR & ~TIMER16_CFGR_PRESC_M) | (0b101 << TIMER16_CFGR_PRESC_S);  // делитель /32
	TIMER16_0->CFGR &= ~TIMER16_CFGR_WAVE_M; //Настроить таймер с ШИМ сигналом
	

	TIMER16_0->ARR = 0; //При отключении таймера и повторном включении необходимо заново проинициализировать значение в регистре ARR.
	TIMER16_0->CMP = 0;	
	//TIMER16_0->ICR = 0; 
	TIMER16_0->ICR = 0xFFFFFFFF;
	TIMER16_0->CR |= TIMER16_CR_ENABLE_M;

#endif
}

void BeepPin2::tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin2::tone(uint16_t count, uint8_t dur)
{

  duration = dur;
#ifndef BEARBOARD
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
#else // BEARBOARD
	//TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_CLR_M | ~(TIMER32_ENABLE_TIM_EN_M);
	//TIMER32_2->TOP = (count*32);
	//TIMER32_2->CHANNELS[1].OCR = TIMER32_2->TOP>>1;
	//TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_CLR_M | TIMER32_ENABLE_TIM_EN_M;
	////TIMER32_2->ENABLE = TIMER32_ENABLE_TIM_EN_M;
	
	//TIMER16_0->CR |= TIMER16_CR_ENABLE_M;
	TIMER16_0->CR |=  TIMER16_CR_CNTSTRT_M; // запустить таймер непрерывно
	
	TIMER16_0->ARR = count;  // максимальное значение. вычисляется в *.h (F_CPU / 16 / 2) + (hz / 2)) / hz. 
	//При отключении таймера и повторном включении необходимо заново проинициализировать значение в регистре ARR.
	TIMER16_0->CMP = count >>1;	
	//while (!( (TIMER16_0->ISR & TIMER16_ISR_ARR_OK_M) & (TIMER16_0->ISR & TIMER16_ISR_CMP_OK_M)));
	//TIMER16_0->ICR = 0; 
	TIMER16_0->ICR = 0xFFFFFFFF;
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
#ifndef BEARBOARD
	#ifdef ECONSOLE
	  TCCR2A = 0; // set normal mode (which disconnects the pin)
	#else     
	 #ifdef AB_ALTERNATE_WIRING
	  TCCR4C = 0; // set normal mode (which disconnects the pin)
	 #else
	  TCCR4A = 0; // set normal mode (which disconnects the pin)
	 #endif
	#endif 
#else // BEARBOARD
	//TIMER32_2->CHANNELS[1].OCR = 0;
	TIMER16_0->CMP = 0;
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
