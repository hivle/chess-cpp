#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>
#include "chess.h"
#include "posn.h"

int main(int argc, char** argv) {
    int whiteTime = 300;
    int blackTime = 300;
    bool white = true;
    Board game;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                whiteTime = atoi(argv[i + 1]);
                blackTime = whiteTime;
            } 
            else {
                std::cout << "Error: -t option requires an integer argument in seconds." << std::endl;
                return 1;
            }
        }
    }
    std::cout << "New Game\n";
    while (true) {
        std::cout << game << '\n';
        repeat:
        if (game.getTurn()) {
            if (game.inCheck(true)) {
                std::cout<< "White in Check\n";
            }
            std::cout<< "White to play\n";
        }
        else {
            if (game.inCheck(false)) {
                std::cout<< "Black in Check\n";
            }
            std::cout<< "Black to play\n";
        }
        std::cout << "Enter a square (eg: \"e2\") or enter a move (eg:\"e2e4\"): ";
        std::string m;
        std::cin >> m;

        if (m == "exit") break;
        else if (m == "back") game.revert();
        else if (m.length() == 2) {
            std::vector<posn> free;
            std::vector<posn> attack;
            game.legalMoves(m, free, attack);
            if (free.empty() && attack.empty()) {
                std::cout << "No valid moves" << std::endl;
            }
            else { 
                if (free.size()) {
                    std::cout << "Free Squares: ";
                    std::copy(free.begin(), free.end() - 1, std::ostream_iterator<posn>(std::cout, ", "));
                    std::cout << free.back() << std::endl;
                }
                if (attack.size()){
                    std::cout << "Attack Squares: ";
                    std::copy(attack.begin(), attack.end() - 1, std::ostream_iterator<posn>(std::cout, ", "));
                    std::cout << attack.back() << std::endl;
                }
            }
            goto repeat;
        }
        else if (m.length() != 4){
            std::cout << m << " is not a move\n";
            goto repeat;
        }
        else {
            std::string m1 = m.substr(0, 2);
            std::string m2 = m.substr(2, 2);
            if (!game.move(m1, m2)){
                std::cout << m << " is an invalid move\n";
                goto repeat;
            }
            else {
                white = !white;
                game.updateBoard();
                if (game.blackWin) std::cout << "Black Won" << std::endl;
                else if (game.whiteWin) std::cout << "White Won" << std::endl;
                else if (game.draw) std::cout << "Draw" << std::endl;
            }
        }
    }
}
