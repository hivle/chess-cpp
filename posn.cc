#include <string>
#include "posn.h"

// assumes valid chess position
posn::posn(std::string chessPos){
    char let = chessPos[0];
    char num = chessPos[1];
    row = 8 - (num - '1' + 1);
    col = let - 'a';
}

bool posn::operator==(const posn &p) {
    return (p.col == col && p.row == row);
}
