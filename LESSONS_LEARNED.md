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

### STTS22H I2C Temperature Sensor

**Lesson 1: I2C Address is critical**
- STTS22H has two I2C addresses: `STTS22H_I2C_ADD_L` (0x7F) and `STTS22H_I2C_ADD_H` (0x71)
- When using HAL_I2C_Mem_Read with 7-bit addressing mode, HAL automatically shifts the address left by 1 bit
- So passing `0x3F` (7-bit) becomes `0x7E` (8-bit write), which is correct
- But passing `0x7E` directly might cause issues - use `0x7F` or `0x71` as the base address

**Lesson 2: HAL_I2C_Mem_Read returns HAL_ERROR but data is transferred**
- When the I2C peripheral has an error flag set (like ARLO - Arbitration Loss), HAL_I2C_Mem_Read returns HAL_ERROR (1) even though data was actually transferred
- Always check `__HAL_I2C_CLEAR_FLAG(&hi2c, I2C_FLAG_ARLO)` to clear error flags before operations
- The buff value can still be valid even when HAL returns error

**Lesson 3: STTS22H Temperature is 16-bit, not 8-bit**
- Despite being able to read 8-bit, the proper temperature output is 16-bit two's complement
- Formula: `temp_C = ((TEMP_H << 8) | TEMP_L) / 256.0`
- Or use the helper: `stts22h_from_lsb_to_celsius(raw16)`
- Reading only TEMP_L (0x06) and treating as 8-bit gives wrong values

**Lesson 4: Sensor needs warm-up time after power-on**
- STTS22H may return 0x00 for the first ~60 readings after init
- The sensor needs time to complete first conversion
- Consider adding a startup delay or retry logic

**Lesson 5: Free-run mode requires correct CTRL register bits**
- CTRL register at 0x04 has: bit0=one_shot, bit1=time_out_dis, bit2=freerun, bit3=if_add_inc
- For continuous conversion: set freerun (bit2) = 1, and if_add_inc (bit3) = 1 for multi-byte reads
- Correct value: 0x0C (freerun + auto-increment)

**Lesson 6: I2C timing from BASE_one works with U5xx**
- Timing value `0x00F07BFF` is verified working for STM32U5xx at 100kHz I2C
- Use `I2C_ANALOGFILTER_ENABLE` as done in BASE_one

**Lesson 7: Init debug messages may not appear in serial output**
- UART buffer captures STATE messages from TickCount ~1000+
- Init messages ("Reading WHO_AM_I", "STTS22H: Configured OK") are sent before STATE loop starts
- These messages may be lost if serial connection starts after init completes
- Solution: Add LED blink pattern to indicate init status instead of relying on UART

**Lesson 8: Debug output format matters for parsing**
- Using `">>>TR=%d,B=%02X"` format helps distinguish debug lines from STATE lines
- STATE lines start with "STATE," so any line NOT starting with "STATE," is debug

**Lesson 9: 16-bit temperature read bug - symptoms and fix**
- Initial bug: read only TEMP_L as 8-bit signed → showed -35°C (WRONG)
- Second bug: read 16-bit but divided by 256 → showed ~11°C (WRONG)
- CORRECT formula from STMicroelectronics reference driver (stts22h_reg.c):
  - `raw = (TEMP_H << 8) | TEMP_L`
  - `temp_C = raw / 100.0f` (NOT 256!)
  - raw IS the temperature in centi-degrees (e.g., raw=2800 → 28.00°C)
- For example: raw=0x0AF4 → 2804/100 = 28.04°C (realistic room temp)
