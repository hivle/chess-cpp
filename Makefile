# Specify the target executable
TARGET = main

# Specify the C++ source files
SOURCES = chess.cc main.cc posn.cc

# Specify the header files
HEADERS = chess.h posn.h

# Specify any additional libraries to link with
LIBS =

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

# Build the target by linking the object files
$(TARGET): $(SOURCES:.cc=.o)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES:.cc=.o) $(LIBS)

# Compile the source files
%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the built files
clean:
	rm -f $(TARGET) $(SOURCES:.cc=.o)
