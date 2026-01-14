# compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -O2 -Wall -Werror

# source files
SRCS = main.cpp \
	   ALSAPlayer.cpp

# object files
OBJS = $(SRCS:.cpp=.o)

# executable file
TARGET = audio-player

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
