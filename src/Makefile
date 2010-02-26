GCC=g++

.PHONY: all clean tags pretty

all: cam_test

cam_test: cam_test.o AiboCore.o AiboNet.o AiboWalk.o AiboHead.o AiboCam.o
	$(GCC) -Wall -Wextra -g -o cam_test $?

%.o: %.cc
	@$(GCC) -o $@ -c $<

clean:
	echo "Cleaning up..."
	rm -fR *.o *dSYM

tags:
	ctags *.{cc,h}

pretty:
	astyle -A1 -f --suffix=none *.{cc,h}
	

