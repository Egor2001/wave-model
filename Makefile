CC= g++
LD= $(CC)

TARGET= bin/main

SRCFILES= src/main.cpp src/core.cpp
OBJFILES= $(patsubst src/%.cpp,obj/%.o,$(SRCFILES))

CFLAGS= -c -std=c++2a -Isrc
LFLAGS=

ifdef RELEASE
    CFLAGS += -Ofast
else
    CFLAGS += -ggdb -O0 -Wall -Wextra
endif

.PHONY: run clean

all: bin $(OBJFILES)
	$(LD) $(LFLAGS) $(OBJFILES) -o $(TARGET)

obj/main.o: obj src/main.cpp
	$(CC) $(CFLAGS) src/main.cpp -o obj/main.o

obj/core.o: obj src/core.cpp
	$(CC) $(CFLAGS) src/core.cpp -o obj/core.o

bin:
	mkdir -p bin

obj:
	mkdir -p obj

test:
	mkdir -p test

run: $(TARGET) test
	./$(TARGET) > test/out.txt
	python src/visualize.py test/out.txt test/fig.png

clean:
	rm -rf obj/*.o
