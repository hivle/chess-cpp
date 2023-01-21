#ifndef _POSN_H_
#define _POSN_H_

enum Direction {
    upleft,
    up,
    upright,
    left,
    stay,
    right,
    downleft,
    down,
    downright
};

class posn {
    public:
        posn();
        posn(int, int);
        posn(std::string chessPos);
        int getRow();
        int getCol();
        bool operator==(const posn &);
        posn& operator=(const posn&); 
        posn(const posn &);
        friend std::ostream& operator<<(std::ostream&, const posn&);
        std::string name() const;
        posn goDir(Direction);
        std::pair<int,int> pos;
        bool onBoard;
};

#endif
