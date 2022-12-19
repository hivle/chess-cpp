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
        void legalMoves(posn, std::vector<posn>&, std::vector<posn>&);
        bool getTurn();
    private:
        std::vector<char> blackPiece;
        std::vector<char> whitePiece;
        bool draw;
        bool whiteWin;
        State state;
        std::vector<State> history;
        char* getPos(posn);
        bool isWhite(posn);
        bool isBlack(posn);
        bool isEnemy(posn, posn);
        bool freeSquare(posn);
        void lineHelper(posn, std::vector<posn>&, std::vector<posn>&, Direction);
        void pawnHelper(posn, std::vector<posn>&, std::vector<posn>&);
        void rookHelper(posn, std::vector<posn>&, std::vector<posn>&);
        void bishopHelper(posn, std::vector<posn>&, std::vector<posn>&);
        void kingHelper(posn, std::vector<posn>&, std::vector<posn>&);
        void knightHelper(posn, std::vector<posn>&, std::vector<posn>&);
        void queenHelper(posn, std::vector<posn>&, std::vector<posn>&);
};

#endif
