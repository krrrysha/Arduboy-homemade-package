#include "analog_reg.h"
//include "analog_reg_my.h"
//include "mik32_hal_adc.h"

unsigned int JoystickXZero=5000;
unsigned int JoystickYZero=5000;
uint8_t 	ADCJoystickState;
int valA0;
int valA1;
bool valD3;
bool valD4;
//ADC_HandleTypeDef hadc;

#define ADC_CONFIG_SAH_TIME_MY          (0x3F << ADC_CONFIG_SAH_TIME_S)
#define myADC_SEL_CHANNEL(channel_selection) (ANALOG_REG->ADC_CONFIG = ((ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_MY)) & (~ADC_CONFIG_SEL_M)) | ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_MY) | ((channel_selection) << ADC_CONFIG_SEL_S))

#define ADC_EXTREF_OFF      0       /* Встроенный источник опорного напряжения 1,2 В */
#define ADC_EXTREF_ON       1       /* Внешний источник опорного напряжения */
#define ADC_EXTCLB_CLBREF      0       /* Настраиваемый ОИН */
#define ADC_EXTCLB_ADCREF      1       /* Внешний вывод */

int count = 10000;
uint32_t time_mid;
uint32_t time;
#define JOYSENSY 200
#define JOYSENSX 200

#define LEFT_BUTTON _BV(5)  /*< The Left button value for functions requiring a bitmask */
#define RIGHT_BUTTON _BV(6) /*< The Right button value for functions requiring a bitmask */
#define UP_BUTTON _BV(7)    /*< The Up button value for functions requiring a bitmask */
#define DOWN_BUTTON _BV(4)  /*< The Down button value for functions requiring a bitmask */
#define A_BUTTON _BV(3)     /*< The A button value for functions requiring a bitmask */
#define B_BUTTON _BV(2)     /*< The B button value for functions requiring a bitmask */

#define PIN_BUTTON_B 0 // Кнопка 3/0 port_0_0
#define PIN_BUTTON_A 8 // Кнопка 4/8 port_0_8
#define PIN_AXISX 7 // Ось X port_1_5
#define PIN_AXISY 5 // Ось Y port_1_7
#define PIN_RANDOM 4 // Ось Y port_0_9

#define CHAN_AXISX 1 // Ось X port_1_5
#define CHAN_AXISY 0 // Ось Y port_1_7
#define CHAN_RANDOM 0 // Ось Y port_0_4

uint8_t chan_converted = 0;
uint8_t chan_selected = 0;  


void setup() {

Serial.begin(9600);

GPIO_0->DIRECTION_IN = 1 << PIN_BUTTON_A; // Установка направления вывода A  на вход
GPIO_0->DIRECTION_IN = 1 << PIN_BUTTON_B; // Установка направления вывода B  на вход
GPIO_0->DIRECTION_IN = 1 << CHAN_RANDOM; // 
GPIO_1->DIRECTION_IN = 1 << CHAN_AXISX; // 
GPIO_1->DIRECTION_IN = 1 << CHAN_AXISY; // 

// ADC init!
chan_selected=0;
PAD_CONFIG->PORT_1_CFG |= (0b11 << (2 * PIN_AXISX)); // аналоговый сигнал. порт A0=1.5
PAD_CONFIG->PORT_1_CFG |= (0b11 << (2 * PIN_AXISY)); // аналоговый сигнал. порт A1=1.7
PAD_CONFIG->PORT_0_CFG |= (0b11 << (2 * PIN_RANDOM)); // аналоговый сигнал. порт A2=0.4
//PAD_CONFIG->PORT_0_CFG &= (0b00 << (2 * PIN_BUTTON_B)); // дискрет . порт D3=0.0
//PAD_CONFIG->PORT_0_CFG &= (0b00 << (2 * PIN_BUTTON_A)); // дискрет . порт D4=0.8

  PM->CLK_APB_P_SET = PM_CLOCK_APB_P_ANALOG_REGS_M;



ANALOG_REG->ADC_CONFIG = 0x3c00;

//HAL_ADC_Enable(&hadc);
ANALOG_REG->ADC_CONFIG = (ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_MY)) |
                                 ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_MY) |
                                 (1 << ADC_CONFIG_EN_S);
//HAL_ADC_ResetEnable:
ANALOG_REG->ADC_CONFIG = (ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_MY)) |
                                 ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_MY) |
                                 (1 << ADC_CONFIG_RESETN_S);
 
