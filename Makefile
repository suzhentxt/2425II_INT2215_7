CXX = g++
CXXFLAGS = -Wall -I src/include 
LDFLAGS = -L src/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
