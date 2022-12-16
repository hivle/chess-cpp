#ifndef _CHESS_H_
#define _CHESS_H_
#include "posn.h"
#include <vector>
#include <iostream>
#include <stack>

class State {
    public:
        State();
        bool whiteTurn;
        bool whiteCastle;
        bool blackCastle;
        bool whiteCastleLong;
        bool blackCastleLong;
        int repeatedMoves;
        std::vector<std::vector<char>> pastBoard;
};

class Board {
    public:
        Board();
        ~Board();
        friend std::ostream& operator<<(std::ostream&, const Board&);
        bool movePiece(posn, posn);
        bool revert();
    private:
        State state;
        std::vector<std::vector<char>> chessBoard;
        std::stack<State> history;
        char* getPos(posn);
        bool isEnemy(posn, posn);
};

#endif
