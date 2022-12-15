#include <string>
#include <iostream>
#include <vector>
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

//true if white, false if black
// bool Board::isWhite(posn tar) {
//     char temp = getPos(tar);
//     if ('A' <= temp <= 'Z'){
//         return true;
//     }
//     else return false;
// }

// char Board::getPos(posn tar) {
//     return chessBoard[tar.row][tar.col];
// }

//true if successful moved
// bool Board::movePiece(posn ini, posn des) {
//     char& tar = getPos(ini);
//     if (tar == ' ' || tar == '_') return false;
//     posn* allow = allowed(tar);
//     //remember to delete, use valgrind
//     for (int i = 0; i < 27; i++) {
//         if (des == allow[i]) {
//             getPos(des) = tar;
//             if ((ini.col - 'a' + '8' - ini.row) % 2 == 0) {
//                 tar = ' ';
//             }   
//             else tar = '_';
//             return true;
//         }
//     }
//     return false;
// }
