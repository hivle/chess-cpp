#include "chess.h"
#include <algorithm>
#include <cctype>

// --- State ---

State::State()
    : whiteTurn(true)
    , whiteCastle(true), blackCastle(true)
    , whiteCastleLong(true), blackCastleLong(true)
    , repeatedMoves(0)
    , board({{
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'0', '0', '0', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '0', '0'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
    }}) {}

bool State::operator==(const State& other) const {
    return whiteTurn == other.whiteTurn
        && whiteCastle == other.whiteCastle
        && blackCastle == other.blackCastle
        && whiteCastleLong == other.whiteCastleLong
        && blackCastleLong == other.blackCastleLong
        && board == other.board;
}

// --- Board ---

Board::Board() : gameResult(GameResult::InProgress) {}

bool Board::getTurn() const { return state.whiteTurn; }

char Board::pieceAt(int row, int col) const {
    char c = state.board[row][col];
    return (c == '0' || c == 'X') ? ' ' : c;
}

GameResult Board::result() const { return gameResult; }

char& Board::getPos(posn p) { return state.board[p.getRow()][p.getCol()]; }
char Board::getPos(posn p) const { return state.board[p.getRow()][p.getCol()]; }

bool Board::isWhite(posn p) const {
    char c = getPos(p);
    return std::isupper(static_cast<unsigned char>(c));
}

bool Board::isBlack(posn p) const {
    char c = getPos(p);
    return std::islower(static_cast<unsigned char>(c));
}

bool Board::isEnemy(posn a, posn b) const {
    if (!a.onBoard || !b.onBoard) return false;
    char ca = getPos(a), cb = getPos(b);
    // en passant marker counts as enemy for pawns
    if (((ca == 'p' || ca == 'P') && cb == 'X') ||
        ((cb == 'p' || cb == 'P') && ca == 'X'))
        return true;
    return (isWhite(a) && isBlack(b)) || (isWhite(b) && isBlack(a));
}

bool Board::isEmpty(const std::string& s) const { return isEmpty(posn(s)); }

bool Board::isEmpty(posn p) const {
    if (!p.onBoard) return false;
    char c = getPos(p);
    return c == '0' || c == 'X';
}

posn Board::locateKing(bool white) const {
    char target = white ? 'K' : 'k';
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (state.board[i][j] == target)
                return posn(i, j);
    return posn(0, 0); // unreachable in a valid game
}

bool Board::inCheck(bool white) const {
    posn king = locateKing(white);
    std::vector<posn> danger;
    dangerSquares(white, danger);
    return std::find(danger.begin(), danger.end(), king) != danger.end();
}

void Board::dangerSquares(bool white, std::vector<posn>& danger) const {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            posn p(r, c);
            if (isEmpty(p)) continue;
            if (isBlack(p) == white) {
                std::vector<posn> free, attack;
                possibleMoves(p, free, attack, false);
                for (const posn& m : attack) {
                    if (std::find(danger.begin(), danger.end(), m) == danger.end())
                        danger.push_back(m);
                }
            }
        }
    }
}

bool Board::isDanger(bool white, posn loc) const {
    std::vector<posn> danger;
    dangerSquares(white, danger);
    return std::find(danger.begin(), danger.end(), loc) != danger.end();
}

// --- Move Helpers ---

void Board::pawnHelper(posn loc, std::vector<posn>& free, std::vector<posn>& attack) const {
    bool white = isWhite(loc);
    bool unmoved = (white && loc.getRow() == 6) || (!white && loc.getRow() == 1);
    posn u1 = white ? loc.goDir(up) : loc.goDir(down);
    posn u2 = white ? u1.goDir(up) : u1.goDir(down);
    posn ur = white ? loc.goDir(upright) : loc.goDir(downleft);
    posn ul = white ? loc.goDir(upleft) : loc.goDir(downright);

    if (u1.onBoard && isEmpty(u1)) {
        free.push_back(u1);
        if (unmoved && isEmpty(u2)) free.push_back(u2);
    }
    if (ur.onBoard) attack.push_back(ur);
    if (ul.onBoard) attack.push_back(ul);
}

void Board::lineHelper(posn loc, std::vector<posn>& attack, Direction dir) const {
    posn d = loc.goDir(dir);
    while (d.onBoard) {
        if (isEmpty(d)) {
            attack.push_back(d);
            d = d.goDir(dir);
        } else {
            if (isEnemy(loc, d)) attack.push_back(d);
            break;
        }
    }
}

