#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <stack>
#include "posn.h"
#include "chess.h"

State::State():
whiteTurn(true),
whiteCastle(true),
blackCastle(true),
whiteCastleLong(true),
blackCastleLong(true),
repeatedMoves(0)
{
    pastBoard =
    {
        {'r','n','b','q','k','b','n','r'},
        {'p','p','p','p','p','p','p','p'},
        {'0','0','0','0','0','0','0','0'},
        {'0','0','0','0','0','0','0','0'},
        {'0','0','0','0','0','0','0','0'},
        {'0','0','0','0','0','0','0','0'},
        {'P','P','P','P','P','P','P','P'},
        {'R','N','B','Q','K','B','N','R'}
    };
}

Board::Board()
{   
    State state;
    chessBoard = state.pastBoard;
}

Board::~Board() {}

char* Board::getPos(posn tar)
{
    return &chessBoard[tar.row][tar.col];
}

std::ostream& operator<<(std::ostream& out, const Board& b)
{
    for (int i = 0; i < 8; i++) {
        int row = 8 - i;
        out <<  "   ---------------------------------\n " << row << " | ";
        for (int j = 0; j < 8; j++) {
            char piece = b.chessBoard[i][j];
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
        history.push(state);
        *tarPos = *iniPos;
        *iniPos = '0';
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                state.pastBoard[i][j] = chessBoard[i][j];
            }
        }
        return true;
    }
    return false;
}

bool Board::revert()
{
    if (history.empty()) return false;
    State temp = history.top();
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            chessBoard[i][j] = temp.pastBoard[i][j];
        }
    }
    history.pop();
    return true;
}

// incorporate smart pointers
