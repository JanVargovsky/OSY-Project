all: compile archive

# create .o files
compile:
	g++ -std=c++11 -c -fPIC Message.cpp MessageHandler.cpp Semaphore.cpp

# create static library shared.a from .o files
archive:
	ar r libshared.a Message.o MessageHandler.o Semaphore.o

clean:
	-rm *.o *.a
