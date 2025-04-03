//#include "mik32_hal_i2c.h"
//#include "mik32_hal_rtc.h"
//#include "mik32_hal_ssd1306.h"

//#include "uart_lib.h"
//#include "xprintf.h"
//void bootOLED();
//void bootLogo();

static inline void  i2c_stop() __attribute__((always_inline));
static inline void Delay_us (uint32_t us) __attribute__((always_inline));

//#define SSD1306_128x32
#define ssd1306_128x64
#define __NOP() __asm volatile ("ADDI x0, x0, 0")

//uint16_t slave_address = 0b00111100;
//bool err1;
//uint8_t data_Set[2];

#define HEIGHT 64
#define WIDTH 128
static uint8_t sBuffer[(HEIGHT*WIDTH)/8];

 uint8_t arduboy_logo[] = {
  0xF0, 0xF8, 0x9C, 0x8E, 0x87, 0x83, 0x87, 0x8E, 0x9C, 0xF8,
  0xF0, 0x00, 0x00, 0xFE, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03,
  0x07, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0xFE, 0xFF, 0x03, 0x03,
  0x03, 0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
  0x00, 0x00, 0xFE, 0xFF, 0x83, 0x83, 0x83, 0x83, 0x83, 0xC7,
  0xEE, 0x7C, 0x38, 0x00, 0x00, 0xF8, 0xFC, 0x0E, 0x07, 0x03,
  0x03, 0x03, 0x07, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0x3F, 0x7F,
  0xE0, 0xC0, 0x80, 0x80, 0xC0, 0xE0, 0x7F, 0x3F, 0xFF, 0xFF,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x00,
  0x00, 0xFF, 0xFF, 0x0C, 0x0C, 0x0C, 0x0C, 0x1C, 0x3E, 0x77,
  0xE3, 0xC1, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0,
  0xC0, 0xE0, 0x70, 0x3F, 0x1F, 0x00, 0x00, 0x1F, 0x3F, 0x70,
  0xE0, 0xC0, 0xC0, 0xC0, 0xE0, 0x70, 0x3F, 0x1F, 0x00, 0x00,
  0x7F, 0xFF, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, 0xE3, 0x77, 0x3E,
  0x1C, 0x00, 0x00, 0x1F, 0x3F, 0x70, 0xE0, 0xC0, 0xC0, 0xC0,
  0xE0, 0x70, 0x3F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00
};

#define WHITE 1
#define BLACK 0
#define CLEAR_BUFFER true
#define OLED_SET_PAGE_ADDRESS      0xB0
#define RAND_SEED_IN_ADMUX (_BV(REFS0) | _BV(REFS1) | _BV(MUX0))
# define OLED_CONTRAST 0xCF // 0xCF for high contrast or 0x80 low contrast

#define OLED_SET_COLUMN_ADDRESS_LO 0x02 //SH1106 only: 1st pixel starts on column 2
#define OLED_SET_COLUMN_ADDRESS_HI 0x10
#define OLED_PIXELS_INVERTED 0xA7 // All pixels inverted
#define OLED_PIXELS_NORMAL 0xA6 // All pixels normal

#define OLED_ALL_PIXELS_ON 0xA5 // all pixels on
#define OLED_PIXELS_FROM_RAM 0xA4 // pixels mapped to display RAM contents

#define OLED_VERTICAL_FLIPPED 0xC0 // reversed COM scan direction
#define OLED_VERTICAL_NORMAL 0xC8 // normal COM scan direction

#define OLED_HORIZ_FLIPPED 0xA0 // reversed segment re-map
#define OLED_HORIZ_NORMAL 0xA1 // normal segment re-map
#define OLED_SET_COLUMN_ADDRESS_HI 0x10

#define COLUMN_ADDRESS_END (WIDTH - 1) & 127   // 128 pixels wide
#define PAGE_ADDRESS_END ((HEIGHT/8)-1) & 7    // 8 pages high
		#define SSD1306_I2C_ADDR 0x3c //0x3c:default, 0x3d: alternative)
		#define SSD1306_I2C_CMD  0x00
		#define SSD1306_I2C_DATA 0x40

		#define I2C_PORT  GPIO_1->STATE
		#define I2C_SCL 13 // D19  PORT 1.13
		#define I2C_SDA 12 // D18  PORT 1.12 


		//port directions
