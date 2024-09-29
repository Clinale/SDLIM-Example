
.PHONY: clean

build: lesson.o
	g++ lesson.o -L/usr/local/SDLIM/lib -lSDL_image -lSDL_ttf -lSDLIM -lX11 -lXt -o main

lesson.o: lesson23.cpp
	g++ -I/usr/local/SDLIM/include -c lesson23.cpp -o lesson.o

clean:
	rm -f main lesson.o
