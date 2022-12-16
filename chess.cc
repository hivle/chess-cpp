#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include "posn.h"
#include "chess.h"

State::State():
whiteTurn(true),
whiteCastle(true),
blackCastle(true),
whiteCastleLong(true),
blackCastleLong(true),
repeatedMoves(0),
chessBoard({
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
}){}

bool State::operator==(const State & other)
{
    if (whiteTurn != other.whiteTurn) return false;
    if (whiteCastle != other.whiteCastle) return false;
    if (blackCastle != other.blackCastle) return false;
    if (whiteCastleLong != other.whiteCastleLong) return false;
    if (blackCastleLong != other.blackCastleLong) return false;
    if (repeatedMoves != other.repeatedMoves) return false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (chessBoard[i][j] != other.chessBoard[i][j]) return false;
        }
    }
    return true;
}

Board::Board()
{   
    State state;
    draw = false;
    whiteWin = false;
}

Board::~Board() {}

char* Board::getPos(posn tar)
{
    return &state.chessBoard[tar.row][tar.col];
}

std::ostream& operator<<(std::ostream& out, const Board& b)
{
    for (int i = 0; i < 8; i++) {
        int row = 8 - i;
        out <<  "   ---------------------------------\n " << row << " | ";
        for (int j = 0; j < 8; j++) {
            char piece = b.state.chessBoard[i][j];
            if (piece == '0') piece = ' ';
            out << piece << " | ";
        }
        out << "\n";
    }
    out << "   ---------------------------------\n     a   b   c   d   e   f   g   h\n";
    return out;
}

 bool Board::isEnemy(posn tar1, posn tar2)
 {
    char start = *getPos(tar1);
    char target = *getPos(tar2);
    return (start != '0' && target != '0' && abs(start - target) > ('z' - 'a')) ? true : false;
 }


//true if successful moved
bool Board::movePiece(posn ini, posn tar)
{
    char *iniPos = getPos(ini);
    char *tarPos = getPos(tar);
    if (isEnemy(ini,tar) || *tarPos == '0'){
        history.push_back(state);
        *tarPos = *iniPos;
        *iniPos = '0';
        if (std::count(history.begin(), history.end(), state) >= 3){
            draw = true;
        }
        return true;
    }
    return false;
}

bool Board::revert()
{
    if (history.empty()) return false;
    state = history.back();
    history.pop_back();
    return true;
}

std::string Board::getDir(posn p, Direction d)
{
    int row = p.row;
    int col = p.col;
    row += d / 3 - 1;
    col += d % 3 - 1;
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return "";
    }
    p.row = row;
    p.col = col;
    return p.name();
}
// incorporate smart pointers when possible