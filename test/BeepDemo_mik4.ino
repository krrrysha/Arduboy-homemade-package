/*
BeepDemo.ino

This sketch provides an example of using the Arduboy2 library's BeepPin1 class
to play simple tones.
*/

/*
Written in 2018 by Scott Allen saydisp-git@yahoo.ca

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

// Comments are only provided for code dealing with tone generation or control.

//include <Arduboy2.h>
// There's no need to #include <Arduboy2Beep.h>
// It will be included in Arduboy2.h

//Arduboy2 arduboy;

 // Create a class instance for speaker pin 1
//BeepPin2 beep; // For speaker pin 2, use this line instead of the line above

//#include <mik32_hal_timer32.h>

uint8_t SPEAKER_1_PIN=3;
bool audio_enabled;


void setup() {
Serial.begin(9600);

Arduboy2Audio_on();

}

void loop() {
BeepPin1_begin();

delay(1000);
BeepPin1_timer();
delay(1000);
BeepPin1_tone(freq(30000), 200); 
delay(50);
BeepPin1_noTone(); // Stop the tone
delay(1000);


}


//=========== beep
// PeriphClkInit.PMClockAPB_P =  PM_CLOCK_TIMER16_0_M; PM_CLOCK_TIMER16_1_M APB_P

//PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_TIMER32_1_M | PM_CLOCK_APB_P_TIMER32_1_M;

// D11= PORT 1.1 Timer32_2_ch2
  static constexpr uint16_t freq(const float hz)
  {
    
        return (uint16_t) ((F_CPU/32/hz)-(1/2)) ; 
      //return (uint16_t) (((F_CPU/32)+hz/2)/hz-1) ; 

  }

  static constexpr uint16_t freq2(const float hz)
  {
     return (uint16_t) (F_CPU/32/hz-1) ; 
      //return (uint16_t) (((F_CPU/32)+hz/2)/hz-1) ; 

  }


uint8_t BeepPin1_duration = 0;


void BeepPin1_begin()
{

`
PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
TIMER32_1->CHANNELS[3].CNTRL &=  TIMER32_CH_CNTRL_DISABLE_M;
TIMER32_1->CHANNELS[3].CNTRL |=  TIMER32_CH_CNTRL_MODE_PWM_M; // 
TIMER32_1->PRESCALER =  0; //Divide by 16 clock prescale
TIMER32_1->INT_MASK =  0;
TIMER32_1->INT_CLEAR =   0xFFFFFFFF;
TIMER32_1->CHANNELS[3].OCR = 0;
TIMER32_1->CHANNELS[3].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;
}

void BeepPin1_tone(uint16_t count)
{
  tone(count, 0);
}

void BeepPin1_tone(uint16_t count, uint8_t dur)
{
BeepPin1_duration = dur;
//TIMER32_1->CHANNELS[3].CNTRL |= TIMER32_CH_CNTRL_ENABLE_M;

//TIMER32_1->TOP = count*65535-1;
//TIMER32_1->CHANNELS[3].OCR = TIMER32_1->TOP>>1;
//TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M;
//TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_EN_M;



//TIMER32_1->TOP = 1000;
//TIMER32_1->CHANNELS[3].OCR = TIMER32_1->TOP>>1;
TIMER32_1->TOP = (count*32-1);

Serial.print("count=");
Serial.println(count);
Serial.print("TOP=");
Serial.println(TIMER32_1->TOP);

TIMER32_1->CHANNELS[3].OCR = TIMER32_1->TOP>>1;
TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_CLR_M;
TIMER32_1->ENABLE = TIMER32_ENABLE_TIM_EN_M;


}

void BeepPin1_timer()
{
  if (BeepPin1_duration && (--BeepPin1_duration == 0)) {
TIMER32_1->CHANNELS[3].OCR = 0;
//PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * SPEAKER_1_PIN)); // установка вывода 3 порта 0 (в режим 0xb00).  
  }
}

void BeepPin1_noTone()
{
  BeepPin1_duration = 0;
//PAD_CONFIG->PORT_0_CFG &= ~(0b11 << (2 * SPEAKER_1_PIN)); // установка вывода 3 порта 0 (в режим 0xb00).  
TIMER32_1->CHANNELS[3].OCR = 0;
}

void delayShort(uint16_t ms)
{
delay(ms);
}

void Arduboy2Audio_on()
{
PAD_CONFIG->PORT_0_CFG |= (0b10 << (2 * SPEAKER_1_PIN)); // установка вывода 3 порта 0 (в режим 0xb10).  Connect!
audio_enabled = true;
}


