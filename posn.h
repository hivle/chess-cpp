#ifndef _POSN_H_
#define _POSN_H_

class posn {
    posn(std::string chessPos);
    bool operator==(const posn &);
    int col;
    int row;
};

#endif
