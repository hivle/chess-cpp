#include <string>
#include <iostream>
#include "chess.h"
#include "posn.h"

int main() {
    Board x;
    posn a = posn("a8");
    posn b = posn("f3");
    std::cout << a.row << std::endl;
    std::cout << a.col << std::endl;
    std::cout<< x <<std::endl;
}