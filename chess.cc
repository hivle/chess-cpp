#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include "posn.h"
#include "chess.h"

State::State():
whiteTurn(true),
whiteCastle(true),
blackCastle(true),
whiteCastleLong(true),
blackCastleLong(true),
repeatedMoves(0),
chessBoard({
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'0', '0', '0', '0', '0', '0', '0', '0'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
}){}

bool State::operator==(const State & other)
{
    if (whiteTurn != other.whiteTurn) return false;
    if (whiteCastle != other.whiteCastle) return false;
    if (blackCastle != other.blackCastle) return false;
    if (whiteCastleLong != other.whiteCastleLong) return false;
    if (blackCastleLong != other.blackCastleLong) return false;
    if (repeatedMoves != other.repeatedMoves) return false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (chessBoard[i][j] != other.chessBoard[i][j]) return false;
        }
    }
    return true;
}

Board::Board()
{   
    State state;
    whitePiece = {'P','R','N','B','Q','K'};
    blackPiece = {'p','r','n','b','q','k'};
    draw = false;
    whiteWin = false;
    blackWin = false;
}

Board::~Board() {}

char& Board::getPos(posn tar)
{
    return state.chessBoard[tar.row][tar.col];
}

std::ostream& operator<<(std::ostream& out, const Board& b)
{
    for (int i = 0; i < 8; i++) {
        int row = 8 - i;
        out <<  "   ---------------------------------\n " << row << " | ";
        for (int j = 0; j < 8; j++) {
            char piece = b.state.chessBoard[i][j];
            if (piece == '0' || piece == 'X') piece = ' ';
            out << piece << " | ";
        }
        out << '\n';
    }
    out << "   ---------------------------------\n     a   b   c   d   e   f   g   h\n";
    return out;
}

bool Board::getTurn() {
    return state.whiteTurn;
}

bool Board::inCheck(bool white) {
    posn king = locateKing(white);
    std::vector<posn> danger;
    dangerSquares(white, danger);
    return ((std::find(danger.begin(), danger.end(), king) != danger.end())? true : false);
}

void Board::dangerSquares(bool white, std::vector<posn> &danger)
{
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            posn temp;
            temp.row = row;
            temp.col = col;
            if (isEmpty(temp)) continue;
            // true when 'temp' contians an opposite colour to the boolean 'white'
            else if (isBlack(temp) == white) {
                std::vector<posn> free;
                std::vector<posn> attack;
                possibleMoves(temp, free, attack);
                for (posn moves : attack) {
                    if (std::find(danger.begin(), danger.end(), moves) == danger.end()) {
                        danger.push_back(moves);
                    }
                }
            }
        }
    }
}

bool Board::isWhite(posn tar) 
{
    char start = getPos(tar);
    for(char piece : whitePiece){
        if (piece == start) return true;
    }
    return false;
}

bool Board::isBlack(posn tar)
{
    char start = getPos(tar);
    for(char piece : blackPiece){
        if (piece == start) return true;
    }
    return false;
}

bool Board::isEnemy(posn tar1, posn tar2)
{   
    if (!tar1.onBoard || !tar2.onBoard) return false;
    char n1 = getPos(tar1);
    char n2 = getPos(tar2);
    /* enpassant rule, everytime a pawn moves 2 squares it leaves an mark 'X' behind it where only another
    pawn can take, since no side can move twice in a row that will be the opposite color pawn, once the
    next move is made the 'X' will be cleared off the board if it still exists */
    if (((n1 == 'p' || n1 == 'P') && (n2 == 'X')) || ((n2 == 'p' || n2 == 'P') && (n2 == 'X'))) {
        return true;
    }
    return ((isWhite(tar1) && isBlack(tar2)) || (isWhite(tar2) && isBlack(tar1)));
}

bool Board::isEmpty(posn tar) 
{   
    if (!tar.onBoard) return false;
    char temp = getPos(tar);
    return (temp == '0' || temp == 'X');
}

// assumes both Kings are always on the board
posn Board::locateKing(bool white)
{
    posn kingPosition;
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            if (state.chessBoard[i][j] == (white? 'K':'k')){
                kingPosition.col = j;
                kingPosition.row = i;
                return kingPosition;
            }
        }
    }
}

// free and attack must be empty, helper only gives legal moves without considering check
void Board::pawnHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{   
    bool white = isWhite(loc);
    // pawn can only move forward, if a pawn on it's original row it means it hasn't moved
    bool pawnNotMoved = (white && loc.row == 6) || (!white && loc.row == 1);
    posn u1 = white ? loc.goDir(up) : loc.goDir(down);
    posn u2 = white ? u1.goDir(up) : u1.goDir(down);
    posn ur = white ? loc.goDir(upright) : loc.goDir(downleft);
    posn ul = white ? loc.goDir(upleft) : loc.goDir(downright);
    if (u1.onBoard && isEmpty(u1)) {
        free.push_back(u1);
        if (pawnNotMoved && isEmpty(u2)) free.push_back(u2);
    }
    if (ur.onBoard && isEnemy(loc, ur)) attack.push_back(ur);
    if (ul.onBoard && isEnemy(loc, ul)) attack.push_back(ul);
}

