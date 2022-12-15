#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include "posn.h"
#include "chess.h"

Board::Board()
{   
    char newBoard[8][8] =
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
    chessBoard = new char*[8];
    for (int i = 0; i < 8; i++) {
        chessBoard[i] = new char[8];
        for (int j = 0; j < 8; j++) {
            chessBoard[i][j] = newBoard[i][j];
        }
    }
}

Board::~Board() 
{
    for (int i = 0; i < 8; i++) {
        delete[] chessBoard[i];
    }
    delete[] chessBoard;
}

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
    if (start != '0' && target != '0' && abs(start - target) > ('z' - 'a')){
        return true;
    }
    return false;
}

//true if successful moved
bool Board::movePiece(posn ini, posn tar)
{
    char *iniPos = getPos(ini);
    char *tarPos = getPos(tar);
    if (isEnemy(ini,tar) || *tarPos == '0'){
        *tarPos = *iniPos;
        *iniPos = '0';
        return true;
    }
    return false;
}
