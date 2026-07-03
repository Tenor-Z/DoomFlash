# DoomFlash
A proof-of-concept bricker/trojan program written for the Nintendo DSi

## WARNING/DISCLOSURE
Running this program will permanently damage your Nintendo DSi. Although recovery is possible via a NAND backup, it does not guarantee that your system will be restored completely intact. Ensure you run this program in an isolated testing environment. I am not responsible for unintentional damage to consoles beyond this point. You take full responsibility in downloading and running this program on real hardware

## Functionality
Using the newer BlocksDS development kit, access to the NAND and other filesystems is a tad more simpler. This program bricks the console by seeking all components of the System Menu (Home Menu) on the system and overwriting it with invalid data. Rather than the System Menu being it's own individual app, it is comprised of various .app files that designed to work together. The most straightforward way to prevent functionality is by writing garbage data to each .app file for the title, stopping operability right as soon as the power is turned on. 

## Note
This program only works on DSi systems. If you received a copy of this program that does not have a graphical banner on the top screen, it can be assumed that it is stolen, and you should take extra precautions. The banner and the warning screens are purposefully designed to inform the user of what they are running, as well as provide a window of space to safely exit out of the application.

## Emulation Limitations
This program does not properly work on any DS emulator. MelonDS prevents writing to the NAND and DSi hardware, while NO$GBA and Desmume are too old to be used for it. DoomFlash will not be able to properly initialize the NAND filesystem in an emulated environment, and as such, it is restricted only to physical consoles.
