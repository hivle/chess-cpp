#ifndef _POSN_H_
#define _POSN_H_

class posn {
    public:
        posn(std::string chessPos);
        bool operator==(const posn &);
        std::string name();
        int col;
        int row;
};

#endif
