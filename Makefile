# Makefile cho project City Defense

# Compiler
CC = x86_64-w64-mingw32-g++

# Flags
CFLAGS = -Wall -I src/include

# Linker flags (bao gồm SDL2_mixer)
LDFLAGS = -Lsrc/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_mixer -lucrt

# Tên file thực thi
TARGET = CityDefense

# Danh sách file nguồn (bao gồm các file bổ sung Vector2D.cpp và MathAddon.cpp)
SOURCES = src/main.cpp \
          src/Game.cpp \
          src/Level.cpp \
          src/Timer.cpp \
          src/TextureLoader.cpp \
          src/SoundLoader.cpp \
          src/Turret.cpp \
          src/Unit.cpp \
          src/Projectile.cpp \
          src/Vector2D.cpp \
          src/MathAddon.cpp

# Tạo danh sách file đối tượng từ danh sách file nguồn
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
