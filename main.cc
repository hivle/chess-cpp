#include <string>
#include <iostream>
#include "chess.h"
#include "posn.h"

int main() {
    Board x;
    std::cout<< x <<std::endl;
    posn a = posn("e2");
    posn b = posn("e4");
    posn c = posn("e5");
    x.movePiece(a,b);
    std::cout<< x <<std::endl;
    x.movePiece(b,c);
    std::cout<< x <<std::endl;
    x.revert();
    std::cout<< x <<std::endl;
    x.revert();
    x.revert();
    std::cout<< x <<std::endl;
}