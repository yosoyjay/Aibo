GCC=g++
SRC = AiboWalk.cc AiboWalk.h AiboCore.cc AiboCore.h AiboHead.cc AiboHead.h AiboNet.cc AiboNet.h unp.h RWLock.cpp RWLock.h jpeg.c jpeg.h AiboCam.cc AiboCam.h dev.cpp dev.h 
OBJS = RWLock.o jpeg.o dev.o AiboWalk.o AiboCore.o AiboHead.o AiboNet.o AiboCam.o

OBJLIBS = libAibo_Driver.so

.PHONY: all clean tags pretty
	  
all: $(OBJLIBS)

$(OBJLIBS): $(OBJS)
	$(CXX) -shared -ljpeg -lplayercore -nostartfiles -o $@ $^

%.o: %.cc %.h
	@$(GCC) -g -o $@ -c $<

%.o: %.c %.h
	gcc -g -o $@ -c $<

clean:
	@echo "Cleaning up..."
	rm -fR *.o *dSYM *.gch

tags:
	@echo "Refreshing tags.."
	@ctags *.{cc,h}

pretty:
	@echo "Formatting code.."
	@astyle -A1 -f --suffix=none *.{cc,h}
	

