/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00503D58;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* USER CODE BEGIN 1 */
#include <string.h>
#include <stdio.h>

#define NTAG_I2C_ADDR       (0x55 << 1)   // 默认 7-bit 地址 0x55，HAL 要左移
#define NTAG_BLOCK_SIZE     16
#define NTAG_WRITE_DELAY_MS 5

/* 读一个 16 字节 block */
static HAL_StatusTypeDef ntag_read_block(uint8_t block, uint8_t *buf)
{
    return HAL_I2C_Mem_Read(&hi2c1, NTAG_I2C_ADDR, block,
                            I2C_MEMADD_SIZE_8BIT, buf, NTAG_BLOCK_SIZE, 200);
}

/* 写一个 16 字节 block，EEPROM 写后等 5ms */
static HAL_StatusTypeDef ntag_write_block(uint8_t block, const uint8_t *buf)
{
    HAL_StatusTypeDef st = HAL_I2C_Mem_Write(&hi2c1, NTAG_I2C_ADDR, block,
                            I2C_MEMADD_SIZE_8BIT, (uint8_t *)buf, NTAG_BLOCK_SIZE, 200);
    HAL_Delay(NTAG_WRITE_DELAY_MS);
    return st;
}

/* 写 session/configuration 寄存器：MEMA=0xFE, REGA, MASK, DATA */
static HAL_StatusTypeDef ntag_write_reg(uint8_t rega, uint8_t mask, uint8_t data)
{
    uint8_t tx[4] = {0xFE, rega, mask, data};
    return HAL_I2C_Master_Transmit(&hi2c1, NTAG_I2C_ADDR, tx, 4, 200);
}

/* 诊断：打印 Block 0 / 3A / FE */
void NTAG_Dump(void)
{
    uint8_t b0[16], b3A[16], bFE[16];

    if(ntag_read_block(0x00, b0) != HAL_OK) { printf("B0 read err\n"); return; }
    if(ntag_read_block(0x3A, b3A) != HAL_OK) { printf("B3A read err\n"); return; }
    if(ntag_read_block(0xFE, bFE) != HAL_OK) { printf("BFE read err\n"); return; }

    printf("B0: "); for(int i=0;i<16;i++) printf("%02X ", b0[i]); printf("\n");
    printf("CC  [12..15]: %02X %02X %02X %02X\n", b0[12], b0[13], b0[14], b0[15]);

    printf("B3A[0..7]: "); for(int i=0;i<8;i++) printf("%02X ", b3A[i]); printf("\n");

    printf("BFE[0..7]: "); for(int i=0;i<8;i++) printf("%02X ", bFE[i]); printf("\n");
    printf("NS_REG=0x%02X, I2C_LOCKED=%d, RF_LOCKED=%d, WR_BUSY=%d, RF_FIELD=%d\n",
           bFE[6],
           (bFE[6] >> 6) & 1,
           (bFE[6] >> 5) & 1,
           (bFE[6] >> 1) & 1,
           (bFE[6] >> 0) & 1);
}

/* 初始化 NTAG，让手机 NFC 可写 */
void NTAG_Init_ForPhoneWrite(void)
{
    uint8_t b0[16];
    uint8_t b1[16] = {0};
    uint8_t b3A[16] = {0};

    /* 1. 读 Block 0 */
    if(ntag_read_block(0x00, b0) != HAL_OK) {
        printf("NTAG read block0 fail\r\n");
        return;
    }
    printf("UID0=0x%02X, I2C comm OK\r\n", b0[0]);

    /* 2. 写 CC：Block 0 byte[12..15] = E1 10 6D 00
       注意：byte[0] 必须写 0xAA 才能保持默认 I2C 地址 0x55；
             如果写 0x04，从机地址会变成 0x02，后续通信会断 */
    b0[0]  = 0xAA;       // 保持默认 I2C 地址 0x55
    /* byte[1..9] 保持读回值（UID/Internal） */
    /* byte[10..11] 保持读回值（Static Lock，默认 00） */
    b0[12] = 0xE1;       // NDEF magic
    b0[13] = 0x10;       // mapping version
    b0[14] = 0x6D;       // 1k memory size
    b0[15] = 0x00;       // read/write
    if(ntag_write_block(0x00, b0) != HAL_OK) printf("CC write fail\r\n");

    /* 3. 写空 NDEF TLV：Block 1 byte[0..3] = 03 00 FE 00 */
    b1[0] = 0x03;
    b1[1] = 0x00;
    b1[2] = 0xFE;
    b1[3] = 0x00;
    if(ntag_write_block(0x01, b1) != HAL_OK) printf("NDEF TLV write fail\r\n");

    /* 4. 写配置寄存器 Block 0x3A */
    b3A[0] = 0x01;       // NC_REG: PTHRU=0, TRANSFER_DIR=1（NFC 可写）
    b3A[1] = 0x01;       // LAST_NDEF_BLOCK
    b3A[2] = 0xF8;       // SRAM_MIRROR_BLOCK
    b3A[3] = 0x48;       // WDT low
    b3A[4] = 0x08;       // WDT high → 默认 20ms
    b3A[5] = 0x01;       // I2C_CLOCK_STR
    b3A[6] = 0x00;       // REG_LOCK=0，千万不要锁配置！
    b3A[7] = 0x00;
    if(ntag_write_block(0x3A, b3A) != HAL_OK) printf("Config write fail\r\n");

    /* 5. 立即同步到 session 寄存器 */
    ntag_write_reg(0x00, 0xFF, 0x01);  // NC_REG session = 0x01
    ntag_write_reg(0x01, 0xFF, 0x01);  // LAST_NDEF_BLOCK session = 0x01

    /* 6. 清除 I2C 锁 */
    ntag_write_reg(0x06, 0x40, 0x00);  // NS_REG.I2C_LOCKED(bit6)=0

    printf("NTAG init done\r\n");
}
/* USER CODE END 1 */
