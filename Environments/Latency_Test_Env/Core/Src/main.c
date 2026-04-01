/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Minimal main for LED + UART2 behavior
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

/* USER CODE BEGIN PD */
#define LED_GREEN_Pin         GPIO_PIN_12
#define LED_GREEN_GPIO_Port   GPIOH

#define LED_ORANGE_Pin        GPIO_PIN_10
#define LED_ORANGE_GPIO_Port  GPIOH

#define UART2_BAUD            115200u
#define ERROR_BLINK_MS        100U
/* Force Cube-generated USART2 init to use TX/RX only (PD5/PD6). */
#undef UART_HWCONTROL_RTS
#define UART_HWCONTROL_RTS    UART_HWCONTROL_NONE

/* ISM330DHCX SPI CS pin (directly defined) */
#define CS_DHCX_Pin           GPIO_PIN_15
#define CS_DHCX_GPIO_Port     GPIOH

/* BlueNRG-M2SA BLE module pins */
#define BLE_CS_Pin            GPIO_PIN_1
#define BLE_CS_GPIO_Port      GPIOE
#define BLE_RST_Pin           GPIO_PIN_13
#define BLE_RST_GPIO_Port     GPIOD
#define BLE_INT_Pin           GPIO_PIN_14
#define BLE_INT_GPIO_Port     GPIOF

/* ISM330DHCX Register Addresses */
#define ISM330DHCX_WHO_AM_I     0x0F
#define ISM330DHCX_CTRL1_XL     0x10
#define ISM330DHCX_CTRL2_G      0x11
#define ISM330DHCX_CTRL3_C      0x12
#define ISM330DHCX_STATUS_REG   0x1E
#define ISM330DHCX_OUTX_L_A     0x28

#define ISM330DHCX_WHO_AM_I_VAL 0x6B

/* Shake detection threshold (in mg, ~300mg above resting noise) */
#define SHAKE_THRESHOLD_MG      300
#define SHAKE_DEBOUNCE_MS       200
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;
DMA_NodeTypeDef Node_GPDMA1_Channel5;
DMA_QListTypeDef List_GPDMA1_Channel5;
DMA_HandleTypeDef handle_GPDMA1_Channel5;
DMA_NodeTypeDef Node_GPDMA1_Channel4;
DMA_QListTypeDef List_GPDMA1_Channel4;
DMA_HandleTypeDef handle_GPDMA1_Channel4;
DMA_HandleTypeDef handle_GPDMA1_Channel3;
DMA_HandleTypeDef handle_GPDMA1_Channel2;
DMA_HandleTypeDef handle_GPDMA1_Channel1;
DMA_HandleTypeDef handle_GPDMA1_Channel0;

/* USER CODE BEGIN PV */
static uint8_t rx_byte;
static char last_action = 'N';
static uint8_t dummy_sensor = 0;
static uint32_t last_state_tick = 0U;
static const uint32_t STATE_INTERVAL_MS = 100U;
static char state_buf[64];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void BlueNRG_GPIO_Init(void);
static void BlueNRG_Reset(void);
static int BlueNRG_SPI_Write(uint8_t *data, uint16_t len);
static int BlueNRG_SPI_Read(uint8_t *data, uint16_t len);
static uint8_t BlueNRG_Init(void);
static uint8_t ISM330DHCX_ReadReg(uint8_t reg);
static void ISM330DHCX_WriteReg(uint8_t reg, uint8_t val);
static uint8_t ISM330DHCX_Init(void);
static void ISM330DHCX_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az);

int main(void)
{
  HAL_Init();
  SystemPower_Config();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  BlueNRG_GPIO_Init();

  /* USER CODE BEGIN 2 */
  /* Start with Green LED off */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

  /* Keep USART2 on PD5/PD6 only; release PD4 that Cube enables for RTS. */
  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_4);

  /* Suppress unused variable warnings */
  (void)rx_byte;

  /* Send ready message */
  (void)HAL_UART_Transmit(&huart2, (uint8_t *)"RL State Vector Ready\r\n", 21, HAL_MAX_DELAY);
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN 3 */
    /* Poll for incoming UART RX byte (non-blocking, zero timeout) */
    if (HAL_UART_Receive(&huart2, &rx_byte, 1, 0) == HAL_OK)
    {
      if (rx_byte == 'A')
      {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        last_action = 'A';
      }
      else if (rx_byte == 'B')
      {
        last_action = 'B';
      }
    }

    /* Transmit STATE CSV every 100ms (non-blocking timing) */
    uint32_t now = HAL_GetTick();
    if ((now - last_state_tick) >= STATE_INTERVAL_MS)
    {
      last_state_tick = now;

      /* Increment dummy sensor, reset at 100 */
      dummy_sensor++;
      if (dummy_sensor >= 100)
      {
        dummy_sensor = 0;
      }

      /* Build STATE CSV string */
      int len = snprintf(state_buf, sizeof(state_buf),
                         "STATE,%lu,%c,%u\r\n",
                         (unsigned long)now,
                         (char)last_action,
                         (unsigned int)dummy_sensor);
      (void)HAL_UART_Transmit(&huart2, (uint8_t *)state_buf, (uint16_t)len, HAL_MAX_DELAY);
    }
    /* USER CODE END 3 */
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSI
                                   | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                              | RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

