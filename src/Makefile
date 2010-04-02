GCC=g++

.PHONY: all clean tags pretty

all: tags cam_test

cam_test: cam_test.o AiboCore.o AiboNet.o AiboWalk.o AiboHead.o AiboCam.o
	$(GCC) -Wall -Wextra -g -o cam_test $^

%.o: %.cc %.h
	@$(GCC) -g -o $@ -c $<

clean:
	@echo "Cleaning up..."
	rm -fR *.o *dSYM

tags:
	@echo "Refreshing tags.."
	@ctags *.{cc,h}

pretty:
	@echo "Formatting code.."
	@astyle -A1 -f --suffix=none *.{cc,h}
	

