//
// TEENSY 65C02 
// 
// Access 65C02 address and databus using Teensy 3.6(3.3v) or 3.5 (5v)
// Can simulate RAM, ROM and ACIA (6551 or 6850)
// Provide access to all pins. 


#include "SBC.h"

// ROM image declared as a C Array.
// Can be easily created from a real ROM image using srec_cat  
// http://srecord.sourceforge.net/man/man1/srec_cat.html

#ifdef ROMEMU
  #include "rom.h"
#endif


// Declare the Pins to use

// Bus Pins (DATA and ADDRESS)
// As the Teensy doens't expose 16 consecutive PINS attached to the same PORT
// we need to split the Address bus over two different PORTS

byte dataPins[] = {5,21,20,6,8,7,14,2};         // Data Bus
byte addressHPins [] = {30,29,1,0,18,19,17,16}; // Address Bus High byte
byte addressLPins[]={12,11,13,10,9,23,22,15};   // Address Bus Low byte

// Variable declarations 

uint8_t chipEnable;                             // RAM, ROM or Serial to access

#ifdef DEBUG
byte databyte=0;              
uint16_t savedaddress=0;
#endif


// Declare global variables
// Must be volatile to avoid any compiler optimization

volatile byte addressL,addressH;
volatile uint16_t i;
volatile boolean rw;
volatile byte ACIAStatus;
volatile uint16_t address;

// If RAM emulation is required 
// declare a corresponding array

#ifdef RAMEMU
   byte mem[RAMSIZE];  // declare RAM as a byte array.
#endif

// Redefine yield() as the default one is useless and waste time.
// This is required to achieve a descent speed

void yield(){}

void setup() {

  Serial.begin(0);                // Start the serial port
  
  #ifdef DEBUG
  Serial.print("Bringing RESET LOW\n");
  #endif

  // First thing, let's bring reset LOW 
  // This will stall the 6502 while we Initialize

  pinMode(RESETPIN, OUTPUT);
  digitalWrite(RESETPIN,LOW);     // Bring the RESET LOW to get the 6502
                                  // in a reset state

  // Set all pins to their initial state.
  #ifdef DEBUG
    delay(2000);
    #ifdef RAMEMU
    Serial.print("RAM Emulation\n");
    #endif
    #ifdef ROMEMU
    Serial.print("ROM Emulation\n");
    #endif
    #ifdef ACIA6551
    Serial.print("6551 ACIA Emulation\n");
    #endif
    #ifdef ACIA6850
    Serial.print("6850 ACIA Emulation\n");
    #endif
    #ifdef BARECPU
    Serial.print("CPU Only\n");
    #endif
    Serial.print("Initializing PINs\n");
  #endif

  // Initialize Data and Address BUSs to Input
  
  for (int i=0;i<8;i++) {
    pinMode(dataPins[i],INPUT);               //Ensure we're not writing to the Bus
    pinMode(addressLPins[i],INPUT_PULLDOWN);
    pinMode(addressHPins[i],INPUT_PULLDOWN);
  }

  // Initialize R/W' and PHI2 

  pinMode(RWPIN, INPUT);
  pinMode(CLOCKPIN, OUTPUT);
  
  // With a bare CPU we need to drive these pins high.
  // And SO LOW
  // On a real SBC, this should be done by the circuit.

  #ifdef BARECPU
  pinMode(BEPIN, OUTPUT);
  pinMode(RDYPIN, OUTPUT);
  pinMode(NMIPIN, OUTPUT);
  pinMode(IRQPIN, OUTPUT);
  pinMode(MLPIN, INPUT);
  pinMode(VPPIN, INPUT);
  pinMode(SYNCPIN, INPUT);
  pinMode(SOPIN,OUTPUT);
  digitalWrite(BEPIN,HIGH);
  digitalWrite(RDYPIN,HIGH);
  digitalWrite(IRQPIN,HIGH);
  digitalWrite(NMIPIN,HIGH);
  digitalWrite(SOPIN,LOW);
  #endif

  // With a CPU on its own circuit we just want to read what happens.

  #ifndef BARECPU
  pinMode(BEPIN, INPUT);
  pinMode(RDYPIN, INPUT);
  pinMode(NMIPIN, INPUT);
  pinMode(IRQPIN, INPUT);
  #endif

  #ifdef DEBUG
  Serial.print("Initializing variables\n");
  #endif

  // Initialize some variables
  addressL=0x0;
  addressH=0x0;
  chipEnable=0x0;
  rw=true;

  
  
  
#ifdef RAMEMU
  #ifdef DEBUG
  Serial.print("Initializing RAM\n");
  #endif
  
  memset(mem,0x00,sizeof(mem));   // Initialize memory to 0x00
#endif                                  
  delay(2000);                    // Keep RESET for 2 second
                                  // in order for Serial to be ready
  #ifdef DEBUG
  Serial.print("Releasing RESET\n");
  #endif
  
  digitalWrite(RESETPIN,HIGH);    // Release RESET and start working
  Serial.print("Starting Teensy 65C02....\n");
}


void loop(){

  
  // Start the loop by bring Clock low and up again
  // Duration of PHI1 (Clock low) is defined by WAITCYCLE

  GPIOA_PDOR &=~(1<<13);            // Bring Clock LOW to start Phase 1
                                    // Clock is on PIN 13 of PORT A

  for (i=0; i< WAITCYCLE; i++){}    // Keep Phase 1 LOW for WAITCYCLES

  GPIOA_PDOR |=1<<13;               // Bring Clock HIGH to start Phase 2

  // Read the Address BUSs

  addressL = GPIOC_PDIR;                  // Read the Address Bus Low byte
  addressH = GPIOB_PDIR;                  // Read the Address Bus High byte

  // PORT B doesn't have 4 consecutive Pins so we need to concatenate

  addressH |= (GPIOB_PDIR >> 12) & 0xF0;  // Shift the register right by 12 bits
                                          // to get bits 16,17,18 and 19
                                          // in position 4,5,6 and 7.
                                          // Mask bits 0,1,2,3 and
                                          // add to addressH.

  address = ((uint16_t)addressH << 8)| addressL; // Build the complete address
  #ifdef DEBUG
  savedaddress=address;                           // Saving original address for DEBUG as it's modified for ROM access. 
  #endif
  
  // Address decoding

  if (address < RAMSIZE) { 
        chipEnable=RAMENABLE;
      } else  if (address >= ROMADDRESS) {
        chipEnable=ROMENABLE;
      } else if (address >= ACIADDRESS) {
        chipEnable=ACIAENABLE;
      }

  
  rw=(GPIOA_PDIR>>12)&0x1;                  // Check if it's a Write Cycle
                                            // or Read Cycle
  
  // If it's a READ Cycle, we need to reconfigure DATA Pins to OUTPUT
  GPIOD_PDDR=(rw) ? GPIOOUTPUT : GPIOINPUT; // Configure PIN direction.


  // Based on Address decoding, now is time to taje appropriate actions.
  switch (chipEnable) {

  #if defined ACIA6551 || defined ACIA6850
    case ACIAENABLE:
      if (rw) {
        if (address == ACIADATA){           // CPU wants to get data
          GPIOD_PDOR=Serial.read() ;        // Send what's in buffer (CPU should have checked buffer is not empty)
        } else if (address == ACIASTATUS) { // CPU Wants to check buffer status
          GPIOD_PDOR=(Serial.available()>0) ? (RDRFBIT | TDREBIT) : TDREBIT;  // We only check if read buffer is empty.
        }
      } else {
        if (address== ACIADATA) {      // Sending something to display ?
          Serial.write((char)GPIOD_PDIR&0xFF);  // Sends the data to Serial Interface.
        }
        // We simply ignore the case where we receive config /Control/Reset byte for ACIA.
      }
      break;
  #endif


#ifdef ROMEMU   
    case ROMENABLE:             // ROM is selected
      address-=ROMADDRESS;      //Change address to map to ROM address range
      GPIOD_PDOR= rom[address]; // Write data from ROM to Address Bus
      break;
#endif
    
#ifdef RAMEMU
    case RAMENABLE:
    if (rw) {
        GPIOD_PDOR= mem[address]; // Read data from RAM
      } else {
        mem[address]=GPIOD_PDIR;  // Write data to RAM
      }
#endif

    default:
      break;
  }
  #ifdef DEBUG
  databyte=(rw) ? GPIOD_PDOR : GPIOD_PDIR;

  Serial.print(savedaddress,HEX);
  Serial.print("\t");
  Serial.print(rw);
  Serial.print("\t");
  Serial.print(databyte,HEX);
  Serial.print("\n");
  delay(50);
  #endif
  GPIOD_PDDR=GPIOINPUT;           // Switch back GPIO to Input
}
