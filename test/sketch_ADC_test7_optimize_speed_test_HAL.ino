#include "analog_reg.h"
//include "analog_reg_my.h"
#include "mik32_hal_adc.h"
//include "mik32_hal_gpio.h"
//include "mik32_hal.h"

unsigned int JoystickXZero=5000;
unsigned int JoystickYZero=5000;
uint8_t 	ADCJoystickState;
int valA0;
int valA1;
bool valD3;
bool valD4;

ADC_HandleTypeDef hadc;
//GPIO_InitTypeDef GPIO_InitStruct;

//define ADC_CONFIG_SAH_TIME_MY          (0x3F << ADC_CONFIG_SAH_TIME_S)
//define myADC_SEL_CHANNEL(channel_selection) (ANALOG_REG->ADC_CONFIG = ((ANALOG_REG->ADC_CONFIG & (~ADC_CONFIG_SAH_TIME_MY)) & (~ADC_CONFIG_SEL_M)) | ((ANALOG_REG->ADC_CONFIG >> 1) & ADC_CONFIG_SAH_TIME_MY) | ((channel_selection) << ADC_CONFIG_SEL_S))

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
//uint8_t hadc.Init.Sel = 0;  


void setup() {

Serial.begin(9600);

/*
GPIO_InitStruct.Pin = GPIO_PIN_0; //канал 0.0 B
GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_INPUT;
GPIO_InitStruct.Pull = HAL_GPIO_PULL_DOWN;
GPIO_InitStruct.DS = HAL_GPIO_DS_2MA;
HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);

GPIO_InitStruct.Pin = GPIO_PIN_8; //канал 0.8 A
GPIO_InitStruct.Mode = HAL_GPIO_MODE_GPIO_INPUT;
GPIO_InitStruct.Pull = HAL_GPIO_PULL_DOWN;
GPIO_InitStruct.DS = HAL_GPIO_DS_2MA;
HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);


GPIO_InitStruct.Pin = GPIO_PIN_5; //канал ADC0
GPIO_InitStruct.Mode = HAL_GPIO_MODE_ANALOG;
GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
GPIO_InitStruct.DS = HAL_GPIO_DS_2MA;
HAL_GPIO_Init(GPIO_1, &GPIO_InitStruct);

GPIO_InitStruct.Pin = GPIO_PIN_7; //канал ADC1
GPIO_InitStruct.Mode = HAL_GPIO_MODE_ANALOG;
GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
GPIO_InitStruct.DS = HAL_GPIO_DS_2MA;
HAL_GPIO_Init(GPIO_1, &GPIO_InitStruct);

GPIO_InitStruct.Pin = GPIO_PIN_4; //канал ADC3
GPIO_InitStruct.Mode = HAL_GPIO_MODE_ANALOG;
GPIO_InitStruct.Pull = HAL_GPIO_PULL_NONE;
GPIO_InitStruct.DS = HAL_GPIO_DS_2MA;
HAL_GPIO_Init(GPIO_0, &GPIO_InitStruct);
*/

GPIO_0->DIRECTION_IN = 1 << PIN_BUTTON_A; // Установка направления вывода A  на вход
GPIO_0->DIRECTION_IN = 1 << PIN_BUTTON_B; // Установка направления вывода B  на вход
GPIO_0->DIRECTION_IN = 1 << CHAN_RANDOM; // 
GPIO_1->DIRECTION_IN = 1 << CHAN_AXISX; // 
GPIO_1->DIRECTION_IN = 1 << CHAN_AXISY; // 
PAD_CONFIG->PORT_1_CFG |= (0b11 << (2 * PIN_AXISX)); // аналоговый сигнал. порт A0=1.5
PAD_CONFIG->PORT_1_CFG |= (0b11 << (2 * PIN_AXISY)); // аналоговый сигнал. порт A1=1.7
PAD_CONFIG->PORT_0_CFG |= (0b11 << (2 * PIN_RANDOM)); // аналоговый сигнал. порт A2=0.4
// ADC init!
hadc.Init.Sel=0;

hadc.Instance = ANALOG_REG;
hadc.Init.EXTRef = ADC_EXTREF_OFF; /* Выбор источника опорного напряжения: «1» - внешний; «0» - встроенный */
hadc.Init.EXTClb = ADC_EXTCLB_ADCREF; /* Выбор источника внешнего опорного напряжения: «1» - внешний вывод; «0» - настраиваемый ОИН */

HAL_ADC_Init(&hadc);
 
  chan_converted=hadc.Init.Sel;
  //if (hadc.Init.Sel!=2) 
  //{
  
  HAL_ADC_Single(&hadc); //считаем хз что

  HAL_ADC_ChannelSet(&hadc); //переключаемся на канал для рандомизации
  //}
  //hadc.Init.Sel=CHAN_AXISX; // ЭТА И СЛЕДУЮАЩАЯ СТРОКА МОЖЕТ УСКОРИТЬ ОБРАБОТКУ КЛАВИШЬ ПОСЛЕ ВЫЗОВА РАНДОМА. ВАЖНО ПОНИМАТ, МОЖЕТ ЛИ РАНДОМ ИСПОЛЬЗОВАТСЯ НЕСКОЛЬКО РАЗ ПОДРЯД
while (!ANALOG_REG->ADC_VALID) {};
  HAL_ADC_Single(&hadc); //считаем новый рандом

  //ADC_SEL_CHANNEL(hadc.Instance, CHAN_AXISX); //переключаемся на канал клавишwhile (~ANALOG_REG->!ADC_VALID) {};

}
void loop() {
uint8_t curbuttons;
//  cubuttons: 6144, 684, 6 мкс | 
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

  if ((chan_converted == CHAN_AXISX | chan_converted==CHAN_AXISY ) & (hadc.Init.Sel==CHAN_AXISY|hadc.Init.Sel==CHAN_AXISX))  {  // последний заданный канал и рассчитанный канал- один из наших
    unsigned int ADCdata=ANALOG_REG->ADC_VALUE;; // данные от прошлого расчета, возможно на канале за пределами используемых

    if (chan_converted ==CHAN_AXISX ) { // if the conversion at the AC0 input is complete
	 ADCJoystickState &= ~(RIGHT_BUTTON | LEFT_BUTTON);
	 if (JoystickXZero>4096) {JoystickXZero=ADCdata;} // if first run
	 
   if (ADCdata > JoystickXZero+JOYSENSX) {ADCJoystickState |= RIGHT_BUTTON;} else if (ADCdata < JoystickXZero-JOYSENSX) {ADCJoystickState |= LEFT_BUTTON;} // we determine the direction along the X axis

    hadc.Init.Sel=CHAN_AXISX;
    chan_converted=CHAN_AXISY;
	  } else if (chan_converted ==CHAN_AXISY)   // if the conversion at the AC1 input is complete 
	  { 
		ADCJoystickState &= ~(UP_BUTTON | DOWN_BUTTON);
		if (JoystickYZero>4096) {JoystickYZero=ADCdata;} // if first run
		if (ADCdata > JoystickYZero+JOYSENSY) {ADCJoystickState |= UP_BUTTON;} else if (ADCdata < JoystickYZero-JOYSENSY) {ADCJoystickState |= DOWN_BUTTON;} // we determine the direction along the Y axis
    hadc.Init.Sel=CHAN_AXISY;
    chan_converted=CHAN_AXISX;
    } 
    HAL_ADC_Single(&hadc);
  ADC_SEL_CHANNEL (ANALOG_REG,hadc.Init.Sel); 
  } else // если не наш канал
  {
    chan_converted=hadc.Init.Sel;
    HAL_ADC_Single(&hadc);
 ADC_SEL_CHANNEL (ANALOG_REG,hadc.Init.Sel); 
  }
/*
    Serial.print(" chan_conv:");
    Serial.print(chan_converted);
    Serial.print(" chan_sel:");
    Serial.print(hadc.Init.Sel);
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


