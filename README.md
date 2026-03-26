# chess-cpp

A chess game written in C++ with a terminal CLI and an X11 graphical interface.

## Build

```bash
make        # build both CLI and GUI
make cli    # CLI only
make gui    # GUI only
make clean  # remove build artifacts
```

Requires `g++` (C++17), `make`, and `libX11-dev` for the GUI.

## Play

### CLI
```bash
./chess
```

| Input | Action |
|-------|--------|
| `e2e4` | Move piece from e2 to e4 |
| `e7e8q` | Move with pawn promotion (q/r/b/n) |
| `e2` | Show legal moves for piece at e2 |
| `z` / `back` / `undo` | Undo last move |
| `h` / `help` | Show help |
| `q` / `exit` | Quit |

Legal moves are highlighted on the board — green for free squares, red for captures.

### GUI
```bash
./chess-gui
```

If you get `cannot open display`, run:
```bash
export DISPLAY=:0 && ./chess-gui
```

| Action | Control |
|--------|---------|
| Select piece | Left-click |
| Move piece | Left-click a highlighted square |
| Deselect | Right-click |
| Undo | `Z` |
| Quit | `Esc` |

Click a piece to see its legal moves highlighted (green dots = free, red border = capture). Pawn promotion opens a dialog to choose the piece.

## Rules supported

- All standard moves (pawns, knights, bishops, rooks, queens, kings)
- Castling (kingside and queenside)
- En passant
- Pawn promotion
- Check and checkmate detection
- Stalemate detection
- Fifty-move draw rule
- Threefold repetition draw rule
