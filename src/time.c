#include "time.h"

static volatile uint32_t sysTickUptime = 0;
// cycles per microsecond
static volatile uint32_t usTicks = 0;

// SysTick
void SysTick_Handler(void)
{
	sysTickUptime++;
}

// Return system uptime in microseconds (rollover in 70minutes)
uint32_t micros(void)
{
	register uint32_t ms, cycle_cnt;
	do {
		ms = sysTickUptime;
		cycle_cnt = SysTick->VAL;
	} while (ms != sysTickUptime);
	return (ms * 1000) + (usTicks * 1000 - cycle_cnt) / usTicks;
}

// Return system uptime in milliseconds (rollover in 49 days)
uint32_t millis(void)
{
    return sysTickUptime;
}

void delayMicroseconds(uint32_t us)
{
	uint32_t now = micros();
	while (micros() - now < us);
}

void delay_ms(uint32_t ms)
{
    while (ms--)
		{
			delayMicroseconds(1000);
		}
}

void systic_init(void)
{
	SysTick_Config (SystemCoreClock/1000);
	usTicks = SystemCoreClock / 1000000;
}
