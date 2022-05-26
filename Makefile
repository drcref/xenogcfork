###########################################################################################
###########################################################################################
#  MASTER MAKEFILE
###########################################################################################
###########################################################################################

include Makefile-help
include Makefile-avrdude

all: help
linux-all: linux-shell linux-hex
windows-all: windows-shell windows-hex

###########################################################################################
###########################################################################################
# LINUX
linux-shell:
	make -f XenoLauncher/Makefile-LINUX

linux-hex:
	make -f XenoAT/Makefile-LINUX

linux-extra:
	make -f XenoLauncher/Makefile-LINUX
	make extra -f XenoAT/Makefile-LINUX

###########################################################################################
###########################################################################################
# WINDOWS
windows-shell:
	$(MAKE) -f XenoLauncher/Makefile-WIN
	cp XenoLauncher/XenoLauncher.bin XenoAT/source/XenoLauncher.bin
windows-hex:
	$(MAKE) -f XenoAT/Makefile-WIN

windows-extra:
	$(MAKE) -f XenoLauncher/Makefile-WIN
	$(MAKE) extra -f XenoAT/Makefile-WIN

###########################################################################################
###########################################################################################
# Clean WINDOWS / LINUX
clean:
	make clean -f XenoAT/Makefile-LINUX
	make clean -f XenoLauncher/Makefile-LINUX

###########################################################################################
###########################################################################################
# AVRDUDE
flash: 
	$(AVRDUDE) $(FLAGS) -v -U flash:w:$(TARGET)/$(TARGET).hex

readfuse:
	uisp -dprog=dasa2 -dserial=$(PORT) --rd_fuses

writefuse:
	$(AVRDUDE) $(FLAGS) -v -U lfuse:w:0xC4:m -U hfuse:w:0xD9:m

###########################################################################################
###########################################################################################
# HELP
help:
