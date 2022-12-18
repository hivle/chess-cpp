#ifndef _CHESS_H_
#define _CHESS_H_
#include "posn.h"
#include <vector>
#include <iostream>

class State {
    public:
        State();
        bool operator==(const State &);
        bool whiteTurn;
        bool whiteCastle;
        bool blackCastle;
        bool whiteCastleLong;
        bool blackCastleLong;
        int repeatedMoves;
        std::vector<std::vector<char>> chessBoard;
};

class Board {
    public:
        Board();
        ~Board();
        friend std::ostream& operator<<(std::ostream&, const Board&);
        bool movePiece(posn, posn);
        bool revert();
    private:
        bool draw;
        bool whiteWin;
        State state;
        std::vector<State> history;
        char* getPos(posn);
        bool isWhite(posn);
        bool isBlack(posn);
        bool isEnemy(posn, posn);
        void pawnHelper(posn, std::vector<posn>, std::vector<posn>);
    };

#endif
