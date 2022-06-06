###########################################################################################
###########################################################################################
#  MASTER MAKEFILE
###########################################################################################
###########################################################################################

include Makefile-help
include Makefile-avrdude

all: shell hex

shell:
	$(MAKE) -f XenoLauncher/Makefile
	cp XenoLauncher/XenoLauncher.bin XenoAT/source/XenoLauncher.bin
hex:
	$(MAKE) -f XenoAT/Makefile

extra:
	$(MAKE) -f XenoLauncher/Makefile
	$(MAKE) extra -f XenoAT/Makefile

###########################################################################################
###########################################################################################
# Clean WINDOWS / LINUX
clean:
	make clean -f XenoAT/Makefile
	make clean -f XenoLauncher/Makefile
	rm -f XenoAT/source/XenoLauncher.bin

###########################################################################################
###########################################################################################
# AVRDUDE
flash: 
	$(AVRDUDE) $(FLAGS) -v -U flash:w:$(TARGET)/$(TARGET).hex

readfuse:
	uisp -dprog=dasa2 -dserial=$(PORT) --rd_fuses

writefuse:
	$(AVRDUDE) $(FLAGS) -v -U lfuse:w:0xC4:m -U hfuse:w:0xD9:m
	
help : .help
