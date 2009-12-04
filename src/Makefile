SRC = aibo_driver.cc aibo_driver.h aibo_comms.c aibo_comms.h 
OBJLIBS = libAibo_Driver.so
OBJS = aibo_driver.o aibo_comms.o 

all: $(OBJLIBS)

$(OBJS): $(SRC)
	echo "Building the AiboDriver plugin..."
	$(CXX) -Wall -fpic -g3 `pkg-config --cflags playercore` -c $(SRC)

$(OBJLIBS): $(OBJS)
	$(CXX) -shared -nostartfiles -o $@ $^

clean:
	echo "Cleaning up the AiboDriver plugin..."
	rm -f $(OBJS) $(OBJLIBS) *.gch
