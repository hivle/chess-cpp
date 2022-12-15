#ifndef _CHESS_H_
#define _CHESS_H_
#include "posn.h"

class Board {
    public:
        Board();
        ~Board();
        friend std::ostream& operator<<(std::ostream&, const Board&);
        bool movePiece(posn, posn);
    private:
        char** chessBoard;
        char* getPos(posn);
        bool isEnemy(posn, posn);
};

#endif
