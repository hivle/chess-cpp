#ifndef _CHESS_H_
#define _CHESS_H_

#include "posn.h"
#include <array>
#include <vector>
#include <string>

using BoardArray = std::array<std::array<char, 8>, 8>;

enum class GameResult { InProgress, WhiteWins, BlackWins, Draw };

class State {
public:
    State();
    bool operator==(const State&) const;
    bool whiteTurn;
    bool whiteCastle, blackCastle;
    bool whiteCastleLong, blackCastleLong;
    int repeatedMoves;
    BoardArray board;
};

class Board {
public:
    Board();

    // Query
    bool getTurn() const;
    char pieceAt(int row, int col) const;
    GameResult result() const;
    bool inCheck(bool white) const;
    posn locateKing(bool white) const;

    // Returns all legal target squares for a piece at 'from'
    // If 'captures' is provided, targets that capture are put there separately
    void legalMoves(posn from, std::vector<posn>& free, std::vector<posn>& attack) const;

    // Actions
    bool move(posn from, posn to, char promotion = 'Q');
    void updateBoard();
    bool revert();

private:
    State state;
    std::vector<State> history;
    GameResult gameResult;

    char& getPos(posn p);
    char getPos(posn p) const;
    bool isWhite(posn p) const;
    bool isBlack(posn p) const;
    bool isEnemy(posn a, posn b) const;
    bool isEmpty(posn p) const;
    bool isEmpty(const std::string& s) const;
    bool isDanger(bool white, posn loc) const;
    void dangerSquares(bool white, std::vector<posn>& danger) const;
    void movePiece(posn from, posn to, char promotion = 'Q');

    void possibleMoves(posn loc, std::vector<posn>& free, std::vector<posn>& attack, bool castling) const;
    void pawnHelper(posn loc, std::vector<posn>& free, std::vector<posn>& attack) const;
    void rookHelper(posn loc, std::vector<posn>& attack) const;
    void bishopHelper(posn loc, std::vector<posn>& attack) const;
    void knightHelper(posn loc, std::vector<posn>& attack) const;
    void queenHelper(posn loc, std::vector<posn>& attack) const;
    void kingHelper(posn loc, std::vector<posn>& free, std::vector<posn>& attack, bool castling) const;
    void lineHelper(posn loc, std::vector<posn>& attack, Direction dir) const;
};

#endif