static void SystemPower_Config(void)
{
  HAL_PWREx_EnableVddIO2();
  HAL_PWREx_DisableUCPDDeadBattery();
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = UART2_BAUD;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_RTS;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();

  /* Initialize CS_DHCX pin high (deselected) */
  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOH, LED_GREEN_Pin | LED_ORANGE_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = LED_GREEN_Pin | LED_ORANGE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* CS_DHCX pin as output */
  GPIO_InitStruct.Pin = CS_DHCX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CS_DHCX_GPIO_Port, &GPIO_InitStruct);
}

static void MX_SPI2_Init(void)
{
  /* Enable SPI2 and GPIO clocks */
  __HAL_RCC_SPI2_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Configure SPI2 GPIO pins: PI1=CLK, PI3=MOSI, PD3=MISO */
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* PI1 = SCK, PI3 = MOSI */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* PD3 = MISO */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* Configure SPI2 - Mode 3 (CPOL=1, CPHA=1) */
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;  /* ~5MHz at 160MHz PCLK */
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI3_Init(void)
{
  /* Enable SPI3 and GPIO clocks */
  __HAL_RCC_SPI3_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Configure SPI3 GPIO pins: PG9=SCK, PB5=MOSI, PB4=MISO */
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* PG9 = SCK */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* PB5 = MOSI */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* PB4 = MISO */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Configure SPI3 - Mode 0 (CPOL=0, CPHA=0) for BlueNRG */
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;  /* ~1.25MHz at 160MHz PCLK */
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
}

static void BlueNRG_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Enable GPIO clocks */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /* Initialize CS pin high (deselected) */
  HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);

  /* Initialize RST pin low (hold in reset initially) */
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET);

  /* BLE_CS (PE1) - Output */
  GPIO_InitStruct.Pin = BLE_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BLE_CS_GPIO_Port, &GPIO_InitStruct);

  /* BLE_RST (PD13) - Output */
  GPIO_InitStruct.Pin = BLE_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BLE_RST_GPIO_Port, &GPIO_InitStruct);

  /* BLE_INT (PF14) - Input with interrupt */
  GPIO_InitStruct.Pin = BLE_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_INT_GPIO_Port, &GPIO_InitStruct);
}

static void BlueNRG_Reset(void)
{
  /* Assert reset low for at least 5ms */
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);

  /* Release reset and wait for BlueNRG boot (needs 100-200ms) */
  HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
}

static int BlueNRG_SPI_Write(uint8_t *data, uint16_t len)
{
  uint8_t header_tx[5] = {0x0A, 0x00, 0x00, 0x00, 0x00};  /* Write header */
  uint8_t header_rx[5] = {0};
  int retry = 10;
  uint16_t wbuf_size;

  /* Wait for BlueNRG to be ready */
  while (retry > 0)
  {
    HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi3, header_tx, header_rx, 5, HAL_MAX_DELAY);

    /* Check if ready (header_rx[0] should be 0x02) */
    if (header_rx[0] == 0x02)
    {
      wbuf_size = (header_rx[2] << 8) | header_rx[1];
      if (wbuf_size >= len)
      {
        /* BlueNRG ready, send data */
        HAL_SPI_Transmit(&hspi3, data, len, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);
        return 0;  /* Success */
      }
    }

    HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    retry--;
  }

  return -1;  /* Timeout */
}

static int BlueNRG_SPI_Read(uint8_t *data, uint16_t len)
{
  uint8_t header_tx[5] = {0x0B, 0x00, 0x00, 0x00, 0x00};  /* Read header */
  uint8_t header_rx[5] = {0};
  int retry = 10;
  uint16_t rbuf_size;

  /* Wait for BlueNRG to have data */
  while (retry > 0)
  {
    HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi3, header_tx, header_rx, 5, HAL_MAX_DELAY);

    /* Check if ready (header_rx[0] should be 0x02) */
    if (header_rx[0] == 0x02)
    {
      rbuf_size = (header_rx[4] << 8) | header_rx[3];
      if (rbuf_size > 0)
      {
        uint16_t to_read = (rbuf_size < len) ? rbuf_size : len;
        /* Send dummy bytes to receive data */
        uint8_t dummy[256] = {0};
        HAL_SPI_TransmitReceive(&hspi3, dummy, data, to_read, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);
        return to_read;  /* Return bytes read */
      }
    }

    HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    retry--;
  }

  return 0;  /* No data */
}

static uint8_t BlueNRG_Init(void)
{
  char dbg[80];
  int dlen;

  HAL_UART_Transmit(&huart2, (uint8_t *)"BlueNRG: Starting init...\r\n", 27, HAL_MAX_DELAY);

  /* Reset the BlueNRG module (includes 200ms boot delay) */
  BlueNRG_Reset();
  HAL_UART_Transmit(&huart2, (uint8_t *)"BlueNRG: Reset done, ready\r\n", 28, HAL_MAX_DELAY);

  /* Try to read SPI header to check communication */
  uint8_t header_tx[5] = {0x0B, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_rx[5] = {0};

  HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi3, header_tx, header_rx, 5, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);

  dlen = snprintf(dbg, sizeof(dbg), "BlueNRG SPI: [0x%02X 0x%02X 0x%02X 0x%02X 0x%02X]\r\n",
                  header_rx[0], header_rx[1], header_rx[2], header_rx[3], header_rx[4]);
  HAL_UART_Transmit(&huart2, (uint8_t *)dbg, dlen, HAL_MAX_DELAY);

  /* Send HCI_RESET command */
  uint8_t hci_reset[] = {0x01, 0x03, 0x0C, 0x00};  /* HCI Reset command */
  if (BlueNRG_SPI_Write(hci_reset, sizeof(hci_reset)) < 0)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)"BlueNRG: SPI Write failed\r\n", 27, HAL_MAX_DELAY);
    return 0;  /* Failed to send reset */
  }

  HAL_UART_Transmit(&huart2, (uint8_t *)"BlueNRG: HCI Reset sent\r\n", 25, HAL_MAX_DELAY);

  /* Wait for response */
  HAL_Delay(100);

  /* Read response */
  uint8_t response[16] = {0};
  int len = BlueNRG_SPI_Read(response, sizeof(response));

  /* Check for command complete event (0x04 0x0E ...) */
  if (len >= 4 && response[0] == 0x04 && response[1] == 0x0E)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)"BlueNRG: HCI Reset OK\r\n", 23, HAL_MAX_DELAY);
    return 1;  /* Success */
  }

  /* Debug print response */
  dlen = snprintf(dbg, sizeof(dbg), "BlueNRG: Response len=%d [0x%02X 0x%02X 0x%02X 0x%02X]\r\n",
                  len, response[0], response[1], response[2], response[3]);
  HAL_UART_Transmit(&huart2, (uint8_t *)dbg, dlen, HAL_MAX_DELAY);

  return 0;  /* Failed */
}

static uint8_t ISM330DHCX_ReadReg(uint8_t reg)
{
  uint8_t tx[2] = {0x80 | reg, 0x00};  /* Read: MSB=1 */
  uint8_t rx[2] = {0};

  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi2, tx, rx, 2, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_SET);

  return rx[1];
}

static void ISM330DHCX_WriteReg(uint8_t reg, uint8_t val)
{
  uint8_t tx[2] = {reg, val};  /* Write: MSB=0 */

  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi2, tx, 2, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_SET);
}

static uint8_t ISM330DHCX_Init(void)
{
  HAL_Delay(10);  /* Boot time */

  /* Check WHO_AM_I */
  uint8_t who = ISM330DHCX_ReadReg(ISM330DHCX_WHO_AM_I);
  
  /* Debug: print WHO_AM_I value */
  char dbg[40];
  int len = snprintf(dbg, sizeof(dbg), "WHO_AM_I: 0x%02X (expect 0x6B)\r\n", who);
  HAL_UART_Transmit(&huart2, (uint8_t *)dbg, len, HAL_MAX_DELAY);
  
  if (who != ISM330DHCX_WHO_AM_I_VAL)
  {
    return 0;  /* Failed */
  }

  /* CTRL3_C: BDU=1 (bit6), IF_INC=1 (bit2) */
  ISM330DHCX_WriteReg(ISM330DHCX_CTRL3_C, 0x44);

  /* CTRL1_XL: ODR=104Hz (0100), FS=+-4g (10) -> 0x48 */
  ISM330DHCX_WriteReg(ISM330DHCX_CTRL1_XL, 0x48);

  /* CTRL2_G: Gyro off (ODR=0) */
  ISM330DHCX_WriteReg(ISM330DHCX_CTRL2_G, 0x00);

  return 1;  /* Success */
}

static void ISM330DHCX_ReadAccel(int16_t *ax, int16_t *ay, int16_t *az)
{
  uint8_t tx[7] = {0x80 | ISM330DHCX_OUTX_L_A, 0, 0, 0, 0, 0, 0};
  uint8_t rx[7] = {0};

  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi2, tx, rx, 7, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(CS_DHCX_GPIO_Port, CS_DHCX_Pin, GPIO_PIN_SET);

  *ax = (int16_t)((rx[2] << 8) | rx[1]);
  *ay = (int16_t)((rx[4] << 8) | rx[3]);
  *az = (int16_t)((rx[6] << 8) | rx[5]);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  GPIO_InitTypeDef led_init = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();
  led_init.Pin = LED_GREEN_Pin | LED_ORANGE_Pin;
  led_init.Mode = GPIO_MODE_OUTPUT_PP;
  led_init.Pull = GPIO_NOPULL;
  led_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &led_init);

  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
  while (1)
  {
    HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
    HAL_Delay(ERROR_BLINK_MS);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif
