/*
Bus Access system
Access 65C02 address and databus on 24 Teensy Pins
Start simulate some memory for 65c02 to Read/Write.
*/

#include "SBC.h"

#ifdef ROMEMU
  #include "rom.h"
#endif


// Declare the Pins to use


byte dataPins[] = {2,14,7,8,6,20,21,5};         // Data Bus
byte addressHPins [] = {16,17,19,18,0,1,29,30}; // Address Bus High byte
byte addressLPins[]={15,22,23,9,10,13,11,12};   // Address Bus Low byte
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

#ifdef RAMEMU
  byte mem[RAMSIZE];  // declare RAM as a byte array.
#endif

// Redefine yield() as the default one is useless and waste time.
void yield(){}

void setup() {

  Serial.begin(0);                // Start the serial port

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

    Serial.print("Initializing PINs\n");
  #endif 
  for (int i=0;i<8;i++) {
    pinMode(dataPins[i],INPUT);               //Ensure we're not writing to the Bus
    pinMode(addressLPins[i],INPUT_PULLDOWN);
    pinMode(addressHPins[i],INPUT_PULLDOWN);
  }

  pinMode(RWPIN, INPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(RESETPIN, OUTPUT);

  #ifdef DEBUG
  Serial.print("Initializing varialbles\n");
  #endif

  // Initialize some variables
  addressL=0x0;
  addressH=0x0;
  chipEnable=0x0;
  rw=true;

  #ifdef DEBUG
  Serial.print("Bringing RESET LOW\n");
  #endif
  
  digitalWrite(RESETPIN,LOW);     // Bring the RESET LOW to get the 6502
                                  // in a reset state

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
  
  Serial.print("Starting SBC....\n");
}


void loop(){

  GPIOA_PDOR &=0x0000;              // Bring Clock LOW to start Phase 1
  for (i=0; i< WAITCYCLE; i++){}    // Keep Phase 1 LOW
  GPIOA_PDOR |=1<<13;               // Bring Clock HIGH to start Phase 2


  addressL = GPIOC_PDIR;                  // Read the Address Bus Low byte
  addressH = GPIOB_PDIR;                  // Read the Address Bus High byte
  addressH |= (GPIOB_PDIR >> 12) & 0xF0;  // Shift the register right by 12 bits
                                          // to get bits 16,17,18 and 19
                                          // in position 4,5,6 and 7.
                                          // Mask bits 0,1,2,3 and
                                          // add to addressH.

  address = ((uint16_t)addressH << 8)| addressL; // Build the complete address
  #ifdef DEBUG
  savedaddress=address;                           // Saving original address for DEBUG as it's modified for ROM access. 
  #endif
  
  if (address < RAMSIZE) { 
        chipEnable=RAMENABLE;
      } else  if (address >= ROMADDRESS) {
        chipEnable=ROMENABLE;
      } else if (address >= ACIADDRESS) {
        chipEnable=ACIAENABLE;
      }
  rw=(GPIOA_PDIR>>12)&0x1;                  // Check if it's a Write Cycle
                                            // or Read Cycle
  GPIOD_PDDR=(rw) ? GPIOOUTPUT : GPIOINPUT; // Configure PIN direction.



  switch (chipEnable) {


#if defined ACIA6551 || defined ACIA6850
    case ACIAENABLE:
      if (rw) {
        if (address == ACIADATA){           // CPU wants to get data
          GPIOD_PDOR=Serial.read() ;        // Send what's in buffer (CPU should have checked buffer ia not empty)
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
