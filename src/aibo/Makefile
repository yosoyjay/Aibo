OBJS = _aibo.so

include ../../Makefile.cfg
include ../../Makefile.src

INCLUDE = -I../device/
CPPFLAGS = -Wall -Wno-unused -D_POSIX_THREADS \
	-D_POSIX_THREAD_SAFE_FUNCTIONS \
	-D_REENTRANT -DPOSIX \
	-D__x86__ -D__linux__ \
	-D__OSVERSION__=2 -frepo -DUSINGTHREADS -DLINUX -D_GNU_SOURCE \
	$(PYTHON_INCLUDE) $(INCLUDE)

LIBDIRS= 
LIBS= -lstdc++ -ldl -lpthread -ljpeg

../device/Device.o: ../device/Device.h ../device/Device.cpp
	cd ../device/; make

_aibo.so: aibo.o Aibo.o Socket.o jpeg.o RWLock.o
	$(CC) $(CPPFLAGS) -shared aibo.o Aibo.o Socket.o jpeg.o RWLock.o ../device/Device.o $(LIBDIRS) -o $@ $(LIBS)

RWLock.o: RWLock.cpp RWLock.h
	$(CC) RWLock.cpp -c

jpeg.o: jpeg.c jpeg.h
	$(CC) jpeg.c -c

Aibo.o: Aibo.cpp Aibo.h ../device/Device.o
	$(CC) $(CPPFLAGS) -o $@ -c Aibo.cpp

Aibo.cc: Aibo.i Aibo.h ../device/Device.o
	swig -python -c++ $(INCLUDE) -o Aibo.cc Aibo.i

aibo.o: Aibo.cc 
	$(CC) -c $(PYTHON_INCLUDE) $(INCLUDE) Aibo.cc -o $@

Socket.o: Socket.cpp Socket.h
	$(CC) -c $(PYTHON_INCLUDE) $(INCLUDE) Socket.cpp -o $@

clean::
	$(RM) *~ *.so *.o Aibo.cc aibo.py

