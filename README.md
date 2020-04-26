# SBC (for Teensy 3.6)

This codes uses a Teensy to emulate hardware of a **65C02** based computer apart from CPU itself.
The following parts can be emulated :

 - Clock (mandatory)
 - RAM 
 - ROM 
 - ACIA (6850 or 6551)

## Configuration

### Global configuration
Edit `SBC.h` and `#define` which part are to be emulated by commenting out the unwanted ones: 

    #define ACIA6551
    #define ACIA6850
    #define ROMEMU
    #define RAMEMU

> **Notes**
> 1. Only one ACIA can be defined at a time, the other must be commented out.
> 2. Make sure to provide real values for `RAMSIZE, ACIADRESS` and `ROMADRRESS` even if they are not emulated. 
> 3. If you emulate a component, make sure a real IC is not actually connected, as both Teensy and the IC will put data on the bus at the same time !

### ROM configuration
If you emulate a ROM, provide it as a C Array in a file called `rom.h`. Three ROMs are provided here:

1. **OSI Basic ROM** (rom.h.OSI)
- Uses a 6850.
- RAMSIZE(max): 		0xA000
- ACIADDRESS: 0xA000
- ROMADDRESS: 0xC000

2. **ROM with Monitor and ehBasic** (rom.h.6551/rom.h.6850)
- Uses a 6850 or 6551 depending on the file you include.
- RAMSIZE(max): 0xA000
- ACIADDRESS: 0xA000
- ROMADDRESS: 0xB000

3. **Java6502** (rom.h)
   My own ROM using Microsoft Basic 2
- RAMSIZE(max): 0xC000
- ACIADDRESS: 0xC100
- ROMADDRESS: 0xD000

You can replace with your own ROM, using srec_cat to generate the C Array. See my [ROM Software repo](https://github.com/olivierjan/ROM-software-for-6502-SBC) for more details. 

The code is very basic but is meant to help debug a hombrew computer. It can be greatly optimised and I will try to add new features over time (usable debugging, support for VIAs and other components, etc...).

**WARNING:** Teensy 3.6 is only 3.3v and will provide power to the whole circuit. A 5V version should be possible using Teensy 3.5 but I can't get this to work on Teensy 3.5 at the moment. 

### PCB Board
A PCB to host the Teensy and the 65C02 is provided, I designed it using EasyEDA and ordered it from JLCPCB.
I provided the design here in Altium and Gerber format, but haven't tested these, only the EasyEDA.

The PCB can be plugged in an existing circuit, and you can choose to power the circuit from Teensy or not using the jumper.


Any comments welcome !


> Written with [StackEdit](https://stackedit.io/).
