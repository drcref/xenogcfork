###########################################################################################
###########################################################################################
# Makefile - XenoAT
###########################################################################################
###########################################################################################
# USER DEFINE VARIABLES
# NOTE: MAY NOT BE REQUIRED
#
# MCU    =  Target AVR < atmega8 >
#
#

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>DEVKITPPC")
endif

MCU    = atmega8
###########################################################################################
###########################################################################################
MNPREFIX= $(DEVKITPPC)/bin/mn10200-

TARGET  = XenoAT
CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size
REMOVE  = rm -f
DIR     = .
SOURCES = $(TARGET)/source
SRC     = $(SOURCES)/$(TARGET).c 
ASRC    = $(SOURCES)/qcode.S
CFLAGS  = -mmcu=$(MCU) -I. -g -O1 \
	-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
	-Wall -Wstrict-prototypes \
	-Wa,-ahlms=$(<:.c=.lst)
ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp -Wa,-ahlms=$(<:.S=.lst),-gstabs 
LDFLAGS = -Wl,-Map=$(SOURCES)/$(TARGET).map,--cref
LDFLAGS += -lm
OBJ     = $(SRC:.c=.o) $(ASRC:.S=.o) 
ELFSIZE = $(SIZE) -A $(SOURCES)/$(TARGET).elf
MOVE    = @mv $(SOURCES)/$(TARGET).hex $(TARGET)
#
LINE    = echo --------------------------------------------
BLD     = @echo;$(LINE);echo BUILDING: XENO HEX;$(LINE)
BLDELF  = @echo;$(LINE);echo CREATING: ELF;$(LINE)
BLDHEX  = @echo;$(LINE);echo CREATING: HEX;$(LINE)
BLDSIZE = @echo;$(LINE);echo AVR Memory Usage;$(LINE);$(ELFSIZE)
MD5	    = @echo;$(LINE);echo MD5SUM:; $(LINE);md5sum $(TARGET)/$(TARGET).hex
END	    = @echo;$(LINE);echo BUILDING: XENO HEX...DONE!!;$(LINE)
DUMP	= @echo;$(LINE);echo DUMPING EXTENDED DATA;$(LINE)
DUMPEND	= @echo;$(LINE);echo DUMPING EXTENDED DATA...DONE!!;$(LINE)
###########################################################################################

# Default target.
all:  clean $(SOURCES)/$(TARGET).elf $(SOURCES)/$(TARGET).hex end

end:
	$(MOVE)
	$(BLDSIZE)
	$(MD5)
	$(END)

extra: all
	$(DUMP)
	$(OBJCOPY) -j .text -j .data -O binary $(SOURCES)/$(TARGET).elf $(SOURCES)/$(TARGET).bin
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O ihex $(SOURCES)/$(TARGET).elf $(SOURCES)/$(TARGET).eep
	$(OBJDUMP) -h -S $(SOURCES)/$(TARGET).elf > $(SOURCES)/$(TARGET).lss
	$(DUMPEND)

###########################################################################################

# Assemble: create object files from assembler source files.
%.o : %.S
	$(BLD)
	$(CC) -c $(ASFLAGS) $< -o $@


# Compile: create object files from C source files.
%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@


# Link: create ELF output file from object files.
%.elf: $(OBJ)
	$(BLDELF)
	$(CC) $(CFLAGS) $(OBJ) --output $@ $(LDFLAGS)


# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	$(BLDHEX)
	$(OBJCOPY) -O ihex -R .eeprom $< $(DIR)/$@


$(SOURCES)/uploader.bin: $(SOURCES)/uploader.S
	@$(MNPREFIX)as -L $(SOURCES)/uploader.S -o $(SOURCES)/uploader.o
	@$(MNPREFIX)ld -Ttext 0x40d000 -O mn10200-elf $(SOURCES)/uploader.o -o $(SOURCES)/uploader.elf
	@$(MNPREFIX)objcopy -O $(SOURCES)/binary $(SOURCES)/uploader.elf $(SOURCES)/uploader.bin
$(SOURCES)/qcode.o: $(SOURCES)/uploader.bin

###########################################################################################
# Target: clean project.
clean: 
	$(REMOVE) $(TARGET)/$(TARGET).hex
	$(REMOVE) $(SOURCES)/$(TARGET).bin
	$(REMOVE) $(SOURCES)/*.eep
	$(REMOVE) $(SOURCES)/*.lss
	$(REMOVE) $(SOURCES)/*.map
	$(REMOVE) $(SOURCES)/*.elf
	$(REMOVE) $(SOURCES)/*.lst
	$(REMOVE) $(SOURCES)/*.o
