#include <string>
#include "posn.h"
#include <iostream>

posn::posn():col(0), row(0), onBoard(true){}

posn::posn(std::string sq):onBoard(true){
    if (sq.length() == 2 && 'a' <= sq[0] && sq[0] <= 'h' && '1' <= sq[1] && sq[1] <= '8') {
        row = 8 - (sq[1] - '1' + 1);
        col = sq[0] - 'a';
    }
    else {
        throw std::invalid_argument("invalid argument, square can only be between \"a1\" and \"h8\"");
    }
}

bool posn::operator==(const posn &p) {
    return (p.col == col && p.row == row);
}

posn &posn::operator=(const posn &other)
{
    if (this == &other) return *this;
    col = other.col;
    row = other.row;
    onBoard = other.onBoard;
    return *this;
}

posn::posn(const posn& other) : col(other.col), row(other.row), onBoard(other.onBoard) {}

std::string posn::name() const
{   
    if (!onBoard) return "W";
    char let = 'a' + col;
    std::string chessPos = "";
    chessPos += let;
    chessPos += std::to_string(8 - row);
    return chessPos;
}

posn posn::goDir(Direction d)
{   
    posn p = *this;
    p.row += d / 3 - 1;
    p.col += d % 3 - 1;
    if (p.row < 0 || p.row > 7 || p.col < 0 || p.col > 7) {
        p.onBoard = false;
    }
    return p;
}

std::ostream& operator<<(std::ostream& out, const posn& p)
{   
    out << p.name();
    return out;
}
