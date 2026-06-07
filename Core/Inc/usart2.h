#ifndef __USART2_H
#define __USART2_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#define USART2_REC_LEN   200

#define SWEEP_FREQ_MIN    1000
#define SWEEP_FREQ_MAX    50000
#define SWEEP_FREQ_STEP   200
#define SWEEP_POINT_COUNT ((SWEEP_FREQ_MAX - SWEEP_FREQ_MIN) / SWEEP_FREQ_STEP + 1)

#define FRAME_HEAD1   0xAA
#define FRAME_HEAD2   0x55
#define FRAME_TAIL1   0x55
#define FRAME_TAIL2   0xAA
#define FRAME_SPLIT   0xA2
#define FRAME_CMD     0xCC
#define FRAME_CMD_REPLICATE 0xA8

#define COLOR_YELLOW  65504
#define COLOR_DEFAULT 65535

extern uint8_t  USART2_RX_BUF[USART2_REC_LEN];
extern uint16_t USART2_RX_STA;
extern uint32_t freq;
extern uint32_t vpp;
extern volatile uint8_t g_init_done;

extern float sweep_vpp_data[SWEEP_POINT_COUNT];
extern float sweep_slope_data[SWEEP_POINT_COUNT];
extern volatile uint8_t sweep_start_flag;
extern volatile uint8_t sweep_running;
extern volatile uint8_t replicate_start_flag;

void usart2_send_byte(uint8_t data);
void usart2_send_string(char *str);
void usart2_send_buf(uint8_t *buf, uint16_t len);

uint8_t usart2_recv_byte(void);
uint32_t usart2_recv_value(void);
void USART2_ProcessPacket(void);
void USART2_Reset(void);
void USART2_StartReceive(void);

#endif