void Board::rookHelper(posn loc, std::vector<posn>& attack) const {
    lineHelper(loc, attack, up);
    lineHelper(loc, attack, down);
    lineHelper(loc, attack, left);
    lineHelper(loc, attack, right);
}

void Board::bishopHelper(posn loc, std::vector<posn>& attack) const {
    lineHelper(loc, attack, upright);
    lineHelper(loc, attack, upleft);
    lineHelper(loc, attack, downright);
    lineHelper(loc, attack, downleft);
}

void Board::knightHelper(posn loc, std::vector<posn>& attack) const {
    posn dirs[] = {
        loc.goDir(upright).goDir(up),    loc.goDir(upright).goDir(right),
        loc.goDir(upleft).goDir(up),     loc.goDir(upleft).goDir(left),
        loc.goDir(downright).goDir(down), loc.goDir(downright).goDir(right),
        loc.goDir(downleft).goDir(down),  loc.goDir(downleft).goDir(left)
    };
    for (const posn& d : dirs) {
        if (d.onBoard && (isEmpty(d) || isEnemy(loc, d)))
            attack.push_back(d);
    }
}

void Board::queenHelper(posn loc, std::vector<posn>& attack) const {
    bishopHelper(loc, attack);
    rookHelper(loc, attack);
}

void Board::kingHelper(posn loc, std::vector<posn>& free, std::vector<posn>& attack, bool castling) const {
    for (int i = upleft; i <= downright; i++) {
        if (i == stay) continue;
        posn temp = loc.goDir(static_cast<Direction>(i));
        if (!temp.onBoard) continue;
        if (isEnemy(loc, temp)) attack.push_back(temp);
        else if (isEmpty(temp)) attack.push_back(temp);
    }

    if (!castling) return;

    if (state.whiteTurn) {
        // King cannot castle out of check
        if (!inCheck(true)) {
            if (state.whiteCastle && isEmpty("f1") && isEmpty("g1")) {
                if (!isDanger(true, posn("f1")) && !isDanger(true, posn("g1")))
                    free.push_back(posn("g1"));
            }
            if (state.whiteCastleLong && isEmpty("d1") && isEmpty("c1") && isEmpty("b1")) {
                if (!isDanger(true, posn("c1")) && !isDanger(true, posn("d1")))
                    free.push_back(posn("c1"));
            }
        }
    } else {
        if (!inCheck(false)) {
            if (state.blackCastle && isEmpty("f8") && isEmpty("g8")) {
                if (!isDanger(false, posn("f8")) && !isDanger(false, posn("g8")))
                    free.push_back(posn("g8"));
            }
            if (state.blackCastleLong && isEmpty("d8") && isEmpty("c8") && isEmpty("b8")) {
                if (!isDanger(false, posn("c8")) && !isDanger(false, posn("d8")))
                    free.push_back(posn("c8"));
            }
        }
    }
}

void Board::possibleMoves(posn loc, std::vector<posn>& free, std::vector<posn>& attack, bool castling) const {
    switch (getPos(loc)) {
        case 'P': case 'p': pawnHelper(loc, free, attack); break;
        case 'R': case 'r': rookHelper(loc, attack); break;
        case 'B': case 'b': bishopHelper(loc, attack); break;
        case 'N': case 'n': knightHelper(loc, attack); break;
        case 'Q': case 'q': queenHelper(loc, attack); break;
        case 'K': case 'k': kingHelper(loc, free, attack, castling); break;
    }
}

// --- Legal Move Filtering ---

void Board::legalMoves(posn loc, std::vector<posn>& free, std::vector<posn>& attack) const {
    if (isEmpty(loc)) return;
    if (isWhite(loc) && !state.whiteTurn) return;
    if (isBlack(loc) && state.whiteTurn) return;

    std::vector<posn> tmpFree, tmpAttack;
    possibleMoves(loc, tmpFree, tmpAttack, true);

    // We need a mutable copy to test moves
    Board test = *this;

    for (posn& target : tmpFree) {
        test.state = state;
        test.movePiece(loc, target);
        if (test.inCheck(!test.state.whiteTurn)) target.onBoard = false;
        test.state = state;
    }
    for (posn& target : tmpAttack) {
        test.state = state;
        test.movePiece(loc, target);
        if (test.inCheck(!test.state.whiteTurn)) target.onBoard = false;
        test.state = state;
    }

    for (const posn& m : tmpFree)
        if (m.onBoard) free.push_back(m);
    for (const posn& m : tmpAttack) {
        if (!m.onBoard) continue;
        if (isEnemy(m, loc)) attack.push_back(m);
        else if (getPos(loc) != 'p' && getPos(loc) != 'P') free.push_back(m);
    }
}

