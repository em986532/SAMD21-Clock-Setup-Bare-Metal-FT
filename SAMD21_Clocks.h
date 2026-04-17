#ifndef __SAMD21_CLOCKS_H_
#define __SAMD21_CLOCKS_H_

class Clocks
{
public:
	// Clocks(bool XOSC32K_Enable);
    // Clocks(int XOSC_Frequency);
    // Clocks(bool XOSC32K_Enable, int XOSC_Frequency);
    Clocks();
	~Clocks();
	
	//SYSCTRL Clocks
	//SYSCTRL – System Controller, Section 17, page 162 
	//there are 20 registers in the SYSCTRL subsystem that I counted.
	void XOSC_Enable(int XOSC_Frequency);
	void OSCULP32K_Enable();
	void OSC32K_Enable();
	void XOSC32K_Enable();
	void OSC8M_Enable();
	void DFLL48M_Enable();
	void FDPLL96M_Enable();

	void initClocks(void);
	void enablePeripheralClock(uint8_t clock_id);
	void disablePeripheralClock(uint8_t clock_id);
	void setClockGenerator(uint8_t generator_id, uint8_t clock_source);
	
private:
	void setupOscillators(void);
	void setupClockGenerators(void);
	void setupPeripheralClocks(void);
};

#endif /* __SAMD21_CLOCKS_H_ */