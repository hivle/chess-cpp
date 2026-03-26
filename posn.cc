#include "posn.h"
#include <stdexcept>

posn::posn() : row(0), col(0), onBoard(true) {}

posn::posn(int row, int col) : row(row), col(col), onBoard(false) {
    if (row >= 0 && row <= 7 && col >= 0 && col <= 7) onBoard = true;
}

posn::posn(const std::string& sq) : onBoard(true) {
    if (sq.length() == 2 && 'a' <= sq[0] && sq[0] <= 'h' && '1' <= sq[1] && sq[1] <= '8') {
        row = 8 - (sq[1] - '0');
        col = sq[0] - 'a';
    } else {
        throw std::invalid_argument("invalid square, must be between \"a1\" and \"h8\"");
    }
}

int posn::getRow() const { return row; }
int posn::getCol() const { return col; }

bool posn::operator==(const posn& p) const {
    return row == p.row && col == p.col;
}

bool posn::operator!=(const posn& p) const {
    return !(*this == p);
}

std::string posn::name() const {
    if (!onBoard) return "??";
    std::string s;
    s += static_cast<char>('a' + col);
    s += std::to_string(8 - row);
    return s;
}

posn posn::goDir(Direction d) const {
    posn p = *this;
    p.row += d / 3 - 1;
    p.col += d % 3 - 1;
    if (p.row < 0 || p.row > 7 || p.col < 0 || p.col > 7) {
        p.onBoard = false;
    }
    return p;
}

std::ostream& operator<<(std::ostream& out, const posn& p) {
    out << p.name();
    return out;
}
