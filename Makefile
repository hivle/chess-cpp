CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

CORE_SRC = chess.cc posn.cc
CORE_OBJ = $(CORE_SRC:.cc=.o)
HEADERS = chess.h posn.h

# On Windows, name the programs *.exe and link the C++ runtime statically
# so chess.exe runs on its own (double-click works, no missing-DLL errors).
ifeq ($(OS),Windows_NT)
    EXE = .exe
    LDFLAGS = -static
    RM = del /Q
else
    EXE =
    LDFLAGS =
    RM = rm -f
endif

CLI = chess$(EXE)
GUI = chess-gui$(EXE)

# Default: build both CLI and GUI (the GUI needs X11, so Linux only)
all: $(CLI) $(GUI)

# Build only CLI
cli: $(CLI)

# Build only GUI
gui: $(GUI)

$(CLI): $(CORE_OBJ) main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(GUI): $(CORE_OBJ) gui.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lX11

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) chess chess-gui chess.exe chess-gui.exe *.o

.PHONY: all cli gui clean
