#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
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

bool Board::isWhite(posn tar) 
{
    char start = *getPos(tar);
    char whitePiece[] = {'P','R','N','B','Q','K'};
    for(char piece : whitePiece){
        if (piece == start) return true;
    }
    return false;
}

bool Board::isBlack(posn tar)
{
    char start = *getPos(tar);
    char blackPiece[] = {'p','r','n','b','q','k'};
        for(char piece : blackPiece){
        if (piece == start) return true;
    }
    return false;
}

bool Board::isEnemy(posn tar1, posn tar2)
{
    return ((isWhite(tar1) && isBlack(tar2)) || (isWhite(tar2) && isBlack(tar1)));
}

void Board::pawnHelper(posn loc, std::vector<posn> free, std::vector<posn> attack){
    posn u1 = loc.goDir(loc, up);
    posn u2 = loc.goDir(u1, up);
    posn ur = loc.goDir(loc, upright);
    posn ul = loc.goDir(loc, upleft);
    free.push_back(u1);
    free.push_back(u2);
    attack.push_back(ur);
    attack.push_back(ul);
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

// incorporate smart pointers when possible