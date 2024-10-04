
#########
#linux x86 and linux x64
# (sudo apt-get install libsdl2-2.0-0)
# (sudo apt-get install libsdl2-dev)
# sudo apt-get install build-essential

TARGET_OS = $(shell uname)
TARGET_ARCH = $(shell uname -m)
#supported are
#TARGET_OS = Linux     //linux OS
#TARGET_ARCH = x86_64  //64 bit x86
#TARGET_ARCH = i686    //32 bit x86

#########
#setup

CC = g++
COMPILERFLAGS = -Wall

ifeq ($(TARGET_OS),Linux)
 LDFLAGS_PLAIN = -lpthread
 ifeq ($(TARGET_ARCH),x86_64)
  LD_LIBSDL = ../../lib/linux_x64/libSDL2.so
  EXE_PATH = ../../exe/linux_x64/
  LDFLAGS_GFX += -L../../lib/linux_x64 -lSDL2 '-Wl,-R,$$ORIGIN/.'
 else ifeq ($(TARGET_ARCH),i686)
  LD_LIBSDL = ../../lib/linux_x86/libSDL2.so
  EXE_PATH = ../../exe/linux_x86/
  LDFLAGS_GFX += -L../../lib/linux_x86 -lSDL2 '-Wl,-R,$$ORIGIN/.'
 else
  LD_LIBSDL = ./boulder
  EXE_PATH = ../../exe/linux/
  LDFLAGS_GFX += -lSDL2
 endif
 LDFLAGS_GFX_GL = -lGL -L/usr/X11R6/lib -lX11
 CFLAGS = $(COMPILERFLAGS) -I../../src/common -I../../src/boulder -I../../src/graph -I../../ext_include/pa -I../../ext_include/sdl
endif

#########
#objects

GRAPH_OBJECTS = ../../src/graph/bmp.o ../../src/graph/targa.o ../../src/graph/jpeg.o \
	../../src/graph/converter.o ../../src/graph/image.o ../../src/graph/font.o \
	../../src/graph/graph.o ../../src/graph/imagediskop.o

BOULDER_OBJECTS = ../../src/boulder/boulder.o ../../src/boulder/game_object.o ../../src/boulder/game_world.o \
	../../src/boulder/hiscorelist.o ../../src/common/gui.o ../../src/common/tileset.o \
	../../src/common/global.o ../../src/common/fileresource.o ../../src/common/timer.o \
	../../src/common/common.o ../../src/common/input.o ../../src/common/soundfx.o \
	$(GRAPH_OBJECTS)
BOULDER_OBJECTS += ../../ext_include/pa/stb_vorbis.o
BOULDER_OBJECTS += ../../ext_include/pa/pa_ringbuffer.o
EDITOR_OBJECTS   = ../../src/editor/mapedit.o ../../src/editor/editor_object.o ../../src/editor/editor_world.o \
	../../src/common/tileset.o ../../src/common/global.o ../../src/common/fileresource.o \
	../../src/common/timer.o ../../src/common/input.o ../../src/common/gui.o \
	$(GRAPH_OBJECTS)
RES_COMP_OBJECTS = ../../utils/res_comp/res_comp.o ../../src/common/fileresource.o
IMG_CUT_OBJECTS  = ../../utils/img_cut/img_cut.o   ../../src/common/fileresource.o $(GRAPH_OBJECTS)
FNT_COMP_OBJECTS = ../../other/fnt_comp/fnt_comp.o ../../src/common/fileresource.o \
	../../other/fnt_comp/font.o ../../other/fnt_comp/glfont.o ../../other/fnt_comp/glgraph.o \
	../../other/fnt_comp/glimage.o ../../other/fnt_comp/glconverter.o ../../other/fnt_comp/gltarga.o

#########
#this will build a .cpp into .o
# make a .d dependency file
# fix so that a new or renamed .h file will cause a rebuild
# and finally fix paths in the .d file
%.o: %.cpp
	$(CC) -c $(CFLAGS) $*.cpp -o $*.o
	$(CC) -MM $(CFLAGS) $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

#########
#targets
all: boulder mapeditor install

utils: ../../utils/res_comp/res_comp ../../utils/fnt_comp/fnt_comp ../../utils/img_cut/img_cut

clean:
	rm -f ../../ext_include/pa/*.o ../../ext_include/pa/*.d
	rm -f ../../src/common/*.o ../../src/common/*.d
	rm -f ../../src/graph/*.o ../../src/graph/*.d
	rm -f ../../src/boulder/*.o ../../src/boulder/*.d
	rm -f ../../src/editor/*.o ../../src/editor/*.d
	rm -f ./boulder
	rm -f ./mapeditor
	rm -f ../../utils/res_comp/*.o ../../utils/res_comp/*.d
	rm -f ../../utils/fnt_comp/*.o ../../utils/fnt_comp/*.d
	rm -f ../../utils/img_cut/*.o ../../utils/img_cut/*.d
	rm -f ../../utils/res_comp/res_comp
	rm -f ../../utils/fnt_comp/fnt_comp
	rm -f ../../utils/img_cut/img_cut

#########
#private

install:
	mkdir -p $(EXE_PATH)
	cp ./boulder   $(EXE_PATH)
	cp ./mapeditor  $(EXE_PATH)
	cp $(LD_LIBSDL) $(EXE_PATH)

boulder: $(BOULDER_OBJECTS)
	$(CC) -o ./boulder $(BOULDER_OBJECTS) $(LDFLAGS_PLAIN) $(LDFLAGS_GFX)
-include $(BOULDER_OBJECTS:.o=.d)

mapeditor: $(EDITOR_OBJECTS)
	$(CC) -o ./mapeditor $(EDITOR_OBJECTS) $(LDFLAGS_PLAIN) $(LDFLAGS_GFX)
-include $(EDITOR_OBJECTS:.o=.d)

../../utils/res_comp/res_comp: $(RES_COMP_OBJECTS)
	$(CC) -o ../../utils/res_comp/res_comp $(RES_COMP_OBJECTS) $(LDFLAGS_PLAIN)
-include $(RES_COMP_OBJECTS:.o=.d)

../../utils/img_cut/img_cut: $(IMG_CUT_OBJECTS)
	$(CC) -o ../../utils/img_cut/img_cut $(IMG_CUT_OBJECTS) $(LDFLAGS_PLAIN) $(LDFLAGS_GFX)
-include $(IMG_CUT_OBJECTS:.o=.d)

../../utils/fnt_comp/fnt_comp: $(FNT_COMP_OBJECTS)
	$(CC) -o ../../utils/fnt_comp/fnt_comp $(FNT_COMP_OBJECTS) $(LDFLAGS_PLAIN) $(LDFLAGS_GFX_GL)
-include $(FNT_COMP_OBJECTS:.o=.d)
