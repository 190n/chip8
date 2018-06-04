CXX=/usr/bin/g++
CXXFLAGS=-O3
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system
OBJS=chip8.o gfx.o input.o main.o

all: $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o chip8

clean:
	rm -f $(OBJS) chip8