// HAL_ADC_ChannelSet
myADC_SEL_CHANNEL (chan_selected);

ANALOG_REG->ADC_CONFIG |= (ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_MY)) |
                                 ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_MY) |
                                 (ADC_EXTREF_OFF << ADC_CONFIG_EXTREF_S) |   // Настройка источника опорного напряжения 
                                 (ADC_EXTCLB_ADCREF << ADC_CONFIG_EXTPAD_EN_S); // Выбор внешнего источника опорного напряжения 

 
  chan_converted=chan_selected;
  //if (chan_selected!=2) 
  //{
  ANALOG_REG->ADC_SINGLE=1; //считаем хз что
  myADC_SEL_CHANNEL(chan_selected); //переключаемся на канал для рандомизации
  //}
  //hadc.Init.Sel=CHAN_AXISX; // ЭТА И СЛЕДУЮАЩАЯ СТРОКА МОЖЕТ УСКОРИТЬ ОБРАБОТКУ КЛАВИШЬ ПОСЛЕ ВЫЗОВА РАНДОМА. ВАЖНО ПОНИМАТ, МОЖЕТ ЛИ РАНДОМ ИСПОЛЬЗОВАТСЯ НЕСКОЛЬКО РАЗ ПОДРЯД
while (!ANALOG_REG->ADC_VALID) {};
  ANALOG_REG->ADC_SINGLE=1; //считаем новый рандом

  //ADC_SEL_CHANNEL(hadc.Instance, CHAN_AXISX); //переключаемся на канал клавишwhile (~ANALOG_REG->!ADC_VALID) {};

}
void loop() {
uint8_t curbuttons;
// analogRead: 8096 байта+768 байта, 467 мкс. | analogread_без_SerialPrint:7344/768 | cubuttons: 5808, 676, 2 мкс | curbuttons без serialprint: 4960/676
time=micros(); 
for (int i=0;i<count; i++)
{
 
curbuttons= buttonsState();
/*
valA0=analogRead(A0);
valA1=analogRead(A1);
valD3=digitalRead(3);
valD4=digitalRead(4);
*/
}
time=(micros()-time);
time_mid=time/count;


Serial.print("cur: ");
Serial.print(curbuttons,BIN);
Serial.println();
/*
Serial.print(" valA0:");
Serial.print(valA0);
Serial.print(" valA1:");
Serial.print(valA1);
Serial.print(" valD3:");
Serial.print(valD3);
Serial.print(" valD4:");
Serial.print(valD4);
*/
Serial.print(" time_mid:");
Serial.println(time_mid);


delay(500); // пауза перед следующим считыванием данных
}


uint8_t buttonsState()
{
 uint8_t 	buttons = 0;
  
  
  if (ANALOG_REG->ADC_VALID) {

  if ((chan_converted == CHAN_AXISX | chan_converted==CHAN_AXISY ) & (chan_selected==CHAN_AXISY|chan_selected==CHAN_AXISX))  {  // последний заданный канал и рассчитанный канал- один из наших
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
    ANALOG_REG->ADC_SINGLE=1;
    myADC_SEL_CHANNEL (chan_selected);
  } else // если не наш канал
  {
    chan_converted=chan_selected;
    chan_selected=CHAN_AXISX;
    ANALOG_REG->ADC_SINGLE=1;
    myADC_SEL_CHANNEL (chan_selected);
  }
/*
    Serial.print(" chan_conv:");
    Serial.print(chan_converted);
    Serial.print(" chan_sel:");
    Serial.print(chan_selected);
 Serial.print(" ADC_VALUE:");
 Serial.print(ANALOG_REG->ADC_VALUE);
    Serial.print(" JoystickXZero:");
 Serial.print(JoystickXZero);
     Serial.print(" JoystickYZero:");
 Serial.print(JoystickYZero);
     Serial.println();
     */

} 


    buttons |= ADCJoystickState;
  //Serial.print(" buttons:");
  //Serial.print(buttons,BIN);

if (not(GPIO_0->STATE & (1 << PIN_BUTTON_A))) { buttons |= A_BUTTON; };

if (not(GPIO_0->STATE & (1 << PIN_BUTTON_B))) { buttons |= B_BUTTON; };

  return buttons;
} 


