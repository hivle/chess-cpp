#include <string>
#include <iostream>
#include <utility>
#include "posn.h"

posn::posn():pos(0,0), onBoard(true){}

posn::posn(int row, int col): pos(row,col), onBoard(false) {
    if (row >= 0 && row <= 7 && col >= 0 && col <= 7) onBoard = true;
}

posn::posn(std::string sq):onBoard(true){
    if (sq.length() == 2 && 'a' <= sq[0] && sq[0] <= 'h' && '1' <= sq[1] && sq[1] <= '8') {
        pos.first = 8 - (sq[1] - '1' + 1);
        pos.second = sq[0] - 'a';
    }
    else {
        throw std::invalid_argument("invalid argument, square can only be between \"a1\" and \"h8\"");
    }
}

int posn::getRow(){
    return pos.first;
}

int posn::getCol(){
    return pos.second;
}

bool posn::operator==(const posn &p) {
    return (p.pos == pos);
}

posn &posn::operator=(const posn &other)
{
    if (this == &other) return *this;
    pos = other.pos;
    onBoard = other.onBoard;
    return *this;
}

posn::posn(const posn& other) : pos(other.pos), onBoard(other.onBoard) {}

std::string posn::name() const
{   
    if (!onBoard) return "W";
    char let = 'a' + pos.second;
    std::string chessPos = "";
    chessPos += let;
    chessPos += std::to_string(8 - pos.first);
    return chessPos;
}

posn posn::goDir(Direction d)
{   
    posn p = *this;
    p.pos.first += d / 3 - 1;
    p.pos.second += d % 3 - 1;
    if (p.pos.first < 0 || p.pos.first > 7 || p.pos.second < 0 || p.pos.second > 7) {
        p.onBoard = false;
    }
    return p;
}

std::ostream& operator<<(std::ostream& out, const posn& p)
{   
    out << p.name();
    return out;
}
