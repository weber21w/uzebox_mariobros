###############################################################################
# Makefile
###############################################################################

## Define the locations of folders.
GAMESRC=../src/code
GAMEINFO=../gameinfo.properties
KERNEL_DIR=../kernel/uzebox/kernel

## General Flags
GAME=MARIOBROS2018
TARGET=$(GAME).elf
MCU=atmega644
CC=avr-gcc -std=c99
# CC = avr-gcc

# Kernel settings
KERNEL_OPTIONS =

# Store the compile date and time.
KERNEL_OPTIONS += -D_COMPILEGAMENAME="\"`echo MARIO BROS 2018-2019`\""
KERNEL_OPTIONS += -D_COMPILEDATETIME1="\"`echo V:`\""
KERNEL_OPTIONS += -D_COMPILEDATETIME2="\"`date +'%Y%m%d-%I:%M%p' | tr /a-z/ /A-Z/`\""

KERNEL_OPTIONS += -DINTRO_LOGO=0

# Screen/Tile options.
KERNEL_OPTIONS += -DVIDEO_MODE=3             # Use video mode 3
KERNEL_OPTIONS += -DSCROLLING=0              # Scrolling
KERNEL_OPTIONS += -DRESOLUTION_EXT=1         # Use the new timing to allow for more square tiles.
KERNEL_OPTIONS += -DVRAM_TILES_H=24          # VRAM width.
KERNEL_OPTIONS += -DVRAM_TILES_V=28          # VRAM height. (Must match DSCREEN_TILES_V.)
KERNEL_OPTIONS += -DSCREEN_TILES_V=28        # VRAM height. (Must match VRAM_TILES_V.)
KERNEL_OPTIONS += -DFIRST_RENDER_LINE=20     # First render line.
KERNEL_OPTIONS += -DTILE_HEIGHT=8            # Tile height in pixels.
KERNEL_OPTIONS += -DTILE_WIDTH=8             # Tile width in pixels.

# Sprite/Ram tile options.
KERNEL_OPTIONS += -DTRANSLUCENT_COLOR=0x00   # Color used for the transparent pixel in sprites.
KERNEL_OPTIONS += -DRAM_TILES_COUNT=34       # Number of ram tiles available.
KERNEL_OPTIONS += -DMAX_SPRITES=17           # Number of sprites available.

# Gamepad options.
KERNEL_OPTIONS += -DCONTROLLERS_VSYNC_READ=0 # Do not read the gamepads at vsync. Must read manually.

# Networking options.
KERNEL_OPTIONS += -DUART=0                   # UART.

# Misc options
# KERNEL_OPTIONS += -DTRUE_RANDOM_GEN=1      # Use the better random number generator.

# Music options.
KERNEL_OPTIONS += -DMUSIC_ENGINE=STREAM      # Use the streaming music engine.
KERNEL_OPTIONS += -DSONG_BUFFER_SIZE=16      # Set the ram buffer size for the music data.
KERNEL_OPTIONS += -DSTREAM_MUSIC_RAM=1       # Indicate where the music data will be sourced from. (1=ram, 0=flash.)
KERNEL_OPTIONS += -DSOUND_MIXER=1            #
KERNEL_OPTIONS += -DSONG_SPEED=1             #
KERNEL_OPTIONS += -DSOUND_CHANNEL_5_ENABLE=0 #
# KERNEL_OPTIONS += -DSOUND_CHANNEL_1_ENABLE=0 #
# KERNEL_OPTIONS += -DSOUND_CHANNEL_2_ENABLE=0 #
# KERNEL_OPTIONS += -DSOUND_CHANNEL_3_ENABLE=0 #
# KERNEL_OPTIONS += -DSOUND_CHANNEL_4_ENABLE=0 #
# KERNEL_OPTIONS += -DSTREAM_MUSIC_DEBUG=1     #

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
# https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Code-Gen-Options.html
CFLAGS = $(COMMON)
CFLAGS += -DF_CPU=28636360UL
CFLAGS += -gdwarf-2
CFLAGS += -std=gnu99
CFLAGS += -Os
CFLAGS += -mcall-prologues
CFLAGS += -fshort-enums
CFLAGS += -fno-jump-tables
CFLAGS += -ffunction-sections
CFLAGS += -Wunused-but-set-variable
CFLAGS += -Wunused-function
# CFLAGS += -w
# CFLAGS += -fdata-sections

# Detect when a variable in a function isn't actually used.
CFLAGS += -Wunused-variable

# Use this one to see functions that are missing their prototype/declaration.
# CFLAGS += -Wmissing-declarations

CFLAGS += $(KERNEL_OPTIONS)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map,${GAME}.map
LDFLAGS += -Wl,-gc-sections
# LDFLAGS += -Wl,--relax
# LDFLAGS += --param inline-call-cost=2

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include Directories
INCLUDES = -I"$(KERNEL_DIR)"

## Objects that must be built in order to link
OBJECTS = uzeboxVideoEngineCore.o uzeboxCore.o uzeboxSoundEngine.o uzeboxSoundEngineCore.o uzeboxVideoEngine.o
OBJECTS +=  spiram.o
OBJECTS +=  sdBase.o
# OBJECTS +=  sdWrite.o
OBJECTS +=  $(GAME).o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: cleanit $(TARGET) $(GAME).hex $(GAME).eep $(GAME).lss $(GAME).uze size send cleanit2

## Compile Kernel files
uzeboxVideoEngineCore.o: $(KERNEL_DIR)/uzeboxVideoEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxSoundEngineCore.o: $(KERNEL_DIR)/uzeboxSoundEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxCore.o: $(KERNEL_DIR)/uzeboxCore.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxSoundEngine.o: $(KERNEL_DIR)/uzeboxSoundEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxVideoEngine.o: $(KERNEL_DIR)/uzeboxVideoEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

spiram.o: $(KERNEL_DIR)/spiram.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

sdBase.o: $(KERNEL_DIR)/sdBase.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

$(GAME).o: $(GAMESRC)/$(GAME).c
	@echo ""
	@echo -- Now compiling the game code --
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<
	@echo -- DONE compiling the game code --
	@echo ""
	@echo CC : $(CC)
	@echo INCLUDES : $(INCLUDES)
	@echo CFLAGS : $(CFLAGS)
	@echo GAMESRC : $(GAMESRC)
	@echo GAME : $(GAME).c
	@echo GAMESRC/GAME : $(GAMESRC)/$(GAME)
	@echo ""

# sdWrite.o: $(GAMESRC)/sdWrite.c
# 	$(CC) $(INCLUDES) $(CFLAGS) -c  $<


##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	@ echo "\n=================================================== hex"
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@
	@ echo "=================================================== hex\n"

%.eep: $(TARGET)
	@ echo "=================================================== eep"
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0
	@ echo "=================================================== eep\n"

%.lss: $(TARGET)
	@ echo "=================================================== lss"
	avr-objdump -h -S $< > $@
	@ echo "=================================================== lss\n"

%.uze: $(TARGET)
	@ echo "=================================================== packrom"
	./packrom $(GAME).hex $@ $(GAMEINFO)
	@ echo "=================================================== packrom"

# UNAME := $(shell sh -c 'uname -s 2>/dev/null || echo not')
# AVRSIZEFLAGS := -A ${TARGET}
# AVRSIZEFLAGS := -C ${TARGET}

# ifneq (,$(findstring MINGW,$(UNAME)))
# AVRSIZEFLAGS := -A --mcu=${MCU} ${TARGET}
# AVRSIZEFLAGS := -B --mcu=${MCU} ${TARGET}
AVRSIZEFLAGS := -C --mcu=${MCU} ${TARGET}
# endif

size: ${TARGET}
	@echo "\n=============================================================== size"
	@echo
	@echo

	#
	@ echo
	@ echo --STARTLASTBUILD.TXT--
	@ echo `date`
	@ echo ${GAME}
	@ avr-size ${AVRSIZEFLAGS}
	@ echo ---ENDLASTBUILD.TXT---
	@ echo

	# @echo --- LAST BUILD
	# @ echo "" > lastbuild.txt
	# # @ avr-nm --size-sort -t d -S -s --line-numbers -f sysv ${GAME}.elf
	# @ echo >> lastbuild.txt
	# @ echo `date` >> lastbuild.txt
	# @ echo ${GAME} >> lastbuild.txt
	# @ avr-size ${AVRSIZEFLAGS} >> lastbuild.txt
	@ echo "===============================================================  size"
	@ echo ""

## Clean target
# .PHONY: clean
send:
# Send to FFT staging area.
	@echo "STAGE: send..."
	@# pwd
	@chmod 0755 $(GAME).hex
	@chmod 0755 $(GAME).uze
	@chmod 0755 lastbuild.txt
	@chmod 0755 Makefile
	@chmod 0755 packrom
	@chmod +x packrom
	@mkdir -p output

	@echo ""
	@echo Sending output to the local game output folder.
	@( cp -f $(GAME).hex ../output/$(GAME).hex && echo SUCCESS: Move $(GAME).hex to local destination. || ERROR: Move $(GAME).hex to local destination. ) 2> /dev/null
	@( cp -f $(GAME).uze ../output/$(GAME).uze && echo SUCCESS: Move $(GAME).uze to local destination. || ERROR: Move $(GAME).uze to local destination. ) 2> /dev/null

	@echo ""

cleanit:
	echo "STAGE: cleanit1..."
	@ #echo "=============================================================== Cleaning up all the extra files from the compile!"
	rm -f $(OBJECTS) $(GAME).* dep/*
	# @ rmdir dep
	@ #echo "=============================================================== Cleaning up all the extra files from the compile!"
	@ #echo ""

cleanit2:
	@echo "STAGE: cleanit2..."

	avr-nm --size-sort --line-numbers --radix=d --print-size --format=sysv ${GAME}.elf > avr-nm.txt

	@echo ----
	@echo ----
	# @ avr-nm -t d -S -s ${GAME}.elf | grep '$(GAMESRC)' | sort > avr-nm.txt
	@echo ----
	@echo ----

	@echo "=============================================================== Cleaning up all the extra files from the compile!"
	@ rm -f $(OBJECTS) $(GAME).* dep/*
	# @ rmdir dep
	@ echo "=============================================================== Cleaning up all the extra files from the compile!"
	@ echo ""
	@# rm *.eep
	@# rm *.elf
	@# rm *.lss
	@# rm *.map
	@# rm *.o


## Other dependencies
# -include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
