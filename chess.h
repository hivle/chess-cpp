#ifndef _CHESS_H_
#define _CHESS_H_

class Board {
    public:
        Board();
        ~Board();
        friend std::ostream& operator<<(std::ostream& out, const Board& b);
    private:
        char** chessBoard;
        bool isWhite(posn tar);
        char getPos(posn tar);
};

#endif
