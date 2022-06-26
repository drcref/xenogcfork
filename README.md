# xenogcfork by drcref

This is a fork of vingt-2's xenogcfork fork which is a fork of the original xenogcfork published by emukidid.

This firmware for your XenoGC modchip allows you to boot into a DOL file on an SD card (in either Memory Card slot, or SP2SD2) by holding down START on the controller when you power on your GameCube with ANY readable disc in the drive.

This is achieved by replacing the multi-game disc shell/loader originally present in Xeno GC with a new custom loader that supports SD cards, FAT32 and SP2SD2.

Your SD card must be FAT32 formatted with a single partition.

The loader attempts to mount an SD card in the SP2 port, Memory Card Slot B or Memory Card Slot A (in that order) then looks for a file named ``swiss.dol`` in the root directory.

# Installation  

The simplest way to install this onto your XenoGC modchip is to use the XenoFlash software updater. Unfortunately, you still need to take your GameCube apart to get to the XenoGC modchip, and will need to solder its RST and GND pads to a switch, so the reset line of the ATmega8 can be activated on demand as required for flashing.

The alternative method is flashing the ATmega8 directly from your PC using a USB ISP programmer (a USBtinyISP or USBasp can be had for £5 on eBay) and AVRdude. You will need to solder wires to the programming pins of the ATmega8.

There's a non-zero chance a failed update via XenoFlash might brick your modchip. In that case you will need to use a USB programmer to reprogram it.

## Flashing using XenoFlash
This consists of using the XenoFlash.dol utility provided in the release. This tool is a little finicky, but if you follow the below steps carefully and in order, it should successfully update your modchip.

First, you will need to take your Gamecube apart and get to the modchip. You will then need to solder wires to the RST pad (any of the XENO letters) and the ground pad. You can either solder these wires to a switch or simply leave them bare and short them as needed ([picture of the wiring](software_installer_switch.jpg)). Once that's done:
  * Place the XenoFlash.dol on your SD/memory card, and launch it with the loader of your choice.
  * Wait until XenoFlash has fully booted and is displaying instructions.
  * Turn the switch ON (connect/short the RST and GND wires together). The red LED should turn OFF. If it doesn't, verify your wiring.
  * Press Y, this will attempt to erase the flash.
  * Turn the switch OFF (disconnect the wires). The red LED should turn ON again.
  * Turn the switch ON (connect/short the RST and GND wires together). The red LED should turn OFF again.
  * Press Y, this will actually properly erase the flash.
  * Turn the switch OFF (disconnect the wires). The red LED should be OFF.
  * Turn the switch back ON (connect/short the RST and GND wires together). The red LED should stay OFF.
  * Press A, this will flash the firmware onto the chip. This is a longer operation, wait until it says it's done.
  * Turn the switch back OFF (disconnect the wires). The red LED should turn ON again. If it does you can turn off the GameCube. If it doesn't, repeat the last 3 steps until successful.
 
At this point, turning on your GameCube with a readable disc in the drive, and holding START, should greet you with your ``swiss.dol`` loading (takes 15-30 seconds from power-on).

## Flashing using a USB programmer
You can also flash the chip directly using the ISP protocol to talk to the atmega8. The make file is configured to use a USBasp programmer, if you have a different programmer you must update the Makefile accordingly.

You will need to download [avrdude](https://www.nongnu.org/avrdude/) and place both files at the root of the repo.

Now is solder time, I like using DuPont head style cables so I can tightly connect each pin of the programmer. Once you have soldered all the wires to the PCB and connected each pin to the programmer (![picture of ISP solder points](xenogc_ISP_solder_points.png)).

Just enter ``make flash`` in a command line in the repo, (you may need to install make on Windows). This will flash xenoAT.hex that is located in XenoAT/. You can now put the optical drive back on its socket and give it a spin.

# To compile this yourself
You will need to install both [devkitppc](https://devkitpro.org/wiki/Getting_Started) and their GameCube libraries (to compile XenoLauncher and XenoFlash) as well as the [AVR 8-bit Toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers) (to compile the ATmega AVR binary).
