#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <clocale>
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

// Move cursor to top-left of board area and redraw in place
static int boardLines = 0; // track how many lines the board takes

static std::string squareBg(bool light, bool sel, bool isFree, bool isAttack) {
    if (sel)      return "\033[48;5;214m";
    if (isAttack) return "\033[48;5;167m";
    if (isFree)   return "\033[48;5;114m";
    return light  ? "\033[48;5;223m" : "\033[48;5;137m";
}

static void printBoard(const Board& game,
                        const std::vector<posn>& highlightFree = {},
                        const std::vector<posn>& highlightAttack = {},
                        posn selected = posn(-1, -1)) {
    // Move cursor up to overwrite previous board
    if (boardLines > 0)
        std::cout << "\033[" << boardLines << "A\033[J";

    int lines = 0;
    for (int r = 0; r < 8; r++) {
        // Top half of each square (blank row for height)
        std::cout << "    ";
        for (int c = 0; c < 8; c++) {
            bool light = (r + c) % 2 == 0;
            posn sq(r, c);
            bool isFree   = std::find(highlightFree.begin(),   highlightFree.end(),   sq) != highlightFree.end();
            bool isAttack = std::find(highlightAttack.begin(), highlightAttack.end(), sq) != highlightAttack.end();
            bool isSel    = selected.onBoard && sq == selected;
            std::cout << squareBg(light, isSel, isFree, isAttack) << "    " << Color::Reset;
        }
        std::cout << "\n";
        lines++;

        // Middle row with piece centered
        std::cout << " " << Color::Bold << (8 - r) << Color::Reset << " ";
        for (int c = 0; c < 8; c++) {
            bool light = (r + c) % 2 == 0;
            posn sq(r, c);
            bool isFree   = std::find(highlightFree.begin(),   highlightFree.end(),   sq) != highlightFree.end();
            bool isAttack = std::find(highlightAttack.begin(), highlightAttack.end(), sq) != highlightAttack.end();
            bool isSel    = selected.onBoard && sq == selected;
            char piece    = game.pieceAt(r, c);
            bool isWhiteP = piece != ' ' && std::isupper(static_cast<unsigned char>(piece));
            std::cout << squareBg(light, isSel, isFree, isAttack)
                      << (isWhiteP ? Color::White : Color::Black)
                      << " " << pieceSymbol(piece) << "  "
                      << Color::Reset;
        }
        std::cout << "\n";
        lines++;

        // Bottom half of each square
        std::cout << "    ";
        for (int c = 0; c < 8; c++) {
            bool light = (r + c) % 2 == 0;
            posn sq(r, c);
            bool isFree   = std::find(highlightFree.begin(),   highlightFree.end(),   sq) != highlightFree.end();
            bool isAttack = std::find(highlightAttack.begin(), highlightAttack.end(), sq) != highlightAttack.end();
            bool isSel    = selected.onBoard && sq == selected;
            std::cout << squareBg(light, isSel, isFree, isAttack) << "    " << Color::Reset;
        }
        std::cout << "\n";
        lines++;
    }

    // File labels
    std::cout << "    ";
    for (int c = 0; c < 8; c++)
        std::cout << Color::Bold << "  " << static_cast<char>('a' + c) << " " << Color::Reset;
    std::cout << "\n\n";
    lines += 2;

    boardLines = lines;
}

static void showHelp() {
    std::cout << Color::Cyan
              << "Commands:\n"
              << "  e2e4  - move piece from e2 to e4\n"
              << "  e2    - show legal moves for piece at e2\n"
              << "  back  - undo last move\n"
              << "  help  - show this help\n"
              << "  exit  - quit the game\n"
              << Color::Reset << "\n";
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

    // Clear screen and move to top
    std::cout << "\033[2J\033[H";
    std::cout << Color::Bold << Color::Cyan << "  Chess\n" << Color::Reset;
    showHelp();

    while (game.result() == GameResult::InProgress) {
        printBoard(game, showFree, showAttack, showSelected);
        showFree.clear();
        showAttack.clear();
        showSelected = posn(-1, -1);

        if (game.inCheck(game.getTurn()))
            std::cout << Color::Red << Color::Bold << "  Check!" << Color::Reset << "\n";

        std::cout << (game.getTurn() ? "  White" : "  Black") << " to move: ";
        std::string input;
        if (!std::getline(std::cin, input)) break;

        // Trim whitespace
        while (!input.empty() && input.front() == ' ') input.erase(input.begin());
        while (!input.empty() && input.back() == ' ') input.pop_back();

        if (input.empty()) continue;
        if (input == "exit" || input == "quit" || input == "q") break;
        if (input == "help" || input == "h" || input == "?") { showHelp(); continue; }
        if (input == "back" || input == "undo" || input == "z") {
            if (!game.revert())
                std::cout << Color::Yellow << "  Nothing to undo." << Color::Reset << "\n";
            continue;
        }

        if (input.length() == 2) {
            try {
                posn sq(input);
                std::vector<posn> free, attack;
                game.legalMoves(sq, free, attack);
                if (free.empty() && attack.empty()) {
                    std::cout << Color::Yellow << "  No legal moves for " << input << Color::Reset << "\n";
                } else {
                    showFree = free;
                    showAttack = attack;
                    showSelected = sq;
                }
            } catch (...) {
                std::cout << Color::Red << "  Invalid square: " << input << Color::Reset << "\n";
            }
            continue;
        }

        if (input.length() == 4 || input.length() == 5) {
            try {
                std::string from = input.substr(0, 2);
                std::string to = input.substr(2, 2);
                char promo = input.length() == 5 ? input[4] : 'Q';
                if (!game.move(posn(from), posn(to), promo)) {
                    std::cout << Color::Red << "  Illegal move: " << input << Color::Reset << "\n";
                    // Show legal moves as a hint
                    std::vector<posn> free, attack;
                    game.legalMoves(posn(from), free, attack);
                    if (!free.empty() || !attack.empty()) {
                        showFree = free;
                        showAttack = attack;
                        showSelected = posn(from);
                    }
                } else {
                    game.updateBoard();
                }
            } catch (...) {
                std::cout << Color::Red << "  Invalid input: " << input << Color::Reset << "\n";
            }
            continue;
        }

        std::cout << Color::Yellow << "  Unknown command. Type 'help' for usage." << Color::Reset << "\n";
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
