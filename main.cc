#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <clocale>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif
#include "chess.h"
#include "posn.h"

// ANSI color codes
namespace Color {
    const char* Reset   = "\033[0m";
    const char* Bold    = "\033[1m";
    const char* Red     = "\033[31m";
    const char* Green   = "\033[32m";
    const char* Yellow  = "\033[33m";
    const char* Blue    = "\033[34m";
    const char* Cyan    = "\033[36m";
    const char* BgLight = "\033[48;5;223m";
    const char* BgDark  = "\033[48;5;137m";
    const char* White   = "\033[97m";
    const char* Black   = "\033[30m";
}

static const char* pieceSymbol(char p) {
    switch (p) {
        case 'K': return "\u2654"; case 'Q': return "\u2655";
        case 'R': return "\u2656"; case 'B': return "\u2657";
        case 'N': return "\u2658"; case 'P': return "\u2659";
        case 'k': return "\u265A"; case 'q': return "\u265B";
        case 'r': return "\u265C"; case 'b': return "\u265D";
        case 'n': return "\u265E"; case 'p': return "\u265F";
        default:  return " ";
    }
}

static void setBg(bool light, bool sel, bool isFree, bool isAtk) {
    if      (sel)    std::cout << "\033[48;5;214m";
    else if (isAtk)  std::cout << "\033[48;5;167m";
    else if (isFree) std::cout << "\033[48;5;114m";
    else             std::cout << (light ? "\033[48;5;223m" : "\033[48;5;137m");
}

static void printBoard(const Board& game,
                        const std::vector<posn>& highlightFree = {},
                        const std::vector<posn>& highlightAttack = {},
                        posn selected = posn(-1, -1)) {
    // Go to top of screen and clear everything below — avoids cursor-up miscounting
    std::cout << "\033[H\033[J";

    for (int r = 0; r < 8; r++) {
        std::cout << " " << Color::Bold << (8 - r) << Color::Reset << " ";
        for (int c = 0; c < 8; c++) {
            bool light = (r + c) % 2 == 0;
            posn sq(r, c);
            bool isFree = std::find(highlightFree.begin(),   highlightFree.end(),   sq) != highlightFree.end();
            bool isAtk  = std::find(highlightAttack.begin(), highlightAttack.end(), sq) != highlightAttack.end();
            bool isSel  = selected.onBoard && sq == selected;
            char piece  = game.pieceAt(r, c);
            setBg(light, isSel, isFree, isAtk);
            if (piece != ' ' && std::isupper((unsigned char)piece)) std::cout << Color::White;
            else                                                     std::cout << Color::Black;
            std::cout << " " << pieceSymbol(piece) << " \033[0m";
        }
        std::cout << "\n";
    }

    // File labels — 3 chars per cell, letter at center position
    std::cout << "   ";
    for (int c = 0; c < 8; c++)
        std::cout << " " << Color::Bold << static_cast<char>('a' + c) << Color::Reset << " ";
    std::cout << "\n\n";
}

static std::string helpText() {
    return std::string(Color::Cyan) +
        "  Commands:\n"
        "    e2e4   move piece from e2 to e4 (add q/r/b/n to promote, e.g. e7e8n)\n"
        "    e2     show legal moves for the piece on e2\n"
        "    back   undo last move (also: undo, z)\n"
        "    help   show this help (also: h, ?)\n"
        "    exit   quit (also: quit, q)\n" + Color::Reset;
}

static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static std::string colored(const char* color, const std::string& text) {
    return std::string(color) + "  " + text + Color::Reset;
}

int main() {
    // Enable UTF-8 output
    std::setlocale(LC_ALL, "en_US.UTF-8");
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    Board game;
    std::vector<posn> showFree, showAttack;
    posn showSelected(-1, -1);
    // Printed under the board on the next redraw (the redraw clears the screen)
    std::string message = helpText();

    while (game.result() == GameResult::InProgress) {
        printBoard(game, showFree, showAttack, showSelected);
        showFree.clear();
        showAttack.clear();
        showSelected = posn(-1, -1);

        if (!message.empty()) std::cout << message << "\n";
        message.clear();

        if (game.inCheck(game.getTurn()))
            std::cout << Color::Red << Color::Bold << "  Check!" << Color::Reset << "\n";

        std::cout << (game.getTurn() ? "  White" : "  Black") << " to move: " << std::flush;
        std::string input;
        if (!std::getline(std::cin, input)) break;
        input = trim(input);
        for (char& ch : input) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        if (input.empty()) continue;
        if (input == "exit" || input == "quit" || input == "q") break;
        if (input == "help" || input == "h" || input == "?") { message = helpText(); continue; }
        if (input == "back" || input == "undo" || input == "z") {
            if (!game.revert()) message = colored(Color::Yellow, "Nothing to undo.");
            continue;
        }

        if (input.length() == 2) {
            try {
                posn sq(input);
                std::vector<posn> free, attack;
                game.legalMoves(sq, free, attack);
                if (free.empty() && attack.empty()) {
                    message = colored(Color::Yellow, "No legal moves for " + input);
                } else {
                    showFree = free;
                    showAttack = attack;
                    showSelected = sq;
                }
            } catch (...) {
                message = colored(Color::Red, "Invalid square: " + input);
            }
            continue;
        }

        if (input.length() == 4 || input.length() == 5) {
            try {
                posn from(input.substr(0, 2));
                posn to(input.substr(2, 2));
                char promo = input.length() == 5 ? input[4] : 'q';
                if (game.move(from, to, promo)) {
                    game.updateBoard();
                } else {
                    message = colored(Color::Red, "Illegal move: " + input);
                    // Show the piece's legal moves as a hint
                    std::vector<posn> free, attack;
                    game.legalMoves(from, free, attack);
                    if (!free.empty() || !attack.empty()) {
                        showFree = free;
                        showAttack = attack;
                        showSelected = from;
                    }
                }
            } catch (...) {
                message = colored(Color::Red, "Invalid input: " + input);
            }
            continue;
        }

        message = colored(Color::Yellow, "Unknown command. Type 'help' for usage.");
    }

    printBoard(game);
    switch (game.result()) {
        case GameResult::WhiteWins:
            std::cout << Color::Bold << Color::Green << "  Checkmate! White wins!" << Color::Reset << "\n"; break;
        case GameResult::BlackWins:
            std::cout << Color::Bold << Color::Green << "  Checkmate! Black wins!" << Color::Reset << "\n"; break;
        case GameResult::Draw:
            std::cout << Color::Bold << Color::Yellow << "  Game drawn." << Color::Reset << "\n"; break;
        default: break;
    }
    return 0;
}
