SRC = AiboWalk.cc AiboWalk.h AiboCore.cc AiboCore.h AiboHead.cc AiboHead.h AiboNet.cc AiboNet.h unp.h aibo/Socket.cpp aibo/Socket.h aibo/RWLock.cpp aibo/RWLock.h aibo/jpeg.c aibo/jpeg.h aibo/Aibo.cpp aibo/Aibo.h aibo/dev.cpp aibo/dev.h 
	  
OBJLIBS = libAibo_Driver.so

OBJS = Aibo.o Socket.o RWLock.o jpeg.o dev.o AiboWalk.o AiboCore.o AiboHead.o AiboNet.o

all: $(OBJLIBS)

$(OBJS): $(SRC)
	echo "Building the AiboDriver plugin..."
	$(CXX) -Wall -fpic -g3 `pkg-config --cflags playercore` -c -g $(SRC)

$(OBJLIBS): $(OBJS)
	$(CXX) -shared -nostartfiles -o $@ $^

clean:
	echo "Cleaning up the AiboDriver plugin..."
	rm -f $(OBJS) $(OBJLIBS) 
