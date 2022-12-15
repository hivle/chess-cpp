#include <string>
#include <iostream>
#include "chess.h"
#include "posn.h"

int main() {
    Board x;
    std::cout<< x <<std::endl;
    posn a = posn("a8");
    posn b = posn("f4");
    x.movePiece(a,b);
    std::cout<< x <<std::endl;
}