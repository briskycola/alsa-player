# compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Werror

# source files
SRCS = alsa-player.cpp

# object files
OBJS = $(SRCS:.cpp=.o)

# executable file
TARGET = alsa-player

# default rule
all: $(TARGET)

# link the object files to create binary
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) -lasound -lsndfile

# compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean up object files and binary
clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean
