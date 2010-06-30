GC=gcc
CXX=g++
PLAYER_DIR = /usr/local/include/player-3.0/
SRC = AiboWalk.cc AiboWalk.h AiboCore.cc AiboCore.h AiboHead.cc AiboHead.h AiboNet.cc AiboNet.h unp.h RWLock.cc RWLock.h jpeg.c jpeg.h AiboCam.cc AiboCam.h dev.cc dev.h PosixTimer.cpp PosixTimer.h
OBJS = RWLock.o jpeg.o dev.o AiboWalk.o AiboCore.o AiboHead.o AiboNet.o AiboCam.o AiboState.o PosixTimer.o

PLAYER_DRIVER = libAibo_Driver.so

.PHONY: all clean tags pretty
	  
all: $(PLAYER_DRIVER)

$(PLAYER_DRIVER): $(OBJS)
ifeq ($(VENDOR),apple)
	$(CXX) -shared -nostartfiles -I $(PLAYER_DIR) -ljpeg -lplayercore -o $@ $^
else
	$(CXX) -shared -nostartfiles -o $@ $^
endif

%.o: %.cc %.h
ifeq ($(VENDOR),apple)
	@$(CXX) -Wall -fpic -g3 -g -I $(PLAYER_DIR) -o $@ -c $<
else
	@$(CXX) -Wall -fpic -g3 `pkg-config --cflags playercore` -c $(SRC)
endif

%.o: %.c %.h
ifeq ($(VENDOR),apple)
	@$(GCC) -Wall -fpic -g3 -g -o $@ -c $<
else
	@$(GCC) -Wall -fpic -g3 `pkg-config --cflags playercore` -c  $(SRC)
endif

clean:
	@echo "Cleaning up..."
	rm -fR *.o *dSYM *.gch

tags:
	@echo "Refreshing tags.."
	@ctags *.{cc,h}

pretty:
	@echo "Formatting code.."
	@astyle -A1 -f --suffix=none *.{cc,h}
	@echo $(VENDOR)
