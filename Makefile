# Makefile cho project AI City Defense

# Compiler
CC = g++

# Flags
CFLAGS = -Wall -I src/include

# Linker flags
LDFLAGS = -Lsrc/lib -lSDL2 -lmingw32 -lSDL2main

# Tên file thực thi
TARGET = CityDefense

# File nguồn
SOURCES = src/main.cpp  # Chỉ định rõ đường dẫn đến main.cpp

# File đối tượng
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean