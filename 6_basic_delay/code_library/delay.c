#include "ti_msp_dl_config.h"
#include "delay.h"

void delay_ms(uint16_t ms)
{
  while (ms--)
    delay_cycles(CPUCLK_FREQ / 1000);
}
