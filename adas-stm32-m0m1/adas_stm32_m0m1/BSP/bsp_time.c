#include "bsp_time.h"
#include "main.h"

void bsp_time_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t bsp_time_cycles(void)
{
    return DWT->CYCCNT;
}

uint32_t bsp_time_cycles_to_us(uint32_t cycles)
{
    const uint32_t mhz = SystemCoreClock / 1000000U;
    return (mhz == 0U) ? 0U : (cycles / mhz);
}
