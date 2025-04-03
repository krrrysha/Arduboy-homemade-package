#include "mik32_hal_timer32.h"
//#include "Serial.print.h"
#include "uart_lib.h"
//#include "stdlib.h"
TIMER32_HandleTypeDef htimer32_1;
TIMER32_CHANNEL_HandleTypeDef htimer32_channel3;
void SystemClock_Config(void);
static void Timer32_1_Init(void);
void GPIO_Init();


void setup(){
  Serial.begin(9600);
}

void loop()
{
    SystemClock_Config();
    GPIO_Init();
    UART_Init(UART_0, (32000000/9600), UART_CONTROL1_TE_M | UART_CONTROL1_M_8BIT_M | UART_CONTROL1_RE_M, 0, 0);
    Serial.print("\r\nTimer32 PWM test run\r\n");
    Timer32_1_Init();
    HAL_Timer32_Channel_Enable(&htimer32_channel3);
    HAL_Timer32_Value_Clear(&htimer32_1);
    HAL_Timer32_Start(&htimer32_1);
    // 
    Serial.print("top = %d\r\n");
    Serial.println(1055);
    // смена ШИМ
    HAL_Timer32_Top_Set(&htimer32_1, 1055);
    HAL_Timer32_Channel_OCR_Set(&htimer32_channel3, 1055 >> 1);
    HAL_Timer32_Value_Clear(&htimer32_1);
    while (1)
    {

    }
}
void SystemClock_Config(void)
{
    PCC_InitTypeDef PCC_OscInit = {0};
    PCC_OscInit.OscillatorEnable = PCC_OSCILLATORTYPE_ALL;
    PCC_OscInit.FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M;
    //PCC_OscInit.FreqMon.ForceOscSys = PCC_FORCE_OSC_SYS_UNFIXED;
    PCC_OscInit.FreqMon.ForceOscSys=PCC_FORCE_OSC_SYS_FIXED;
    PCC_OscInit.FreqMon.Force32KClk = PCC_FREQ_MONITOR_SOURCE_OSC32K;
    PCC_OscInit.AHBDivider = 0;
    PCC_OscInit.APBMDivider = 0;
    PCC_OscInit.APBPDivider = 0;
    PCC_OscInit.HSI32MCalibrationValue = 128;
    PCC_OscInit.LSI32KCalibrationValue = 128;
    PCC_OscInit.RTCClockSelection = PCC_RTC_CLOCK_SOURCE_AUTO;
    PCC_OscInit.RTCClockCPUSelection = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K;
    HAL_PCC_Config(&PCC_OscInit);
    
    //PM->CLK_APB_P_SET = PM_CLOCK_APB_P_TIMER32_1_M | PM_CLOCK_APB_P_GPIO_0_M;
    //PM->CLK_APB_M_SET |= PM_CLOCK_APB_M_PAD_CONFIG_M | PM_CLOCK_APB_M_WU_M | PM_CLOCK_APB_M_PM_M;
}
static void Timer32_1_Init(void)
{
    htimer32_1.Instance = TIMER32_1;
    htimer32_1.Top = 7543;
    // htimer32_1.Top = 49;
    htimer32_1.State = TIMER32_STATE_DISABLE;
    htimer32_1.Clock.Source = TIMER32_SOURCE_PRESCALER;
    //htimer32_1.Clock.Source = TIMER32_SOURCE_TIM1_HCLK;
    htimer32_1.Clock.Prescaler = 0;
    htimer32_1.InterruptMask = 0;
    htimer32_1.CountMode = TIMER32_COUNTMODE_FORWARD;
    HAL_Timer32_Init(&htimer32_1);
    htimer32_channel3.TimerInstance = htimer32_1.Instance;
    htimer32_channel3.ChannelIndex = TIMER32_CHANNEL_3;
    htimer32_channel3.PWM_Invert = TIMER32_CHANNEL_NON_INVERTED_PWM;
    htimer32_channel3.Mode = TIMER32_CHANNEL_MODE_PWM;
    htimer32_channel3.CaptureEdge = TIMER32_CHANNEL_CAPTUREEDGE_RISING;
    htimer32_channel3.OCR = 7544 >> 1;
    // htimer32_channel0.OCR = 50 >> 1;
    htimer32_channel3.Noise = TIMER32_CHANNEL_FILTER_OFF;
    HAL_Timer32_Channel_Init(&htimer32_channel3);
}
void GPIO_Init()
{
PAD_CONFIG->PORT_0_CFG |= (0b10 << (2 * 3));
    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();
    __HAL_PCC_GPIO_2_CLK_ENABLE();
}