## SBC (for Teensy 3.6)

This code emulate all the basic hardware of a simple computer on a Teensy apart from the CPU and RAM itself.

It comes loaded with an OSI Basic Rom and use Teensy's USB as a Serial port.
Base addresses are :

 - 0x0000-0x9FFF - **RAM**
 - 0xA000 - **Serial Port**
 - 0xC000 - **ROM**

It runs at ~1.2Mhz with a standard 180Mhz Teensy and goes up to 1.7Mhz with an overclocked (240Mhz) Teensy.

The connections are described in the Fritzing "SBC v0.5.fzz" one level up which includes also a reset button.

Next steps will be to start replacing the Teensy by actual ROM and ACIA ICs.

Any comments welcome !