#define I2C_SDA_AS_INPUT()  GPIO_1->DIRECTION_IN =  (1 << I2C_SDA)	// установка SDA в 1
#define I2C_SCL_AS_INPUT()  GPIO_1->DIRECTION_IN =  (1 << I2C_SCL) // установка SCL в 1
// установка SDA в 0
#define I2C_SDA_AS_OUTPUT() GPIO_1->DIRECTION_OUT = (1 << I2C_SDA); 
#define I2C_SDA_LOW()   GPIO_1->CLEAR = (1 << I2C_SDA); 

// установка SCL в 0
#define I2C_SCL_AS_OUTPUT()  GPIO_1->DIRECTION_OUT = (1 << I2C_SCL); 
#define I2C_SCL_LOW()    GPIO_1->CLEAR = (1 << I2C_SCL); 

// установка SCL в 1
//#define I2C_SCL_AS_OUTPUT_1() GPIO_1->DIRECTION_OUT = (1 << I2C_SCL); 
//#define I2C_SCL_HIGH()    GPIO_1->SET |= (1 << I2C_SCL); 



void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);

    //UART_Init(UART_0, 3333, UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    i2c1_Init();

    /* Задержка для включения экрана */
    for (volatile int i = 0; i < 1000; i++); 
    
    /* Инициализация */
    //HAL_SSD1306_Init(&hi2c1, BRIGHTNESS_FULL);

#ifdef MIK32V0
Serial.println("MIK32V0");
#else // MIK32V2
Serial.println("MIK32V2");
#endif 

Serial.print("(OSC_SYSTEM_VALUE/500000)/5=");
Serial.println((OSC_SYSTEM_VALUE/500000)/5);

//HAL

bootOLED();
   for (volatile int i = 0; i < 1000; i++); 

bootLogo();

}


void loop()
{    

}


void i2c_start(uint8_t mode)
{
  //I2C_SDA_LOW();       // disable posible internal pullup, ensure SDA low on enabling output
  //I2C_SDA_AS_OUTPUT(); // SDA low before SCL for start condition
  //I2C_SCL_LOW();
  //I2C_SCL_AS_OUTPUT();  
  //i2c_sendByte(SSD1306_I2C_ADDR << 1);

  // условия старта передачи для шины I2C и выбор режима для экрана (передача команд, передача данных)

      // высокий уровень SDA задан после i2c_stop (в т.ч. после инициализации)
      I2C_SDA_LOW();
      I2C_SDA_AS_OUTPUT();  // SDA в 0. Старт-последовательность: провал SDA при высоком уровне SCL
      I2C_SCL_LOW();
      //Delay_us(4);
      I2C_SCL_AS_OUTPUT(); // SLC в 0. синхронизация последовательно снижается 
      //Delay_us(4);
  i2c_sendByte(SSD1306_I2C_ADDR << 1);
  i2c_sendByte(mode);
}




static inline void i2c_stop() 
//static inline void  i2c_stop() __attribute__((always_inline))
{
      // SDA and SCL both are already low, from writing ACK bit no need to change state
      //I2C_SDA_AS_INPUT(); // switch to input so SDA is pulled up externally first for stop condition
      //I2C_SCL_AS_INPUT(); // pull up SCL externally

      // условия стопа передачи для шины I2C 
      //I2C_SDA_AS_OUTPUT(); // SDA в 0. тут порядок вероятно не особо важен. Но предполагаем, что порядок изменения уровней SDA и SCL другой
      //Delay_us(4);
      //I2C_SCL_AS_OUTPUT(); // SLC в 0.
      //Delay_us(4);
      I2C_SCL_AS_INPUT(); // Условия стоп: при высоком уровне SCL
      //Delay_us(4);
      I2C_SDA_AS_INPUT(); // линия SDA переходит в 1
      // тут была проверка ошибок
      //Delay_us(4);
}


