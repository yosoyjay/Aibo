GCC=gcc
CXX=g++
PLAYER_DIR = /usr/local/include/player-3.0/
SRC = AiboWalk.cc AiboWalk.h AiboCore.cc AiboCore.h AiboHead.cc AiboHead.h AiboNet.cc AiboNet.h unp.h RWLock.cc RWLock.h jpeg.c jpeg.h AiboCam.cc AiboCam.h dev.cc dev.h 
OBJS = RWLock.o jpeg.o dev.o AiboWalk.o AiboCore.o AiboHead.o AiboNet.o AiboCam.o

PLAYER_DRIVER = libAibo_Driver.so

.PHONY: all clean tags pretty
	  
all: $(PLAYER_DRIVER)

$(PLAYER_DRIVER): $(OBJS)
	$(CXX) -shared -I $(PLAYER_DIR) -ljpeg -lplayercore -nostartfiles -o $@ $^

%.o: %.cc %.h
	@$(CXX) -g -I $(PLAYER_DIR) -o $@ -c $<

%.o: %.c %.h
	@$(GCC) -g -o $@ -c $<

clean:
	@echo "Cleaning up..."
	rm -fR *.o *dSYM *.gch

tags:
	@echo "Refreshing tags.."
	@ctags *.{cc,h}

pretty:
	@echo "Formatting code.."
	@astyle -A1 -f --suffix=none *.{cc,h}
