#include "mik32_hal_i2c.h"
#include "mik32_hal_rtc.h"
#include "mik32_hal_ssd1306.h"

#include "uart_lib.h"
#include "xprintf.h"
void bootOLED();
void bootLogo();

//#define SSD1306_128x32
#define ssd1306_128x64

//uint16_t slave_address = 0b00111100;
bool err1;
uint8_t data_Set[2];
uint16_t slave_address1 = 0b00111100;
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


I2C_HandleTypeDef hi2c1;
RTC_HandleTypeDef hrtc;

RTC_TimeTypeDef LastTime = {0};
RTC_TimeTypeDef CurrentTime = {0};

//void SystemClock_Config(void);
static void I2C1_Init(void);
//static void RTC_Init(void);

void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);

    UART_Init(UART_0, 3333, UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M, 0, 0);

    I2C1_Init();

    /* Задержка для включения экрана */
    for (volatile int i = 0; i < 1000; i++); 
    
    /* Инициализация */
    //HAL_SSD1306_Init(&hi2c1, BRIGHTNESS_FULL);

//HAL

bootOLED();
   for (volatile int i = 0; i < 1000; i++); 

bootLogo();

}


void loop()
{    





   
    
}

/*
void SystemClock_Config(void)
{
    PCC_InitTypeDef PCC_OscInit = {0};

    PCC_OscInit.OscillatorEnable = PCC_OSCILLATORTYPE_ALL;
    PCC_OscInit.FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M;
    PCC_OscInit.FreqMon.ForceOscSys = PCC_FORCE_OSC_SYS_UNFIXED;
    PCC_OscInit.FreqMon.Force32KClk = PCC_FREQ_MONITOR_SOURCE_OSC32K;
    PCC_OscInit.AHBDivider = 0;
    PCC_OscInit.APBMDivider = 0;
    PCC_OscInit.APBPDivider = 0;
    PCC_OscInit.HSI32MCalibrationValue = 128;
    PCC_OscInit.LSI32KCalibrationValue = 8;
    PCC_OscInit.RTCClockSelection = PCC_RTC_CLOCK_SOURCE_AUTO;
    PCC_OscInit.RTCClockCPUSelection = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K;
    HAL_PCC_Config(&PCC_OscInit);
}
*/

static void I2C1_Init(void)
{
    /* Общие настройки */
    hi2c1.Instance = I2C_1;

    hi2c1.Init.Mode = HAL_I2C_MODE_MASTER;

    hi2c1.Init.DigitalFilter = I2C_DIGITALFILTER_OFF;
    hi2c1.Init.AnalogFilter = I2C_ANALOGFILTER_DISABLE;
    hi2c1.Init.AutoEnd = I2C_AUTOEND_ENABLE;

    /* Настройка частоты */
    hi2c1.Clock.PRESC = 5;
    hi2c1.Clock.SCLDEL = 10;
    hi2c1.Clock.SDADEL = 10;
    hi2c1.Clock.SCLH = 16;
    hi2c1.Clock.SCLL = 16;


    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        xprintf("I2C_Init error\n");
    }

}

/*
static void RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    __HAL_PCC_RTC_CLK_ENABLE();

    hrtc.Instance = RTC;

    // Выключение RTC для записи даты и времени/
    HAL_RTC_Disable(&hrtc);

    // Установка даты и времени RTC /
    sTime.Dow       = RTC_WEEKDAY_FRIDAY;
    sTime.Hours     = 12;
    sTime.Minutes   = 0;
    sTime.Seconds   = 0;
    HAL_RTC_SetTime(&hrtc, &sTime);

    sDate.Century   = 21;
    sDate.Day       = 22;
    sDate.Month     = RTC_MONTH_JUNE;
    sDate.Year      = 23;
    HAL_RTC_SetDate(&hrtc, &sDate);

    HAL_RTC_Enable(&hrtc);


}
*/



void i2c_start(uint8_t mode)
{
  //I2C_SDA_LOW();       // disable posible internal pullup, ensure SDA low on enabling output
  //I2C_SDA_AS_OUTPUT(); // SDA low before SCL for start condition
  //I2C_SCL_LOW();
  //I2C_SCL_AS_OUTPUT();  
  //i2c_sendByte(SSD1306_I2C_ADDR << 1);
    switch (mode)
    {
    case SSD1306_I2C_CMD:
      data_Set[0]=SSD1306_I2C_CMD; 
      break;
    case  SSD1306_I2C_DATA:
      data_Set[0]=SSD1306_I2C_DATA;         
      break;
    default:
           //ничего не делать     
      break;
    }
}


static void inline i2c_stop() 
{
      // SDA and SCL both are already low, from writing ACK bit no need to change state
      //I2C_SDA_AS_INPUT(); // switch to input so SDA is pulled up externally first for stop condition
      //I2C_SCL_AS_INPUT(); // pull up SCL externally
}


void i2c_sendByte(uint8_t byte) {
      data_Set[1] = byte;
    //i2c_sendByte(pgm_read_byte(lcdBootProgram + i));
      err1= HAL_I2C_Master_Transmit(&hi2c1, slave_address1, data_Set, sizeof(data_Set), I2C_TIMEOUT_DEFAULT);


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
    //err1= HAL_I2C_Master_Transmit(&hi2c1, slave_address1, data_Set, sizeof(data_Set), I2C_TIMEOUT_DEFAULT);
}
i2c_stop();


}