void i2c_sendByte(uint8_t byte) {
 uint8_t i;
for (i=0;i<8;i++)
    {
        if (byte & 0x80) 
				{
				I2C_SDA_AS_INPUT();  // лог.1
        }
				else 
				{
				I2C_SDA_LOW();
        I2C_SDA_AS_OUTPUT(); // Выставить бит на SDA (лог.0
				}
        //Delay_us(4);
        I2C_SCL_AS_INPUT();   // Записать его импульсом на SCL       // отпустить SCL (лог.1)
        //Delay_us(4);
        I2C_SCL_LOW();
        I2C_SCL_AS_OUTPUT(); // притянуть SCL (лог.0)
        byte<<=1; // сдвигаем на 1 бит влево
					
    }
    I2C_SDA_AS_INPUT(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK. В оригинальном тексте Arduboy2 тут выставляется лог.0. Вероятно, чтобы не дожидаться, пока это сделает ведомый?
    //Delay_us(4);
    I2C_SCL_AS_INPUT(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK
    //I2C_SCL_AS_OUTPUT_1();
    //Delay_us(4);
    //SDA=SDA_I;
		//if (SDA==0x00) ack=1; else ack=0;    // Считать ACK
    I2C_SCL_LOW();
    I2C_SCL_AS_OUTPUT(); // притянуть SCL (лог.0)  // приём ACK завершён
     //   Delay_us(4);
    //delayMicroseconds(4);
    //return ack; // вернуть ACK (0) или NACK (1)   
}



static void i2c1_Init(void)
{

//PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_UART_0_M | PM_CLOCK_APB_P_GPIO_0_M | PM_CLOCK_APB_P_GPIO_1_M | PM_CLOCK_APB_P_GPIO_2_M; 
PM->CLK_APB_P_SET |= PM_CLOCK_APB_P_GPIO_0_M | PM_CLOCK_APB_P_GPIO_1_M; 
// включение тактирования GPIO. Надо ли тактировать UART0?

PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;  
// включение тактирования блока для смены режима выводов

PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * I2C_SDA)); // Обнуление вывода 12 порта 1 (в режим GPIO)
PAD_CONFIG->PORT_1_CFG &= ~(0b11 << (2 * I2C_SCL)); // Обнуление вывода 13 порта 1 (в режим GPIO)
PAD_CONFIG->PORT_1_PUPD &= ~(0b11 << (2 * I2C_SDA)); // Обнуление. Отключается подтяжка при работе в режиме выхода
PAD_CONFIG->PORT_1_PUPD &= ~(0b11 << (2 * I2C_SCL)); // Обнуление
PAD_CONFIG->PORT_1_DS &= ~(0b11 << (2 * I2C_SDA)); // Обнуление.
PAD_CONFIG->PORT_1_DS &= ~(0b11 << (2 * I2C_SCL)); // Обнуление
PAD_CONFIG->PORT_1_DS |= (0b10 << (2 * I2C_SDA)); // Обнуление.
PAD_CONFIG->PORT_1_DS |= (0b10 << (2 * I2C_SCL)); // Обнуление

    for (volatile int i = 0; i < 1000; i++);
i2c_stop();
//i2c_stop();  //вероятно это дублирование для более стабильного результата?
}





void boot()
{


  //bootPins();
  //bootOLED();
}

void bootLogo()
{
  bootLogoShell(drawLogoBitmap);
}

bool bootLogoShell(void (&drawLogo)(int16_t))
{
    // Using display(CLEAR_BUFFER) instead of clear() may save code space.
    // The extra time it takes to repaint the previous logo isn't an issue.
  for (int16_t y = -15; y <= 24; y++) {
    display(CLEAR_BUFFER);
    (*drawLogo)(y); // call the function that actually draws the logo
    display();
    delayByte(15);
  }
  delayShort(400);
  return true;
}

void delayShort(uint16_t ms)
{
  delay((unsigned long) ms);
}