// --- Move Execution ---

void Board::movePiece(posn ini, posn tar, char promotion) {
    std::string start = ini.name();
    std::string end = tar.name();

    state.repeatedMoves++;
    // Clear en passant markers
    for (auto& row : state.board)
        for (char& c : row)
            if (c == 'X') c = '0';

    char& iniPos = getPos(ini);
    char& tarPos = getPos(tar);
    if (iniPos == 'P' || iniPos == 'p' || isEnemy(ini, tar))
        state.repeatedMoves = 0;

    history.push_back(state);

    // Place en passant marker
    if (ini.getRow() == 6 && iniPos == 'P' && tar.getRow() == 4)
        state.board[5][ini.getCol()] = 'X';
    else if (ini.getRow() == 1 && iniPos == 'p' && tar.getRow() == 3)
        state.board[2][ini.getCol()] = 'X';

    // En passant capture: pawn moves diagonally to empty square
    if ((iniPos == 'P' || iniPos == 'p') && ini.getCol() != tar.getCol() && tarPos == '0') {
        state.board[ini.getRow()][tar.getCol()] = '0';
    }

    char piece = iniPos;
    tarPos = piece;
    iniPos = '0';

    // Pawn promotion
    if (piece == 'P' && tar.getRow() == 0)
        tarPos = std::toupper(static_cast<unsigned char>(promotion));
    else if (piece == 'p' && tar.getRow() == 7)
        tarPos = std::tolower(static_cast<unsigned char>(promotion));

    // Castling rook movement
    if (start == "e1") {
        if (state.whiteCastle && end == "g1") {
            getPos(posn("h1")) = '0';
            getPos(posn("f1")) = 'R';
        }
        if (state.whiteCastleLong && end == "c1") {
            getPos(posn("a1")) = '0';
            getPos(posn("d1")) = 'R';
        }
        state.whiteCastle = false;
        state.whiteCastleLong = false;
    } else if (start == "e8") {
        if (state.blackCastle && end == "g8") {
            getPos(posn("h8")) = '0';
            getPos(posn("f8")) = 'r';
        }
        if (state.blackCastleLong && end == "c8") {
            getPos(posn("a8")) = '0';
            getPos(posn("d8")) = 'r';
        }
        state.blackCastle = false;
        state.blackCastleLong = false;
    }
    // Rook moves invalidate castling
    else if (start == "h1") state.whiteCastle = false;
    else if (start == "a1") state.whiteCastleLong = false;
    else if (start == "h8") state.blackCastle = false;
    else if (start == "a8") state.blackCastleLong = false;

    // Fifty-move rule
    if (state.repeatedMoves >= 100) gameResult = GameResult::Draw;
    // Threefold repetition: current state + >= 2 in history = 3 total occurrences
    if (std::count(history.begin(), history.end(), state) >= 2)
        gameResult = GameResult::Draw;
}

bool Board::move(posn ini, posn tar, char promotion) {
    std::vector<posn> free, attack;
    legalMoves(ini, free, attack);
    if (std::find(free.begin(), free.end(), tar) != free.end() ||
        std::find(attack.begin(), attack.end(), tar) != attack.end()) {
        movePiece(ini, tar, promotion);
        state.whiteTurn = !state.whiteTurn;
        return true;
    }
    return false;
}

void Board::updateBoard() {
    bool hasLegalMoves = false;
    for (int r = 0; r < 8 && !hasLegalMoves; r++) {
        for (int c = 0; c < 8 && !hasLegalMoves; c++) {
            posn p(r, c);
            if (isEmpty(p)) continue;
            if (state.whiteTurn == isWhite(p)) {
                std::vector<posn> free, attack;
                legalMoves(p, free, attack);
                if (!free.empty() || !attack.empty())
                    hasLegalMoves = true;
            }
        }
    }
    if (!hasLegalMoves) {
        if (inCheck(state.whiteTurn))
            gameResult = state.whiteTurn ? GameResult::BlackWins : GameResult::WhiteWins;
        else
            gameResult = GameResult::Draw;
    }
}

bool Board::revert() {
    if (history.empty()) return false;
    state = history.back();
    history.pop_back();
    gameResult = GameResult::InProgress;
    return true;
}
