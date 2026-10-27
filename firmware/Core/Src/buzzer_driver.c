//
// Created by 18282 on 2026/9/11.
//
#include "buzzer_driver.h"
#include "main.h"           /* 包含 CubeMX 生成的 TIM_HandleTypeDef htim1 */

extern TIM_HandleTypeDef htim1;

/* =========================================
 * 定时器分频后计数频率（必须和 CubeMX 里的 PSC 对应）
 * 例：64 MHz / (63+1) = 1 MHz
 * ========================================= */
#ifndef BUZZER_PWM_CLK
#define BUZZER_PWM_CLK    1000000UL
#endif

static volatile Buzzer_State_t bState = BUZZER_IDLE;
static volatile bool           bFault = false;

/* =========================================
 * 内部：动态改频
 * ========================================= */
static void Buzzer_SetFreq(uint16_t freq_hz)
{
    if (freq_hz == 0U) {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0U);
        return;
    }

    /* ARR = 1 MHz / freq - 1 */
    uint32_t arr = (BUZZER_PWM_CLK / (uint32_t)freq_hz) - 1UL;

    if (arr > 0xFFFFU) arr = 0xFFFFU;
    if (arr < 1U)      arr = 1U;

    __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (arr + 1U) / 2U); /* 50% */

    /* 立即生效 */
    htim1.Instance->EGR = TIM_EGR_UG;
}

/* =========================================
 * 差分 PWM 启停（重点：MOE 安全操作）
 * ========================================= */

void Buzzer_PWM_Start(void)
{
    /* 读 BDTR 寄存器判断 MOE，兼容所有 HAL 版本 */
    if ((htim1.Instance->BDTR & TIM_BDTR_MOE) == 0U)
    {
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
        __HAL_TIM_MOE_ENABLE(&htim1);
    }
    else
    {
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    }
}

void Buzzer_PWM_Stop(void)
{
    /* 必须顺序：互补 -> 主通道 -> MOE */
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_MOE_DISABLE(&htim1);
}

/* =========================================
 * 状态机 API
 * ========================================= */

void Buzzer_Init(void)
{
    bState = BUZZER_IDLE;
    bFault = false;
    Buzzer_PWM_Stop();
}

void Buzzer_Stop(void)
{
    Buzzer_PWM_Stop();
    bState = BUZZER_IDLE;
}

void Buzzer_Pause(void)
{
    if (bState == BUZZER_PLAYING) {
        Buzzer_PWM_Stop();
        bState = BUZZER_PAUSED;
    }
}

void Buzzer_Resume(void)
{
    if ((bState == BUZZER_PAUSED) && (!bFault)) {
        Buzzer_PWM_Start();
        bState = BUZZER_PLAYING;
    }
}

Buzzer_State_t Buzzer_GetState(void)
{
    return bState;
}

/* =========================================
 * PVD 事件响应
 * ========================================= */

void Buzzer_PowerFault(void)
{
    bFault = true;
    Buzzer_PWM_Stop();
    bState = BUZZER_FAULT;
}

void Buzzer_PowerRecover(void)
{
    bFault = false;
    if (bState == BUZZER_FAULT) {
        bState = BUZZER_IDLE;
    }
}

/* =========================================
 * 阻塞式轮询播放（可被 PVD 随时打断退出）
 * ========================================= */

void Buzzer_PlayMusic_Poll(const MusicNote_t *score, uint16_t length)
{
    if ((score == NULL) || (length == 0U)) return;
    if (bFault) return;

    bState = BUZZER_PLAYING;

    for (uint16_t i = 0; i < length; i++)
    {
        if (bFault) {
            Buzzer_PWM_Stop();
            bState = BUZZER_FAULT;
            return;
        }

        Buzzer_SetFreq(score[i].freq);

        if (score[i].freq > 0U) {
            Buzzer_PWM_Start();
        } else {
            /* 休止符：彻底关断，防止互补死区造成端电压 */
            Buzzer_PWM_Stop();
        }

        /* 逐毫秒轮询等待，期间实时检查 bFault */
        uint32_t tickstart = HAL_GetTick();
        while ((HAL_GetTick() - tickstart) < (uint32_t)score[i].duration)
        {
            if (bFault) {
                Buzzer_PWM_Stop();
                bState = BUZZER_FAULT;
                return;
            }
        }
    }

    /* 正常结束 */
    Buzzer_PWM_Stop();
    bState = BUZZER_IDLE;
}
