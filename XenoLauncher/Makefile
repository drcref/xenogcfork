###########################################################################################
###########################################################################################
# Makefile - XenoLauncher
###########################################################################################
###########################################################################################

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>DEVKITPPC")
endif

include $(DEVKITPPC)/base_tools

TARGET  = XenoLauncher
SOURCES = $(TARGET)/source
FLAGS   = -nostdlib -fno-builtin -Wall -Os -pipe -Wl,-s,-x,--gc-sections -Wa,-mregnames -fno-exceptions\
		  -mhard-float -Wl,-T,$(SOURCES)/first.lds -Wl,-Map=$(SOURCES)/$(TARGET).map
REMOVE  = rm -f
LINE    = echo --------------------------------------------
BLD     = @echo;$(LINE);echo BUILDING: XENO LAUNCHER;$(LINE)
END		= @echo;$(LINE);echo BUILDING: XENO LAUNCHER...DONE!!;$(LINE)
MOVE	= mv $(SOURCES)/$(TARGET).bin $(TARGET)

all: clean build move

build:
	$(BLD)
	$(CC) $(FLAGS) $(SOURCES)/boot.s $(SOURCES)/main.c $(SOURCES)/exi.c $(SOURCES)/ulibsd/sd_io.c $(SOURCES)/ulibsd/spi_io.c $(SOURCES)/pff/diskio.c $(SOURCES)/pff/pff.c -lgcc -o $(SOURCES)/$(TARGET).elf
	$(OBJCOPY) -O  binary $(SOURCES)/$(TARGET).elf $(SOURCES)/$(TARGET).bin

move:
	$(MOVE)
	$(END)

clean:
	$(REMOVE) $(TARGET)/$(TARGET).bin
	$(REMOVE) $(SOURCES)/$(TARGET).elf
	$(REMOVE) $(SOURCES)/$(TARGET).map
	
