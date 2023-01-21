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
    if (chessBoard != other.chessBoard) return false;
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
    return state.chessBoard[tar.getRow()][tar.getCol()];
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
            posn temp(row,col);
            if (isEmpty(temp)) continue;
            // true when 'temp' contians an opposite colour to the boolean 'white'
            else if (isBlack(temp) == white) {
                std::vector<posn> free;
                std::vector<posn> attack;
                possibleMoves(temp, free, attack, false);
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
    if (((n1 == 'p' || n1 == 'P') && (n2 == 'X')) || ((n2 == 'p' || n2 == 'P') && (n1 == 'X'))) {
        return true;
    }
    return ((isWhite(tar1) && isBlack(tar2)) || (isWhite(tar2) && isBlack(tar1)));
}

bool Board::isEmpty(std::string tar) {
    return isEmpty(posn(tar));
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
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            if (state.chessBoard[i][j] == (white? 'K':'k')){
                return posn(i,j);
            }
        }
    }
}

// free and attack must be empty, helper only gives legal moves without considering check
void Board::pawnHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack)
{   
    bool white = isWhite(loc);
    // pawn can only move forward, if a pawn on it's original row it means it hasn't moved
    bool pawnNotMoved = (white && loc.getRow() == 6) || (!white && loc.getRow() == 1);
    posn u1 = white ? loc.goDir(up) : loc.goDir(down);
    posn u2 = white ? u1.goDir(up) : u1.goDir(down);
    posn ur = white ? loc.goDir(upright) : loc.goDir(downleft);
    posn ul = white ? loc.goDir(upleft) : loc.goDir(downright);
    if (u1.onBoard && isEmpty(u1)) {
        free.push_back(u1);
        if (pawnNotMoved && isEmpty(u2)) free.push_back(u2);
    }
    if (ur.onBoard) attack.push_back(ur);
    if (ul.onBoard) attack.push_back(ul);
}

void Board::lineHelper(posn loc, std::vector<posn> &attack, Direction dir){
    posn d1 = loc.goDir(dir);
    while(d1.onBoard){
        if (isEmpty(d1)) {
            attack.push_back(d1);
            d1 = d1.goDir(dir);
        }
        else {
            if(isEnemy(loc, d1)) attack.push_back(d1);
            break;
        } 
    }
}

void Board::rookHelper(posn loc, std::vector<posn> &attack)
{   
    lineHelper(loc, attack, up);
    lineHelper(loc, attack, down);
    lineHelper(loc, attack, left);
    lineHelper(loc, attack, right);
}

void Board::bishopHelper(posn loc, std::vector<posn> &attack)
{   
    lineHelper(loc, attack, upright);
    lineHelper(loc, attack, upleft);
    lineHelper(loc, attack, downright);
    lineHelper(loc, attack, downleft);
}

void Board::knightHelper(posn loc, std::vector<posn> &attack)
{
    posn dirs[] = {loc.goDir(upright).goDir(up), loc.goDir(upright).goDir(right), 
                   loc.goDir(upleft).goDir(up), loc.goDir(upleft).goDir(left), 
                   loc.goDir(downright).goDir(down), loc.goDir(downright).goDir(right), 
                   loc.goDir(downleft).goDir(down), loc.goDir(downleft).goDir(left)};             
    for (posn i : dirs) {
        if (isEnemy(loc, i)) attack.push_back(i);
        if (isEmpty(i)) attack.push_back(i);
    }
}

void Board::queenHelper(posn loc, std::vector<posn> &attack)
{   
    bishopHelper(loc, attack);
    rookHelper(loc, attack);
}

bool Board::isDanger(bool white, posn loc) {
    std::vector<posn> danger;
    dangerSquares(white, danger);
    return (std::find(danger.begin(), danger.end(), loc) != danger.end());
}

void Board::kingHelper(posn loc, std::vector<posn> &free, std::vector<posn> &attack, bool c)
{   
    for (int i = upleft; i < downright; i++){
        if (i != stay) {
            posn temp = loc.goDir((Direction)i);
            if (isEnemy(loc, temp)) attack.push_back(temp);
            else if (isEmpty(temp)) attack.push_back(temp);
        }
    }
    if (c) {
        if (state.whiteTurn){
            if (state.whiteCastle) {
                if (isEmpty("f1") && isEmpty("g1")) {
                    if (!isDanger(state.whiteTurn, posn("f1")) && !isDanger(state.whiteTurn, posn("g1"))) {
                        free.push_back(posn("g1"));
                    }
                }
            }
            if (state.whiteCastleLong) {
                if (isEmpty("d1") && isEmpty("c1") && isEmpty("b1")) {
                    if (!isDanger(state.whiteTurn, posn("c1")) && !isDanger(state.whiteTurn, posn("b1"))) {
                        free.push_back(posn("c1"));
                    }
                }
            }
        }
        else {
            if (state.blackCastle) {
                if (isEmpty("f8") && isEmpty("g8")) {
                    if (!isDanger(state.whiteTurn, posn("f8")) && !isDanger(state.whiteTurn, posn("g8"))) {
                        free.push_back(posn("g8"));
                    }
                }
            }
            if (state.blackCastleLong) {
                if (isEmpty("d1") && isEmpty("c8") && isEmpty("b8")) {
                    if (!isDanger(state.whiteTurn, posn("c8")) && !isDanger(state.whiteTurn, posn("b8"))) {
                        free.push_back(posn("c8"));
                    }
                }
            }
        }
    }
}

