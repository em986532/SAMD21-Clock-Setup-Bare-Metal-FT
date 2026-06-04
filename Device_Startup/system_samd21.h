#ifndef SYSTEM_SAMD21_H_
#define SYSTEM_SAMD21_H_

#include <stdint.h>

extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#endif /* SYSTEM_SAMD21_H_ */
