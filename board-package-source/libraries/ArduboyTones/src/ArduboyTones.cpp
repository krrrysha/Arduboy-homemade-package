/**
 * @file ArduboyTones.cpp
 * \brief An Arduino library for playing tones and tone sequences, 
 * intended for the Arduboy game system.
 */

/*****************************************************************************
  ArduboyTones

An Arduino library to play tones and tone sequences.

Specifically written for use by the Arduboy miniature game system
https://www.arduboy.com/
but could work with other Arduino AVR boards that have 16 bit timer 3
available, by changing the port and bit definintions for the pin(s)
if necessary.

Copyright (c) 2017 Scott Allen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/

#include "ArduboyTones.h"

// pointer to a function that indicates if sound is enabled
static bool (*outputEnabled)();

static volatile long durationToggleCount = 0;
static volatile bool tonesPlaying = false;
static volatile bool toneSilent;
#ifdef TONES_VOLUME_CONTROL
static volatile bool toneHighVol;
static volatile bool forceHighVol = false;
static volatile bool forceNormVol = false;
#endif

static volatile uint16_t *tonesStart;
static volatile uint16_t *tonesIndex;
static volatile uint16_t toneSequence[MAX_TONES * 2 + 1];
static volatile bool inProgmem;


ArduboyTones::ArduboyTones(bool (*outEn)())
{
  outputEnabled = outEn;

  toneSequence[MAX_TONES * 2] = TONES_END;

#ifndef ELBEARBOY
	  bitClear(TONE_PIN_PORT, TONE_PIN); // set the pin low
	  bitSet(TONE_PIN_DDR, TONE_PIN); // set the pin to output mode
	#ifdef TONES_2_SPEAKER_PINS
	  bitClear(TONE_PIN2_PORT, TONE_PIN2); // set pin 2 low
	  bitSet(TONE_PIN2_DDR, TONE_PIN2); // set pin 2 to output mode
	#endif
#else	 // ELBEARBOY
	// Timer32_1_ch4, D9= PORT 0.3 
	PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_TIMER32_1_M | PM_CLOCK_APB_P_GPIO_0_M;
	PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M | PM_CLOCK_APB_M_EPIC_M;
	
	PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * TONE_PIN)); // установка вывода 3 порта 0 (в режим 0xb00).  Timer Disconnect!
	GPIO_0->DIRECTION_OUT = 1 << TONE_PIN; //
	GPIO_0->CLEAR = 1 << TONE_PIN;

	TIMER32_1->CHANNELS[3].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;

	TIMER32_1->INT_MASK = TIMER32_INT_OVERFLOW_M;
	TIMER32_1->INT_CLEAR =   0xFFFFFFFF;
	TIMER32_1->PRESCALER =  0; //Divide by 16 clock prescale
	
	//Блок инициализации канала
	
	TIMER32_1->CHANNELS[3].OCR = 0;
	TIMER32_1->CHANNELS[3].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 

	TIMER32_1->CHANNELS[3].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
	
	
	
	#ifdef TONES_2_SPEAKER_PINS
		// // Timer32_2_ch2, D11= PORT 1.1 
		PM->CLK_APB_P_SET |=  PM_CLOCK_APB_P_GPIO_1_M;
		PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * TONE_PIN2)); // установка вывода 3 порта 0 (в режим 0xb00).  Timer Disconnect!
		GPIO_1->DIRECTION_OUT = 1 << TONE_PIN2; //
		GPIO_1->CLEAR = 1 << TONE_PIN2;

	#endif
#endif	
}

void ArduboyTones::tone(uint16_t freq, uint16_t dur)
{
#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  bitWrite(TIMSK1, OCIE1A, 0); // disable the output compare match interrupt
	#else
	  bitWrite(TIMSK3, OCIE3A, 0); // disable the output compare match interrupt
	#endif
#else // ELBEARBOY
	// отключаем прерывания по сравнению. Возможно здесь потребуется перезапуск канала, либо достаточно обнулить OCR или остановить счёт?
		EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
#endif	
	  inProgmem = false;
	  tonesStart = tonesIndex = toneSequence; // set to start of sequence array
	  toneSequence[0] = freq;
	  toneSequence[1] = dur;
	  toneSequence[2] = TONES_END; // set end marker
	  nextTone(); // start playing
}

void ArduboyTones::tone(uint16_t freq1, uint16_t dur1,
                        uint16_t freq2, uint16_t dur2)
{
#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  bitWrite(TIMSK1, OCIE1A, 0); // disable the output compare match interrupt
	#else
	  bitWrite(TIMSK3, OCIE3A, 0); // disable the output compare match interrupt
	#endif
#else // ELBEARBOY
	// отключаем прерывания по сравнению. Возможно здесь потребуется перезапуск канала, либо достаточно обнулить OCR или остановить счёт?

	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
#endif	
  inProgmem = false;
  tonesStart = tonesIndex = toneSequence; // set to start of sequence array
  toneSequence[0] = freq1;
  toneSequence[1] = dur1;
  toneSequence[2] = freq2;
  toneSequence[3] = dur2;
  toneSequence[4] = TONES_END; // set end marker
  nextTone(); // start playing
}

void ArduboyTones::tone(uint16_t freq1, uint16_t dur1,
                        uint16_t freq2, uint16_t dur2,
                        uint16_t freq3, uint16_t dur3)
{
#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  bitWrite(TIMSK1, OCIE1A, 0); // disable the output compare match interrupt
	#else
	  bitWrite(TIMSK3, OCIE3A, 0); // disable the output compare match interrupt
	#endif
#else // ELBEARBOY
	// отключаем прерывания по сравнению. Возможно здесь потребуется перезапуск канала, либо достаточно обнулить OCR или остановить счёт?
 
	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
#endif		
  inProgmem = false;
  tonesStart = tonesIndex = toneSequence; // set to start of sequence array
  toneSequence[0] = freq1;
  toneSequence[1] = dur1;
  toneSequence[2] = freq2;
  toneSequence[3] = dur2;
  toneSequence[4] = freq3;
  toneSequence[5] = dur3;
  // end marker was set in the constructor and will never change
  nextTone(); // start playing
}

void ArduboyTones::tones(const uint16_t *tones)
{
#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  bitWrite(TIMSK1, OCIE1A, 0); // disable the output compare match interrupt
	#else
	  bitWrite(TIMSK3, OCIE3A, 0); // disable the output compare match interrupt
	#endif
#else // ELBEARBOY
	// отключаем прерывания по сравнению. Возможно здесь потребуется перезапуск канала, либо достаточно обнулить OCR или остановить счёт?

	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
#endif		
  inProgmem = true;
  tonesStart = tonesIndex = (uint16_t *)tones; // set to start of sequence array
  nextTone(); // start playing
}

void ArduboyTones::tonesInRAM(uint16_t *tones)
{
#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  bitWrite(TIMSK1, OCIE1A, 0); // disable the output compare match interrupt
	#else
	  bitWrite(TIMSK3, OCIE3A, 0); // disable the output compare match interrupt
	#endif
#else // ELBEARBOY
	// отключаем прерывания по сравнению. Возможно здесь потребуется перезапуск канала, либо достаточно обнулить OCR или остановить счёт?

	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
#endif		
  inProgmem = false;
  tonesStart = tonesIndex = tones; // set to start of sequence array
  nextTone(); // start playing
}

void ArduboyTones::noTone()
{
#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  bitWrite(TIMSK1, OCIE1A, 0); // disable the output compare match interrupt
	  TCCR1B = 0; // stop the counter
	#else
	  bitWrite(TIMSK3, OCIE3A, 0); // disable the output compare match interrupt
	  TCCR3B = 0; // stop the counter
	#endif
    bitClear(TONE_PIN_PORT, TONE_PIN); // set the pin low
	#ifdef TONES_VOLUME_CONTROL
	  bitClear(TONE_PIN2_PORT, TONE_PIN2); // set pin 2 low
	#endif
#else // ELBEARBOY
	//Serial.println("!STOP HERE!");
	// отключаем прерывания по сравнению. Возможно здесь потребуется перезапуск канала, либо достаточно обнулить OCR или остановить счёт?

	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
  	GPIO_0->CLEAR = 1 << TONE_PIN;
	#ifdef TONES_VOLUME_CONTROL
	  //bitClear(TONE_PIN2_PORT, TONE_PIN2); // set pin 2 low
	   GPIO_1->CLEAR = 1 << TONE_PIN2;
	#endif
#endif	
 tonesPlaying = false;
}

void ArduboyTones::volumeMode(uint8_t mode)
{
#ifdef TONES_VOLUME_CONTROL
  forceNormVol = false; // assume volume is tone controlled
  forceHighVol = false;

  if (mode == VOLUME_ALWAYS_NORMAL) {
    forceNormVol = true;
  }
  else if (mode == VOLUME_ALWAYS_HIGH) {
    forceHighVol = true;
  }
#endif
}

bool ArduboyTones::playing()
{
  return tonesPlaying;
}

void ArduboyTones::nextTone()
{
  uint16_t freq;
  uint16_t dur;
  long toggleCount;
  uint32_t ocrValue;
#ifdef TONES_ADJUST_PRESCALER
  uint8_t tccrxbValue;
#endif

  freq = getNext(); // get tone frequency

  if (freq == TONES_END) { // if freq is actually an "end of sequence" marker
    noTone(); // stop playing
    return;
  }

  tonesPlaying = true;

  if (freq == TONES_REPEAT) { // if frequency is actually a "repeat" marker
    tonesIndex = tonesStart; // reset to start of sequence
    freq = getNext();
  }

#ifdef TONES_VOLUME_CONTROL
  if (((freq & TONE_HIGH_VOLUME) || forceHighVol) && !forceNormVol) {
    toneHighVol = true;
  }
  else {
    toneHighVol = false;
  }
#endif

  freq &= ~TONE_HIGH_VOLUME; // strip volume indicator from frequency

#ifndef ELBEARBOY
	#ifdef TONES_ADJUST_PRESCALER
	  if (freq >= MIN_NO_PRESCALE_FREQ) {
		#ifdef ECONSOLE
			tccrxbValue = _BV(WGM12) | _BV(CS10); // CTC mode, no prescaling
		#else
			tccrxbValue = _BV(WGM32) | _BV(CS30); // CTC mode, no prescaling
		#endif
		ocrValue = F_CPU / freq / 2 - 1;
		toneSilent = false;
	  }
	  else {
		#ifdef ECONSOLE
			tccrxbValue = _BV(WGM12) | _BV(CS11); // CTC mode, prescaler /8
		#else
			tccrxbValue = _BV(WGM32) | _BV(CS31); // CTC mode, prescaler /8
		#endif
	#endif
		if (freq == 0) { // if tone is silent
		  ocrValue = F_CPU / 8 / SILENT_FREQ / 2 - 1; // dummy tone for silence
		  freq = SILENT_FREQ;
		  toneSilent = true;
		  bitClear(TONE_PIN_PORT, TONE_PIN); // set the pin low
		}
		else {
		  ocrValue = F_CPU / 8 / freq / 2 - 1;
		  toneSilent = false;
		}
	#ifdef TONES_ADJUST_PRESCALER
	  }
	#endif
#else // ELBEARBOY
	if (freq == 0) { // if tone is silent
      ocrValue = F_CPU / SILENT_FREQ / 2 - 1; // dummy tone for silence
      freq = SILENT_FREQ;
      toneSilent = true;
      GPIO_0->CLEAR = 1 << TONE_PIN; // set the pin low
    }
    else {
      ocrValue = F_CPU / freq / 2 - 1; // счет без делителя
      toneSilent = false;
    }
#endif
  if (!outputEnabled()) { // if sound has been muted
    toneSilent = true;
  }

#ifdef TONES_VOLUME_CONTROL
	if (toneHighVol && !toneSilent) {
    // set pin 2 to the compliment of pin 1
	#ifndef ELBEARBOY
		if (bitRead(TONE_PIN_PORT, TONE_PIN)) { bitClear(TONE_PIN2_PORT, TONE_PIN2);}
		else { bitSet(TONE_PIN2_PORT, TONE_PIN2);}
	}
	else { bitClear(TONE_PIN2_PORT, TONE_PIN2);} // set pin 2 low for normal volume
  	#else // ELBEARBOY
		if (GPIO_0->STATE & (1 << TONE_PIN)) {GPIO_0->CLEAR = 1 << TONE_PIN2;}
		else {GPIO_0->SET = 1 << TONE_PIN2;}
	}
	else {GPIO_1->CLEAR = 1 << TONE_PIN2;} // set pin 2 low for normal volume		
	#endif
#endif

  dur = getNext(); // get tone duration
  if (dur != 0) {
    // A right shift is used to divide by 512 for efficency.
    // For durations in milliseconds it should actually be a divide by 500,
    // so durations will by shorter by 2.34% of what is specified.
    toggleCount = ((long)dur * freq) >> 9;
  }
  else {
    toggleCount = -1; // indicate infinite duration
  }

#ifndef ELBEARBOY
	#ifdef ECONSOLE
	  TCCR1A = 0;
		#ifdef TONES_ADJUST_PRESCALER
		  TCCR1B = tccrxbValue;
		#else
		  TCCR1B = _BV(WGM12) | _BV(CS11); // CTC mode, prescaler /8
		#endif
	  OCR1A = ocrValue;
	  durationToggleCount = toggleCount;
	  bitWrite(TIMSK1, OCIE1A, 1); // enable the output compare match interrupt
	#else
	  TCCR3A = 0;
		#ifdef TONES_ADJUST_PRESCALER
		  TCCR3B = tccrxbValue;
		#else
		  TCCR3B = _BV(WGM32) | _BV(CS31); // CTC mode, prescaler /8
		#endif
	  OCR3A = ocrValue;
	  durationToggleCount = toggleCount;
	  bitWrite(TIMSK3, OCIE3A, 1); // enable the output compare match interrupt
	#endif
#else // ELBEARBOY
	// Timer32_1_ch4, D9= PORT 0.3 
	// выполняем на случай, если audio.on/off переключит в неправильный режим
	PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * TONE_PIN)); // установка вывода 3 порта 0 (в режим 0xb00).  Timer Disconnect!
	GPIO_0->DIRECTION_OUT = 1 << TONE_PIN; //
	#ifdef TONES_VOLUME_CONTROL
	PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * TONE_PIN2)); // установка вывода 1 порта 1 (в режим 0xb00).  Timer Disconnect!
	GPIO_1->DIRECTION_OUT = 1 << TONE_PIN2; //
	#endif
	
	
	
	TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M | ~(TIMER32_ENABLE_TIM_EN_M); // без  этого таймер временно "зависает" при быстрой смене TOP/OCR
	TIMER32_1->TOP = (ocrValue); // счет без делителя, не умножаем на 2
	TIMER32_1->CHANNELS[3].OCR = 0;
	TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M | TIMER32_ENABLE_TIM_EN_M;

	// enable the output compare match interrupt
	durationToggleCount = toggleCount;
	
	EPIC->MASK_LEVEL_SET = HAL_EPIC_TIMER32_1_MASK ;
    
	//HAL_IRQ_EnableInterrupts();
	set_csr(mstatus, MSTATUS_MIE);
    set_csr(mie, MIE_MEIE);
#endif
}

uint16_t ArduboyTones::getNext()
{
  if (inProgmem) {
    return pgm_read_word(tonesIndex++);
  }
  return *tonesIndex++;
}

#ifndef ELBEARBOY
	#ifdef ECONSOLE
	ISR(TIMER1_COMPA_vect)
	#else
	ISR(TIMER3_COMPA_vect)
	#endif
#else // ELBEARBOY
	extern "C" void ISR()
#endif	
{
  long toggleCount = durationToggleCount;
  #ifdef ELBEARBOY
	TIMER32_1->INT_CLEAR =   0xFFFFFFFF;
  #endif 
 //Serial.print(" toggleCount="); Serial.println(toggleCount);
  if (toggleCount != 0) {
    if (!toneSilent) {
	#ifndef ELBEARBOY
			  bitSet(*(&TONE_PIN_PIN), TONE_PIN); // toggle the pin
		#ifdef TONES_VOLUME_CONTROL
			  if (toneHighVol) {
				bitSet(*(&TONE_PIN2_PIN), TONE_PIN2); // toggle pin 2
			  }
		#endif
	#else // ELBEARBOY
		if (GPIO_0->STATE & (1 << TONE_PIN)) {GPIO_0->CLEAR = 1 << TONE_PIN;}
		else {GPIO_0->SET = 1 << TONE_PIN;}
		#ifdef TONES_VOLUME_CONTROL
			if (GPIO_1->STATE & (1 << TONE_PIN2)) {GPIO_1->CLEAR = 1 << TONE_PIN2;}
			else {GPIO_1->SET = 1 << TONE_PIN2;}
		#endif
	#endif
    }
    if (--toggleCount >= 0) {
      durationToggleCount = toggleCount;
    }
  }
  else {
	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK;
	ArduboyTones::nextTone();
  }

}
