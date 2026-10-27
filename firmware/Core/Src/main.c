/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   蜂鸣器驱动 + PVD 欠压保护 最终方案（无 RTC）
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "buzzer_driver.h"
#include "music_data.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void PVD_UserInit(void); /* 手动 PVD，避免与 CubeMX pwr.c 冲突 */

/* USER CODE BEGIN 0 */

/**
 * @brief PVD 中断回调（上升沿/下降沿都会进）
 * @note  【必须放在 main() 外面，全局函数】
 */
void HAL_PWR_PVDCallback(void)
{
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_PVDO))
    {
        /* 电压跌落至阈值以下 */
        Buzzer_PowerFault();
    }
    else
    {
        /* 电压回升至阈值以上 */
        Buzzer_PowerRecover();
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
    /* 在 MX_I2C1_Init() 之后调用 */

  MX_I2C1_Init();
    NTAG_Dump();              // 先看当前状态
    NTAG_Init_ForPhoneWrite();// 写入 CC / 配置
    NTAG_Dump();              // 确认写进去了
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
    /* MCU Configuration--------------------------------------------------------*//* CubeMX 生成的 TIM1 初始化 */
  Buzzer_Init();
  PVD_UserInit();            /* 使能 PVD 中断 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        if (Buzzer_GetState() == BUZZER_FAULT)
        {
            /* 欠压中：进 STOP 苟命，等待 PVD 恢复中断唤醒 */
            HAL_SuspendTick();
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
            HAL_ResumeTick();

            /* STOP 唤醒后默认切回 HSI，必须 restore 系统时钟 */
            SystemClock_Config();
        }
        else
        {
            /* 正常播放：内部每 1 ms 检测一次欠压标志，可被中断打断 */
            Buzzer_PlayMusic_Poll(music_score, MUSIC_LEN);
        }
        /* USER CODE END 3 */
    }
}

/* ============== 时钟配置（示例：HSI16 -> PLL -> 64 MHz）=============== */
/**
  * @brief System Clock Configuration
  * @retval None
  * @note   若你的 CubeMX 已生成 SystemClock_Config，请直接用 CubeMX 版本覆盖下方代码。
  *         唯一要求：TIM1 时钟建议 ≥ 32 MHz，否则蜂鸣器高音上不去。
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* 保留 CubeMX 为你生成的 MX_GPIO_Init() 全部代码。此处仅防止编译报错 */
    /* USER CODE END MX_GPIO_Init_1 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* 其它引脚配置由 CubeMX 生成 ... */

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
}

/* ============== PVD 手动初始化（避免与 CubeMX pwr.c 冲突）============= */
/**
  * @brief 手动配置 PVD（欠压/恢复双沿中断）
  * @note   如果你已经在 CubeMX 里 enable 了 PVD，请删掉本函数，
  *         并在 main() 中改为调用 CubeMX 生成的 MX_PVD_Init()。
  *         阈值请对照数据手册选择，G0 系列典型值如下（上升沿）：
  *           LEVEL_0 ≈ 2.0V | LEVEL_1 ≈ 2.2V | LEVEL_2 ≈ 2.4V
  *           LEVEL_3 ≈ 2.5V | LEVEL_4 ≈ 2.6V | LEVEL_5 ≈ 2.8V | LEVEL_6 ≈ 2.9V
  */
static void PVD_UserInit(void)
{
    PWR_PVDTypeDef sConfigPVD = {0};

    sConfigPVD.PVDLevel = PWR_PVDLEVEL_1;       /* 例：2.2V 档，按需求改 */
    sConfigPVD.Mode     = PWR_PVD_MODE_IT_RISING_FALLING;

    HAL_PWR_ConfigPVD(&sConfigPVD);
    HAL_PWR_EnablePVD();

    HAL_NVIC_SetPriority(PVD_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(PVD_IRQn);
}

/* ============== 错误处理 ============================================== */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {

  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file; (void)line;
}
#endif /* USE_FULL_ASSERT */
