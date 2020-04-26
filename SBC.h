//
// TEENSY 65C02 
// 
// Access 65C02 address and databus using Teensy 3.6(3.3v) or 3.5 (5v)
// Can simulate RAM, ROM and ACIA (6551 or 6850)
// Provide access to all pins. 



//#define DEBUG 0
#define ACIA6551 1            // Model of ACIA to emulate

//#define ACIA6850          // 6850 or 6551
//#define ROMEMU              // Do we emulate RAM or is it a real chip ? 
//#define RAMEMU              // Do we emulate ROM or is it a real chip ?
#define BARECPU 1            // Are we just using the 65C02 ? 

#ifdef BARECPU
    #ifndef ROMEMU  
        #define ROMEMU 1
    #endif
    #ifndef RAMEMU
        #define RAMEMU 1
    #endif
#endif


// For the time being, memory map is :
// 0x0000 - (RAMSIZE-1):                RAM
// SERIALADDRESS - (ROMADDRESS-1):      ACIA
// ROMADDRESS - 0XFFFF:                 ROM
// Make sure you provide right values below, even if the component is not emulated


#define RAMSIZE     0xC000  // Size of RAM to emulate
#define ACIADDRESS  0xC100  // Base address of Serial Device
#define ROMADDRESS  0xD000  // Base address of ROM Data 
                            // make sure your ROM fit as there is nocheck.
                            // if the ROM is too big some of it won't be reachable with 16bits addresses

#define ROMENABLE   1
#define RAMENABLE   2
#define ACIAENABLE  3

#ifdef ACIA6551
#define TDREBIT     0x10     // Transmit Data Register Empty bit
#define RDRFBIT     0x08     // Receive Data Buffer Full bit
#define ACIADATA    ACIADDRESS
#define ACIASTATUS  ACIADDRESS+1
#endif

#ifdef ACIA6850
#define TDREBIT     0x02     // Transmit Data Register Empty bit
#define RDRFBIT     0x01     // Receive Data Buffer Full bit
#define ACIADATA    ACIADDRESS+1
#define ACIASTATUS  ACIADDRESS
#endif

#define WAITCYCLE   1       // How long do we wante Phase 1 to be.

// PIN definitions

#define RWPIN      3       // Read/Write' signal
#define CLOCKPIN   4       // Clock generation
#define RESETPIN   33      // Reset signal
#define BEPIN      24
#define RDYPIN     34
#define IRQPIN     27
#define NMIPIN     26
#define MLPIN      31
#define SYNCPIN    32
#define VPPIN      36
#define SOPIN      35

// #define SOPIN   

#define GPIOOUTPUT  0xFF       // Value to configure Pins for Output
#define GPIOINPUT   0x00        // Value to configure Pins for Input

