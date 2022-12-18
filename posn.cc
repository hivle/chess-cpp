#include <string>
#include "posn.h"

// assumes valid chess position
posn::posn(std::string sq){
    row = 8 - (sq[1] - '1' + 1);
    col = sq[0] - 'a';
    onBoard = true;
}

bool posn::operator==(const posn &p) {
    return (p.col == col && p.row == row);
}

posn &posn::operator=(const posn &p)
{
    row = p.row;
    col = p.col;
    onBoard = p.onBoard;
    return *this;
}

std::string posn::name()
{   
    if (!onBoard) return "W";
    char let = 'a' + col;
    std::string chessPos = "";
    chessPos += let;
    chessPos += std::to_string(8 - row);
    return chessPos;
}

posn posn::goDir(posn p, Direction d)
{   
    int row = p.row;
    int col = p.col;
    row += d / 3 - 1;
    col += d % 3 - 1;
    posn p2 = p;
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        p2.onBoard = false;
    }
    p2.row = row;
    p2.col = col;
    return p2;
}
