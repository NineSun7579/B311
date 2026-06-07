#ifndef __AD9910_H__
#define __AD9910_H__

#include "stm32f4xx.h"

#define uchar unsigned char
#define uint  unsigned int	
#define ulong  unsigned long int

#define AD9910_PWR		PAout(1)
#define AD9910_SDIO		PAout(2)
#define DRHOLD				PAout(3)
#define DROVER				PAin(7)
#define UP_DAT				PAout(8)
#define PROFILE1			PAout(15)
#define MAS_REST			PBout(6)
#define SCLK					PBout(7)
#define DRCTL					PBout(12)
#define OSK						PBout(13)
#define PROFILE0			PCout(0)
#define PROFILE2			PCout(1)
#define CS						PCout(2)

void AD9110_IOInit(void);
void Init_AD9910(void);
void AD9910_FreWrite(ulong Freq);										//??潩
void AD9910_AmpWrite(uint16_t Amp);
#endif


