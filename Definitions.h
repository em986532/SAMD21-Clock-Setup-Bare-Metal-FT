/*
 * Definitions.h
 *
 * Created: 12/5/2020
 *  Author: ForceTronics
 */ 


#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

// Constants for Clock Generators
#define GENERIC_CLOCK_GENERATOR_0   (0u)
#define GENERIC_CLOCK_GENERATOR_1   (1u)

//Constants for clock identifiers
//XOSC32K is a 32.768 kHz external crystal oscillator
#define CLOCK_XOSC32K	0x05
//DFLL48M is a 48 MHz internal oscillator
#define CLOCK_DFLL48	0x07
//8 MHz internal oscillator
#define CLOCK_8MHZ		0x06

#endif /* DEFINITIONS_H_ */