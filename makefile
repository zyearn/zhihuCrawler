
CC=g++
CFLAGS=-g -Wall -Wextra
LIBS=-lpthread

SOURCES=$(wildcard src/*.cc)
OBJECTS=$(patsubst %.cc, objs/%.o, $(SOURCES))

TARGET=zhihuCrawler

all: $(TARGET)

$(TARGET): build $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LIBS)

objs/%.o: %.cc
	$(CC) -c -o $@ $<

build:
	@mkdir -p objs/src

clean:
	@rm -rf objs
	@rm $(TARGET)
