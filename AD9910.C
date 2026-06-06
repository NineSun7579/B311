/**********************************************************
                       Ported to STM32H7 HAL

功能：STM32H750VBT6控制AD9910
接口：请参照AD9910.H引脚定义
版本：1.0 (H7 HAL移植版)

移植内容：
- stm32f4xx.h -> stm32h7xx_hal.h
- delay_ms()  -> HAL_Delay()
- 标准库GPIO  -> HAL GPIO / BSRR快速操作
- 移除 sys.h / delay.h 依赖
**********************************************************/

#include "stm32h7xx_hal.h"
#include "AD9910.h"

const uchar cfr1[] = {0x00, 0x40, 0x00, 0x00};  //cfr1控制字
const uchar cfr2[] = {0x01, 0x00, 0x00, 0x00};  //cfr2控制字
const uchar cfr3[] = {0x05, 0x0F, 0x41, 0x32};  //cfr3控制字  40M输入  25倍频  VC0=101   ICP=001;
uchar profile11[] = {0x3f, 0xff, 0x00, 0x00, 0x25, 0x09, 0x7b, 0x42}; //profile1控制字
//01振幅控制 23相位控制 4567频率调谐字

void AD9910_IOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // PA0,PA1,PA4,PA7,PA11,PA12 -> 推挽输出
    GPIO_InitStruct.Pin   = AD9910_PWR_PIN | AD9910_SDIO_PIN | DRHOLD_PIN |
                            UP_DAT_PIN | PROFILE1_PIN | MAS_REST_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA5 -> 输入 (DROVER)
    GPIO_InitStruct.Pin  = DROVER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PB4,PB7,PB8,PB9,PB12,PB13 -> 推挽输出
    GPIO_InitStruct.Pin   = DRCTL_PIN | OSK_PIN | PROFILE0_PIN | PROFILE2_PIN | CS_PIN | SCLK_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//=====================================================================

//======================发送8位数据程序===================================
void txd_8bit(uchar txdat)
{
    uchar i, sbt;
    sbt = 0x80;
    SCLK(0);
    for (i = 0; i < 8; i++)
    {
        if ((txdat & sbt) == 0) AD9910_SDIO(0);
        else AD9910_SDIO(1);
        SCLK(1);
        sbt = sbt >> 1;
        SCLK(0);
    }
}


/************************************************************
** 函数名称 ：void Init_AD9910(void)
** 函数功能 ：初始化AD9910的管脚和最简单的内部寄存器的配置
** 入口参数 ：无
** 出口参数 ：无
**************************************************************/
void Init_AD9910(void)
{
    uchar k, m;

    AD9910_IOInit();
    AD9910_PWR(0);

    PROFILE2(0); PROFILE1(0); PROFILE0(0);
    DRCTL(0);
    DRHOLD(0);
    MAS_REST(1);
    HAL_Delay(5);
    MAS_REST(0);

    CS(0);
    txd_8bit(0x00);
    for (m = 0; m < 4; m++)
        txd_8bit(cfr1[m]);
    CS(1);
    for (k = 0; k < 10; k++);

    CS(0);
    txd_8bit(0x01);
    for (m = 0; m < 4; m++)
        txd_8bit(cfr2[m]);
    CS(1);
    for (k = 0; k < 10; k++);

    CS(0);
    txd_8bit(0x02);
    for (m = 0; m < 4; m++)
        txd_8bit(cfr3[m]);
    CS(1);
    for (k = 0; k < 10; k++);

    UP_DAT(1);
    for (k = 0; k < 10; k++);
    UP_DAT(0);
    HAL_Delay(1);
}

/************************************************************
** 函数名称 ：void Txfrc(void)
** 函数功能 ：向AD9910芯片发送profile控制数据
** 入口参数 ：无
** 出口参数 ：无
**************************************************************/
void Txfrc(void)
{
    uchar m;

    CS(0);
    txd_8bit(0x0e);
    for (m = 0; m < 8; m++)
        txd_8bit(profile11[m]);
    CS(1);

    UP_DAT(1);
    UP_DAT(0);
}

/************************************************************
** 函数名称 ：void AD9910_FreWrite(ulong Freq)
** 函数功能 ：写入频率控制字
** 入口参数 ：目标频率，单位Hz，范围0~420000000
** 出口参数 ：无
**************************************************************/
void AD9910_FreWrite(ulong Freq)
{
    ulong Temp;
    Temp = (ulong)Freq * 4.294967296;
    profile11[7] = (uchar)Temp;
    profile11[6] = (uchar)(Temp >> 8);
    profile11[5] = (uchar)(Temp >> 16);
    profile11[4] = (uchar)(Temp >> 24);
    Txfrc();
}

/************************************************************
** 函数名称 ：void AD9910_AmpWrite(uint16_t Amp)
** 函数功能 ：写入幅度控制字
** 入口参数 ：幅度控制字，范围0~16383
** 出口参数 ：无
** 函数说明 ：14位幅度控制字，0~16383对应输出幅度0~800mV左右
**************************************************************/
void AD9910_AmpWrite(uint16_t Amp)
{
    profile11[0] = (Amp % 16384) >> 8;
    profile11[1] = (Amp % 16384) & 0xff;
    Txfrc();
}
