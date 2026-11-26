CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

CORE_SRC = chess.cc posn.cc
CORE_OBJ = $(CORE_SRC:.cc=.o)
HEADERS = chess.h posn.h

X11_LIBS = -lX11

# Default: build both CLI and GUI
all: chess chess-gui

# CLI version
chess: $(CORE_OBJ) main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# GUI version (X11)
chess-gui: $(CORE_OBJ) gui.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(X11_LIBS)

# Build only CLI
cli: chess

# Build only GUI
gui: chess-gui

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f chess chess-gui *.o

.PHONY: all cli gui clean
