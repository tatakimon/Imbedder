# Lessons Learned

## Permanent Rules

### UART RX Interrupts
**Rule:** USART2 global interrupts are disabled in the Base_Firmware CubeMX profile. Never use `HAL_UART_Receive_IT`. Always use zero-timeout polling: `HAL_UART_Receive(&huart, &byte, 1, 0)`.

### UART IRQ Handler
**Rule:** The stm32u5xx_it.c file does not include a `USART2_IRQHandler` by default. When using UART interrupts, you MUST add the handler in the USER CODE 1 block:
```c
extern UART_HandleTypeDef huart2;
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}
```

---
