#ifndef __USART3_H
#define __USART3_H
#include "sys.h"

#define UART5_REC_LEN   200
#define EN_UART5_RX     1

extern u8  UART5_RX_BUF[UART5_REC_LEN];
extern u16 UART5_RX_STA;
extern u32 freq;
extern u32 vpp;
extern volatile u8 g_init_done;

void uart5_init(u32 bound);
void uart5_send_byte(u8 data);
void uart5_send_string(char *str);
void uart5_send_buf(u8 *buf, u16 len);

u8  uart5_recv_byte(void);
u32 uart5_recv_value(void);
void UART5_ProcessPacket(void);
void UART5_Reset(void);

#endif