void Board::lineHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack, Direction dir){
    posn d1 = loc.goDir(dir);
    while(d1.onBoard){
        if (isEmpty(d1)) {
            free.push_back(d1);
            d1 = d1.goDir(dir);
        }
        else {
            if(isEnemy(loc, d1)) attack.push_back(d1);
            break;
        } 
    }
}

void Board::rookHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{   
    lineHelper(loc, free, attack, up);
    lineHelper(loc, free, attack, down);
    lineHelper(loc, free, attack, left);
    lineHelper(loc, free, attack, right);
}

void Board::bishopHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{   
    lineHelper(loc, free, attack, upright);
    lineHelper(loc, free, attack, upleft);
    lineHelper(loc, free, attack, downright);
    lineHelper(loc, free, attack, downleft);
}

void Board::knightHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{
    posn dirs[] = {loc.goDir(upright).goDir(up), loc.goDir(upright).goDir(right), 
                   loc.goDir(upleft).goDir(up), loc.goDir(upleft).goDir(left), 
                   loc.goDir(downright).goDir(down), loc.goDir(downright).goDir(right), 
                   loc.goDir(downleft).goDir(down), loc.goDir(downleft).goDir(left)};             
    for (posn i : dirs) {
        if (isEnemy(loc, i)) attack.push_back(i);
        if (isEmpty(i)) free.push_back(i);
    }
}

void Board::queenHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{   
    bishopHelper(loc, free, attack);
    rookHelper(loc, free, attack);
}

void Board::kingHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{   
    for (int i = upleft; i < downright; i++){
        if (i != stay) {
            posn temp = loc.goDir((Direction)i);
            if (isEnemy(loc, temp)) attack.push_back(temp);
            else if (isEmpty(temp)) free.push_back(temp);
        }
    }
}

void Board::legalMoves(posn loc, std::vector<posn> &free, std::vector<posn> &attack){
    if (isEmpty(loc)) return;
    if (isWhite(loc) && !state.whiteTurn) return;
    if (isBlack(loc) && state.whiteTurn) return;
    std::vector<posn> tempFree;
    std::vector<posn> tempAttack;
    possibleMoves(loc, tempFree, tempAttack);
    for (posn &target : tempFree) {
        movePiece(loc, target);
        if (inCheck(!state.whiteTurn)) target.onBoard = false;
        revert();
    }
    for (posn &target : tempAttack) {
        movePiece(loc, target);
        if (inCheck(!state.whiteTurn)) target.onBoard = false;
        revert();
    }
    for (posn move : tempFree)   if (move.onBoard) free.push_back(move);
    for (posn move : tempAttack) if (move.onBoard) attack.push_back(move);
}

//TODO: add const
void Board::possibleMoves(posn loc, std::vector<posn> &free, std::vector<posn> &attack){
    switch (getPos(loc)){
        case 'P':
        case 'p': pawnHelper(loc,free,attack); break;
        case 'R':
        case 'r': rookHelper(loc,free,attack); break;
        case 'B':
        case 'b': bishopHelper(loc,free,attack); break;
        case 'N':
        case 'n': knightHelper(loc,free,attack); break;
        case 'Q':
        case 'q': queenHelper(loc,free,attack); break;
        case 'K':
        case 'k': kingHelper(loc,free,attack); break;
    }
}

// move no matter if a move is legal
void Board::movePiece(posn ini, posn tar)
{   
    // clear enpassant mark at beginning
    for (auto &row : state.chessBoard) {
        for (char &piece : row) if (piece == 'X') piece = '0';
    }
    char& iniPos = getPos(ini);
    char& tarPos = getPos(tar);
    history.push_back(state);

    if (ini.row == 6 && iniPos == 'P' && tar.row == 4) {
        state.chessBoard[5][ini.col] = 'X';
    }
    else if (ini.row == 1 && iniPos == 'p' && tar.row == 3) {
        state.chessBoard[2][ini.col] = 'X';
    }

    tarPos = iniPos;
    iniPos = '0';
    state.whiteTurn = !state.whiteTurn;
    // Third time a state repeats the game is drawn
    if (std::count(history.begin(), history.end(), state) >= 3) draw = true;
}

bool Board::move(posn ini, posn tar) {
    bool legal = false;
    std::vector<posn> free;
    std::vector<posn> attack;
    legalMoves(ini, free, attack);
        if (std::find(free.begin(), free.end(), tar) != free.end() ||
            std::find(attack.begin(), attack.end(), tar) != attack.end())
        {
            movePiece(ini, tar);
            return true;
        }
    return false;
}


void Board::updateBoard() {
    bool legal = false; // boolean to tell whether there exist legal moves
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            posn temp;
            temp.row = row;
            temp.col = col;
            if (isEmpty(temp)) continue;
            std::vector<posn> free;
            std::vector<posn> attack;
            if (state.whiteTurn ^ isBlack(temp)) legalMoves(temp, free, attack); // ^ is the XOR operator
            if (free.size() || attack.size()) legal = true; break;
        }
    }
    if (!legal) {
        if (inCheck(state.whiteTurn)) {
            whiteWin = !state.whiteTurn;
            blackWin = state.whiteTurn;
        }
        else draw = true;
    }
}

bool Board::revert()
{
    if (history.empty()) return false;
    state = history.back();
    history.pop_back();
    return true;
}

// incorporate smart pointers when possible
// todo: castle, pawn promotion, timer, 50 move rule
