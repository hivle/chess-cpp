#include <string>
#include <iostream>
#include "chess.h"
#include "posn.h"

int main(int argc, char** argv) {
    int seconds = 0;
    bool white = true;
    Board game;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            white = false;
        }
        else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                seconds = atoi(argv[i + 1]);
            } 
            else {
                std::cout << "Error: -t option requires an integer argument in seconds." << std::endl;
                return 1;
            }
        }
    }
    std::cout << "New Game" << std::endl;
    while (true) {
        std::cout << game <<std::endl;

        repeat:
        std::cout << "\rEnter a move (eg:\"e2e4\"): ";
        std::string m;
        std::cin >> m;

        if (m == "exit") {
            break;
        }
        else if (m == "back") {
            game.revert();
        }
        else if (m.length() != 4){
            std::cout << m << " is not a move" << std::endl;
            goto repeat;
        }
        else {
            std::string m1 = m.substr(0, 2);
            std::string m2 = m.substr(2, 2);
            game.movePiece(m1, m2);
        }
    }
}