void Board::legalMoves(posn loc, std::vector<posn> &free, std::vector<posn> &attack){
    if (isEmpty(loc)) return;
    if (isWhite(loc) && !state.whiteTurn) return;
    if (isBlack(loc) && state.whiteTurn) return;
    std::vector<posn> tempFree;
    std::vector<posn> tempAttack;
    possibleMoves(loc, tempFree, tempAttack, true);
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
    for (posn move : tempFree)  if (move.onBoard) free.push_back(move);
    for (posn move : tempAttack) {
        if (move.onBoard){
            if (isEnemy(move,loc)) attack.push_back(move);
            else if (getPos(loc) != 'p' && getPos(loc) != 'P') free.push_back(move);
        }
    }

}

void Board::possibleMoves(posn loc, std::vector<posn> &free, std::vector<posn> &attack, bool c){
    switch (getPos(loc)){
        case 'P':
        case 'p': pawnHelper(loc, free, attack); break;
        case 'R':
        case 'r': rookHelper(loc,attack); break;
        case 'B':
        case 'b': bishopHelper(loc,attack); break;
        case 'N':
        case 'n': knightHelper(loc,attack); break;
        case 'Q':
        case 'q': queenHelper(loc,attack); break;
        case 'K':
        case 'k': kingHelper(loc,free,attack, c); break;
    }
}

void Board::movePiece(std::string start, std::string end) {
    movePiece(posn(start), posn(end));
}

// assumes moves are legal already
void Board::movePiece(posn ini, posn tar)
{   
    auto start = ini.name();
    auto end = tar.name();
    state.repeatedMoves++;
    // clear enpassant mark at beginning
    for (auto &row : state.chessBoard) {
        for (char &piece : row) if (piece == 'X') piece = '0';
    }
    char& iniPos = getPos(ini);
    char& tarPos = getPos(tar);
    if (iniPos == 'P' || iniPos == 'p' || isEnemy(ini, tar)) state.repeatedMoves = 0;
    history.push_back(state);
    // place enpassant mark
    if (ini.getRow() == 6 && iniPos == 'P' && tar.getRow() == 4) {
        state.chessBoard[5][ini.getCol()] = 'X';
    }
    else if (ini.getRow() == 1 && iniPos == 'p' && tar.getRow() == 3) {
        state.chessBoard[2][ini.getCol()] = 'X';
    }

    tarPos = iniPos;
    iniPos = '0';

    // Castle
    if (start == "e1") {
        if (state.whiteCastle && end == "g1"){
            getPos(posn("h1")) = '0';
            getPos(posn("f1")) = 'R';          
        }  
        if (state.whiteCastleLong &&  end == "c1"){
            getPos(posn("a1")) = '0';
            getPos(posn("d1")) = 'R';
        }
        state.whiteCastle = false;
        state.whiteCastleLong = false;
    }
    else if (start == "e8") {
        if (state.blackCastle  && end == "g8"){
            getPos(posn("h8")) = '0';
            getPos(posn("f8")) = 'R';  
        }
        if (state.blackCastleLong &&  end == "c8") {
            getPos(posn("a8")) = '0';
            getPos(posn("d8")) = 'R';
        }
        state.blackCastle = false;
        state.blackCastleLong = false;
    }
    else if (start == "h1") state.whiteCastle = false;
    else if (start == "a1") state.whiteCastleLong = false;
    else if (start == "h8") state.blackCastle = false;
    else if (start == "a8") state.blackCastleLong = false;

    // Fifty move rule
    if (state.repeatedMoves == 100) draw = true;
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
        // Switch turns
        state.whiteTurn = !state.whiteTurn;
        return true;
    }
    return false;
}

void Board::updateBoard() {
    std::cout<< state.whiteCastle << state.whiteCastleLong << state.blackCastle << state.blackCastleLong << std::endl;
    bool legal = false; // boolean to tell whether there exist legal moves
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            posn temp(row,col);
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

// todo: pawn promotion, timer
