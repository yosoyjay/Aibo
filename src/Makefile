SRC = aibo_driver.cc aibo_driver.h aibo_comms.c aibo_comms.h unp.h 
OBJLIBS = libAibo_Driver.so
OBJS = aibo_driver.o aibo_comms.o 
INCLUDE = "/usr/local/include/player-3.0/"
LIB = "/usr/local/lib"


all: $(OBJLIBS)

$(OBJS): $(SRC)
	echo "Building the AiboDriver plugin..."
	$(CXX) -Wall -g -m32 -fpic -I $(INCLUDE) -g3 -c $(SRC)

$(OBJLIBS): $(OBJS)
	$(CXX) -shared -nostartfiles -L$(LIB) -lplayercore.3.0 -lplayercommon -o $@ $^

clean:
	echo "Cleaning up the AiboDriver plugin..."
	rm -f $(OBJS) $(OBJLIBS) *.gch
