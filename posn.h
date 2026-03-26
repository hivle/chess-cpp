#ifndef _POSN_H_
#define _POSN_H_

#include <string>
#include <iostream>

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
    int row, col;
public:
    bool onBoard;

    posn();
    posn(int row, int col);
    posn(const std::string& chessPos);
    posn(const posn&) = default;
    posn& operator=(const posn&) = default;

    int getRow() const;
    int getCol() const;
    bool operator==(const posn& other) const;
    bool operator!=(const posn& other) const;
    std::string name() const;
    posn goDir(Direction d) const;

    friend std::ostream& operator<<(std::ostream&, const posn&);
};

#endif