void delayByte(uint8_t ms)
{
  delayShort(ms);
}


void drawBitmap
(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h,
 uint8_t color = WHITE)
{
  // no need to draw at all if we're offscreen
  if (x + w <= 0 || x > WIDTH - 1 || y + h <= 0 || y > HEIGHT - 1)
    return;

  int8_t yOffset = y & 7;
  int8_t sRow = y >> 3;
  uint8_t rows = (h+7) >> 3;
  for (int a = 0; a < rows; a++) {
    int bRow = sRow + a;
    if (bRow > (HEIGHT/8)-1) break;
    if (bRow > -2) {
      for (int iCol = 0; iCol<w; iCol++) {
        if (iCol + x > (WIDTH-1)) break;
        if (iCol + x >= 0) {
          uint16_t data = bitmap [(a*w)+iCol] << yOffset;
          if (bRow >= 0) {
            if (color == WHITE)
              sBuffer[(bRow*WIDTH) + x + iCol] |= data;
            else if (color == BLACK)
              sBuffer[(bRow*WIDTH) + x + iCol] &= ~data;
            else
              sBuffer[(bRow*WIDTH) + x + iCol] ^= data;
          }
          if (yOffset && bRow<(HEIGHT/8)-1 && bRow > -2) {
            if (color == WHITE)
              sBuffer[((bRow+1)*WIDTH) + x + iCol] |= (data >> 8);
            else if (color == BLACK)
              sBuffer[((bRow+1)*WIDTH) + x + iCol] &= ~(data >> 8);
            else
              sBuffer[((bRow+1)*WIDTH) + x + iCol] ^= (data >> 8);
          }
        }
      }
    }
  }
}

void drawLogoBitmap(int16_t y)
{
  drawBitmap(20 - (64 - WIDTH / 2), y, arduboy_logo, 88, 16);
}


void display()
{
paintScreen(sBuffer);


}


void display(bool clear)
{
  paintScreen(sBuffer, clear);
}

void paintScreen(const uint8_t *image)
{
//elif  defined (OLED_SH1106_I2C)
  for (int page = 0; page < HEIGHT/8; page++)
  {
    i2c_start(SSD1306_I2C_CMD);
    i2c_sendByte(OLED_SET_PAGE_ADDRESS + page); // set page
    i2c_sendByte(OLED_SET_COLUMN_ADDRESS_HI);   // only reset hi nibble to zero
    i2c_stop();
    const uint8_t *line = image + page*WIDTH;
    i2c_start(SSD1306_I2C_DATA);
    for (int i = 0; i < WIDTH; i++)
      i2c_sendByte(line[i]);
    i2c_stop();
  }
}

void paintScreen(uint8_t image[], bool clear)
{
//elif  defined (OLED_SH1106_I2C)
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
}

uint8_t lcdBootProgram[] = {
  // boot defaults are commented out but left here in case they
  // might prove useful for reference
  //
  // Further reading: https://www.adafruit.com/datasheets/SSD1306.pdf
  
//elif defined(OLED_SH1106) || defined(OLED_SH1106_I2C)
  0x8D, 0x14,                   // Charge Pump Setting v = enable (0x14)
  0xA1,                         // Set Segment Re-map
  0xC8,                         // Set COM Output Scan Direction
  0x81, OLED_CONTRAST,          // Set Contrast v = 0xCF
  0xD9, 0xF1,                   // Set Precharge = 0xF1
  OLED_SET_COLUMN_ADDRESS_LO,   //Set column address for left most pixel
  0xAF                          // Display On
};

void bootOLED()
{

for (uint8_t i = 0; i < sizeof(lcdBootProgram); i++){
    i2c_start(SSD1306_I2C_CMD);
    i2c_sendByte(lcdBootProgram[i]);
//    i2c_stop();
}
i2c_stop();


}


static void inline Delay_us (uint32_t us) //Функция задержки в микросекундах us
{
        int long i;
      for (i=0;i<us;i++)
      {
       i++;
       i--;
      }

}

void Delay_us2(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}
