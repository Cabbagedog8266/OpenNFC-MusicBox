//
// Created by 18282 on 2026/9/11.
//

#ifndef __BUZZER_DRIVER_H
#define __BUZZER_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================
 * 音符结构
 * ========================================= */
typedef struct {
    uint16_t freq;      /* 频率(Hz)。0 表示休止符 */
    uint16_t duration;  /* 持续时间(ms) */
} MusicNote_t;

/* =========================================
 * 播放器状态
 * ========================================= */
typedef enum {
    BUZZER_IDLE = 0,
    BUZZER_PLAYING,
    BUZZER_PAUSED,
    BUZZER_FAULT          /* 欠压保护触发中 */
} Buzzer_State_t;

/* =========================================
 * 用户 API
 * ========================================= */
void Buzzer_Init(void);
void Buzzer_PlayMusic_Poll(const MusicNote_t *score, uint16_t length);
void Buzzer_Stop(void);
void Buzzer_Pause(void);
void Buzzer_Resume(void);
Buzzer_State_t Buzzer_GetState(void);

/* PVD 事件接口（在 HAL_PWR_PVDCallback 中调用） */
void Buzzer_PowerFault(void);
void Buzzer_PowerRecover(void);

/* 底层 PWM 控制（通常不直接调用） */
void Buzzer_PWM_Start(void);
void Buzzer_PWM_Stop(void);

#endif
