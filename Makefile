#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment. export PSL1GHT=<path>")
endif

include $(PSL1GHT)/ppu_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
RESOURCELIST	:= source/resources/resourcelist.cpp
RESOURCES	:=  source/images \
				source/sounds
SOURCES		:=	source \
				source/resources \
				$(RESOURCES)
DATA		:=	data
INCLUDES	:=	include


TITLE		:=	PuneFlow
APPID		:=	PUNEFLOW0
CONTENTID	:=	UP0001-$(APPID)_00-0000000000000000
PKGFILES	:=	release
SFOXML		:=	$(CURDIR)/sfo.xml
ICON0		:=	$(CURDIR)/ICON0.PNG

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS		=	-O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE) -DCONTENT_ID=\"$(CONTENTID)\" -DAPP_ID=\"$(APPID)\"
CXXFLAGS	=	$(CFLAGS) -DCONTENT_ID=\"$(CONTENTID)\" -DAPP_ID=\"$(APPID)\"

LDFLAGS		=	$(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
AUDIOLIBS	:=  -lspu_sound -laudioplayer -lmpg123 -lvorbisfile -lvorbis -logg -laudio -lspu_soundmodule.bin
LIBS	:=	$(AUDIOLIBS) -ltiny3d -lfreetype -lz -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm -lsysfs -lpngdec -ljpgdec -lsysmodule

LIBDIRS	:=	$(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
TTFFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
PCMFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pcm)))
OGGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ogg)))
MP3FILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.mp3)))
WAVFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.wav)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					resourcelist.o \
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(PCMFILES:.pcm=.pcm.o) $(PNGFILES:.png=.png.o) \
					$(OGGFILES:.ogg=.ogg.o) $(MP3FILES:.mp3=.mp3.o) \
					$(WAVFILES:.wav=.wav.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(LIBPSL1GHT_INC) \
					-I$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					$(LIBPSL1GHT_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean resources

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory resources
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).self


resources:
	@./resourceLister $(RESOURCELIST) $(RESOURCES)


#---------------------------------------------------------------------------------
run:
	ps3load $(OUTPUT).self

pkg: $(BUILD) $(OUTPUT).pkg
#pkg: $(BUILD)
#	@echo Creating PKG...
#	@mkdir -p $(BUILD)/pkg
#	@mkdir -p $(BUILD)/pkg/USRDIR
#	@cp $(ICON0) $(BUILD)/pkg/
#	@$(FSELF) -n $(BUILD)/$(TARGET).elf $(BUILD)/pkg/USRDIR/EBOOT.BIN
#	@$(SFO) --title "$(TITLE)" --appid "$(APPID)" -f $(SFOXML) $(BUILD)/pkg/PARAM.SFO
#	@$(PKG) --contentid $(CONTENTID) $(BUILD)/pkg/ $(OUTPUT).pkg


#---------------------------------------------------------------------------------
else

DEPENDS	:= $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with the .bin extension
#---------------------------------------------------------------------------------
%.bin.o : %.bin
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.ttf.o : %.ttf
		@echo [BIN2S] $(notdir $<)
		@bin2s -a 32 $< | $(AS) -o $(@)

%.png.o : %.png
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.pcm.o : %.pcm
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.ogg.o : %.ogg
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.mp3.o : %.mp3
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.wav.o : %.wav
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

%.SFO.o : %.SFO
	@echo [BIN2S] $(notdir $<)
	@bin2s -a 32 $< | $(AS) -o $(@)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

