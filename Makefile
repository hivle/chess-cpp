CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

CORE_SRC = chess.cc posn.cc
CORE_OBJ = $(CORE_SRC:.cc=.o)
HEADERS = chess.h posn.h

# GUI: native Win32 window on Windows, X11 window elsewhere
ifeq ($(OS),Windows_NT)
    GUI_SRC = gui_win.cc
    GUI_LIBS = -lgdi32 -mwindows
else
    GUI_SRC = gui.cc
    GUI_LIBS = -lX11
endif

# Default: build both CLI and GUI
all: chess chess-gui

# CLI version
chess: $(CORE_OBJ) main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# GUI version (Win32 on Windows, X11 on Linux)
chess-gui: $(CORE_OBJ) $(GUI_SRC:.cc=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GUI_LIBS)

# Build only CLI
cli: chess

# Build only GUI
gui: chess-gui

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f chess chess-gui *.o

.PHONY: all cli gui clean
