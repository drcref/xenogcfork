This is a disorganised collection of my notes and some documents that were useful to me when reversing the original code.

Most important bit to know if you want to further develop XenoLauncher:

Use a hex editor to patch XenoAT/source/qcode.bin:

Offset 7C6 -> 16bit CREDITS_SIZE value in bytes -> set this to the size of XenoLauncher.bin + 4 (or a bit more)

Offset 73A -> 16bit DELAY value in milliseconds -> set this to 3500 + (size of XenoLauncher.bin in bytes divided by 3.5)

This is to make sure that the drivecode will correctly download your entire XenoLauncher binary from the ATmega to the drive controller RAM, and then will wait long enough for the entire binary to fully upload onto system RAM when requested.
