/*
Bus Access system
Access 65C02 address and databus on 24 Teensy Pins
Start simulate some memory for 65c02 to Read/Write.
*/


#define ACIA6551            // Model of ACIA to emulate
//#define ACIA6850          // 6850 or 6551
#define ROMEMU              // Do we emulate RAM or is it a real chip ? 
#define RAMEMU              // Do we emulate ROM or is it a real chip ?

// For the time being, memory map is :
// 0x0000 - (RAMSIZE-1):                RAM
// SERIALADDRESS - (ROMADDRESS-1):      ACIA
// ROMADDRESS - 0XFFFF:                 ROM
// Make sure you provide right values below, even if the component is not emulated


#define RAMSIZE     0xA000  // Size of RAM to emulate
#define ACIADDRESS  0xA000  // Base address of Serial Device
#define ROMADDRESS  0xC000  // Base address of ROM Data 
                            // make sure your ROM fit as there is nocheck.
                            // if the ROM is too big some of it won't be reachable with 16bits addresses

#define ROMENABLE   1
#define RAMENABLE   2
#define ACIAENABLE  3

#ifdef ACIA6551
#define TDREBIT     0x20     // Transmit Data Register Empty bit
#define RDRFBIT     0x10     // Receive Data Buffer Full bit
#define ACIADATA    ACIADDRESS
#define ACIASTATUS  ACIADDRESS+1
#endif

#ifdef ACIA6850
#define TDREBIT     0x02     // Transmit Data Register Empty bit
#define RDRFBIT     0x01     // Receive Data Buffer Full bit
#define ACIADATA    ACIADDRESS+1
#define ACIASTATUS  ACIADDRESS
#endif

#define WAITCYCLE   5       // How long do we wante Phase 1 to be.

// PIN definitions

#define RWPIN       3       // Read/Write' signal
#define CLOCKPIN    4       // Clock generation
#define RESETPIN    33      // Reset signal

#define GPIOOUTPUT  0xFF       // Value to configure Pins for Output
#define GPIOINPUT   0x00        // Value to configure Pins for Input

