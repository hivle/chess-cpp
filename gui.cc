#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "chess.h"
#include "posn.h"

static const int SQ = 80;
static const int MARGIN = 32;
static const int BOARD_PX = SQ * 8;
static const int WIN_W = BOARD_PX + MARGIN * 2;
static const int WIN_H = BOARD_PX + MARGIN * 2 + 36;

// Chess piece characters (letter-based, clear and readable)
static const char* pieceLetter(char p) {
    switch (p) {
        case 'K': case 'k': return "K";
        case 'Q': case 'q': return "Q";
        case 'R': case 'r': return "R";
        case 'B': case 'b': return "B";
        case 'N': case 'n': return "N";
        case 'P': case 'p': return "P";
        default: return "";
    }
}

struct Colors {
    unsigned long lightSq, darkSq, selSq, freeDot, attackSq, checkSq;
    unsigned long whitePiece, blackPiece, bg, text, labelText;
    unsigned long btnBg, btnText, btnBorder;
};

static unsigned long makeColor(Display* dpy, int r, int g, int b) {
    XColor c;
    c.red = r * 257; c.green = g * 257; c.blue = b * 257;
    c.flags = DoRed | DoGreen | DoBlue;
    XAllocColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &c);
    return c.pixel;
}

static posn pixelToSquare(int x, int y) {
    int col = (x - MARGIN) / SQ;
    int row = (y - MARGIN) / SQ;
    if (col < 0 || col > 7 || row < 0 || row > 7)
        return posn(-1, -1);
    return posn(row, col);
}

static void drawText(Display* dpy, Window win, GC gc, XFontStruct* font,
                     int x, int y, const std::string& text, unsigned long color) {
    XSetForeground(dpy, gc, color);
    XSetFont(dpy, gc, font->fid);
    XDrawString(dpy, win, gc, x, y, text.c_str(), text.length());
}

static void drawCenteredText(Display* dpy, Window win, GC gc, XFontStruct* font,
                              int cx, int cy, const std::string& text, unsigned long color) {
    int tw = XTextWidth(font, text.c_str(), text.length());
    int th = font->ascent;
    drawText(dpy, win, gc, font, cx - tw / 2, cy + th / 2, text, color);
}

static void fillRect(Display* dpy, Window win, GC gc, int x, int y, int w, int h, unsigned long color) {
    XSetForeground(dpy, gc, color);
    XFillRectangle(dpy, win, gc, x, y, w, h);
}

static void fillCircle(Display* dpy, Window win, GC gc, int cx, int cy, int r, unsigned long color) {
    XSetForeground(dpy, gc, color);
    XFillArc(dpy, win, gc, cx - r, cy - r, r * 2, r * 2, 0, 360 * 64);
}

static char promotionDialog(Display* dpy, Window win, GC gc, XFontStruct* bigFont,
                             XFontStruct* smallFont, const Colors& colors) {
    const char choices[] = {'Q', 'R', 'B', 'N'};
    const char* labels[] = {"Queen", "Rook", "Bishop", "Knight"};
    int bw = 110, bh = 44, gap = 16;
    int totalW = 4 * bw + 3 * gap;
    int startX = (WIN_W - totalW) / 2;
    int y = WIN_H / 2 - bh / 2;

    // Darken background
    fillRect(dpy, win, gc, 0, 0, WIN_W, WIN_H, makeColor(dpy, 0, 0, 0));

    // Title
    drawCenteredText(dpy, win, gc, bigFont, WIN_W / 2, y - 30, "Promote pawn to:", colors.text);

    // Buttons
    for (int i = 0; i < 4; i++) {
        int bx = startX + i * (bw + gap);
        fillRect(dpy, win, gc, bx, y, bw, bh, colors.btnBg);
        XSetForeground(dpy, gc, colors.btnBorder);
        XDrawRectangle(dpy, win, gc, bx, y, bw, bh);
        drawCenteredText(dpy, win, gc, smallFont, bx + bw / 2, y + bh / 2, labels[i], colors.btnText);
    }
    XFlush(dpy);

    while (true) {
        XEvent ev;
        XNextEvent(dpy, &ev);
        if (ev.type == ButtonPress && ev.xbutton.button == 1) {
            int mx = ev.xbutton.x, my = ev.xbutton.y;
            if (my >= y && my <= y + bh) {
                for (int i = 0; i < 4; i++) {
                    int bx = startX + i * (bw + gap);
                    if (mx >= bx && mx <= bx + bw)
                        return choices[i];
                }
            }
        }
        if (ev.type == KeyPress) {
            KeySym key = XLookupKeysym(&ev.xkey, 0);
            if (key == XK_q) return 'Q';
            if (key == XK_r) return 'R';
            if (key == XK_b) return 'B';
            if (key == XK_n) return 'N';
        }
    }
}

int main() {
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return 1;
    int screen = DefaultScreen(dpy);

    Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen),
                                      100, 100, WIN_W, WIN_H, 0,
                                      BlackPixel(dpy, screen), BlackPixel(dpy, screen));

    XStoreName(dpy, win, "Chess");

    // Set minimum/maximum size to prevent resizing
    XSizeHints hints;
    hints.flags = PMinSize | PMaxSize;
    hints.min_width = hints.max_width = WIN_W;
    hints.min_height = hints.max_height = WIN_H;
    XSetWMNormalHints(dpy, win, &hints);

    XSelectInput(dpy, win, ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);

    // Handle window close
    Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDelete, 1);

    XMapWindow(dpy, win);
    GC gc = XCreateGC(dpy, win, 0, nullptr);

    // Load fonts
    XFontStruct* bigFont = XLoadQueryFont(dpy, "-*-dejavu sans-bold-r-*-*-32-*-*-*-*-*-*-*");
    if (!bigFont) bigFont = XLoadQueryFont(dpy, "-*-helvetica-bold-r-*-*-34-*-*-*-*-*-*-*");
    if (!bigFont) bigFont = XLoadQueryFont(dpy, "fixed");

    XFontStruct* smallFont = XLoadQueryFont(dpy, "-*-dejavu sans-medium-r-*-*-14-*-*-*-*-*-*-*");
    if (!smallFont) smallFont = XLoadQueryFont(dpy, "-*-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*");
    if (!smallFont) smallFont = XLoadQueryFont(dpy, "fixed");

    XFontStruct* labelFont = XLoadQueryFont(dpy, "-*-dejavu sans-medium-r-*-*-13-*-*-*-*-*-*-*");
    if (!labelFont) labelFont = smallFont;

    Colors colors;
    colors.lightSq    = makeColor(dpy, 240, 217, 181);
    colors.darkSq     = makeColor(dpy, 181, 136, 99);
    colors.selSq      = makeColor(dpy, 246, 190, 0);
    colors.freeDot    = makeColor(dpy, 100, 180, 100);
    colors.attackSq   = makeColor(dpy, 220, 80, 80);
    colors.checkSq    = makeColor(dpy, 255, 60, 60);
    colors.whitePiece = makeColor(dpy, 255, 255, 255);
    colors.blackPiece = makeColor(dpy, 30, 30, 30);
    colors.bg         = makeColor(dpy, 40, 40, 40);
    colors.text       = makeColor(dpy, 220, 220, 220);
    colors.labelText  = makeColor(dpy, 180, 180, 180);
    colors.btnBg      = makeColor(dpy, 60, 60, 60);
    colors.btnText    = makeColor(dpy, 255, 255, 255);
    colors.btnBorder  = makeColor(dpy, 150, 150, 150);

    Board game;
    posn selected(-1, -1);
    std::vector<posn> freeMoves, attackMoves;
    std::string statusMsg;
    bool needRedraw = true;

    // Double buffering with Pixmap
    Pixmap buffer = XCreatePixmap(dpy, win, WIN_W, WIN_H, DefaultDepth(dpy, screen));

    while (true) {
        while (XPending(dpy)) {
            XEvent ev;
            XNextEvent(dpy, &ev);

            if (ev.type == ClientMessage && (Atom)ev.xclient.data.l[0] == wmDelete)
                goto done;

            if (ev.type == Expose)
                needRedraw = true;

            if (ev.type == KeyPress) {
                KeySym key = XLookupKeysym(&ev.xkey, 0);
                if (key == XK_Escape) goto done;
                if (key == XK_z || key == XK_u) {
                    game.revert();
                    selected = posn(-1, -1);
                    freeMoves.clear();
                    attackMoves.clear();
                    statusMsg = "Move undone";
                    needRedraw = true;
                }
            }

            if (ev.type == ButtonPress) {
                if (ev.xbutton.button == 3) { // Right click = deselect
                    selected = posn(-1, -1);
                    freeMoves.clear();
                    attackMoves.clear();
                    statusMsg.clear();
                    needRedraw = true;
                    continue;
                }
                if (ev.xbutton.button != 1) continue;
                if (game.result() != GameResult::InProgress) continue;

                posn clicked = pixelToSquare(ev.xbutton.x, ev.xbutton.y);
                if (!clicked.onBoard) continue;

                if (selected.onBoard) {
                    bool isFree = std::find(freeMoves.begin(), freeMoves.end(), clicked) != freeMoves.end();
                    bool isAtk = std::find(attackMoves.begin(), attackMoves.end(), clicked) != attackMoves.end();

                    if (isFree || isAtk) {
                        char promo = 'Q';
                        char piece = game.pieceAt(selected.getRow(), selected.getCol());
                        if ((piece == 'P' && clicked.getRow() == 0) ||
                            (piece == 'p' && clicked.getRow() == 7)) {
                            promo = promotionDialog(dpy, win, gc, bigFont, smallFont, colors);
                        }
                        game.move(selected, clicked, promo);
                        game.updateBoard();
                        selected = posn(-1, -1);
                        freeMoves.clear();
                        attackMoves.clear();
                        statusMsg.clear();

                        if (game.result() != GameResult::InProgress) {
                            switch (game.result()) {
                                case GameResult::WhiteWins: statusMsg = "Checkmate! White wins!"; break;
                                case GameResult::BlackWins: statusMsg = "Checkmate! Black wins!"; break;
                                case GameResult::Draw: statusMsg = "Game drawn."; break;
                                default: break;
                            }
                        } else if (game.inCheck(game.getTurn())) {
                            statusMsg = "Check!";
                        }
                    } else {
                        char p = game.pieceAt(clicked.getRow(), clicked.getCol());
                        bool own = (game.getTurn() && std::isupper((unsigned char)p)) ||
                                   (!game.getTurn() && std::islower((unsigned char)p));
                        if (own && clicked != selected) {
                            selected = clicked;
                            freeMoves.clear();
                            attackMoves.clear();
                            game.legalMoves(selected, freeMoves, attackMoves);
                            statusMsg.clear();
                        } else {
                            selected = posn(-1, -1);
                            freeMoves.clear();
                            attackMoves.clear();
                        }
                    }
                } else {
                    char p = game.pieceAt(clicked.getRow(), clicked.getCol());
                    if (p == ' ') continue;
                    bool own = (game.getTurn() && std::isupper((unsigned char)p)) ||
                               (!game.getTurn() && std::islower((unsigned char)p));
                    if (own) {
                        selected = clicked;
                        freeMoves.clear();
                        attackMoves.clear();
                        game.legalMoves(selected, freeMoves, attackMoves);
                        statusMsg.clear();
                    }
                }
                needRedraw = true;
            }
        }

        if (!needRedraw) {
            usleep(16000); // ~60fps
            continue;
        }
        needRedraw = false;

        // --- Render to buffer ---
        fillRect(dpy, buffer, gc, 0, 0, WIN_W, WIN_H, colors.bg);

        posn kingPos = game.locateKing(game.getTurn());
        bool isInCheck = game.inCheck(game.getTurn());

        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                int x = MARGIN + c * SQ;
                int y = MARGIN + r * SQ;
                bool light = (r + c) % 2 == 0;
                posn sq(r, c);

                // Square color
                unsigned long sqColor = light ? colors.lightSq : colors.darkSq;
                fillRect(dpy, buffer, gc, x, y, SQ, SQ, sqColor);

                // Check highlight
                if (isInCheck && sq == kingPos)
                    fillRect(dpy, buffer, gc, x, y, SQ, SQ, colors.checkSq);

                // Selection highlight
                if (selected.onBoard && sq == selected)
                    fillRect(dpy, buffer, gc, x, y, SQ, SQ, colors.selSq);

                // Legal move highlights
                bool isFree = std::find(freeMoves.begin(), freeMoves.end(), sq) != freeMoves.end();
                bool isAtk = std::find(attackMoves.begin(), attackMoves.end(), sq) != attackMoves.end();

                if (isAtk) {
                    // Red border for captures
                    XSetForeground(dpy, gc, colors.attackSq);
                    XSetLineAttributes(dpy, gc, 4, LineSolid, CapButt, JoinMiter);
                    XDrawRectangle(dpy, buffer, gc, x + 2, y + 2, SQ - 4, SQ - 4);
                    XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
                }
                if (isFree)
                    fillCircle(dpy, buffer, gc, x + SQ / 2, y + SQ / 2, SQ / 6, colors.freeDot);

                // Draw piece
                char piece = game.pieceAt(r, c);
                if (piece != ' ') {
                    const char* letter = pieceLetter(piece);
                    bool isW = std::isupper((unsigned char)piece);
                    unsigned long pieceColor = isW ? colors.whitePiece : colors.blackPiece;

                    // Draw piece shadow/outline for contrast
                    unsigned long outlineColor = isW ? makeColor(dpy, 60, 60, 60) : makeColor(dpy, 200, 200, 200);
                    int tw = XTextWidth(bigFont, letter, strlen(letter));
                    int tx = x + (SQ - tw) / 2;
                    int ty = y + SQ / 2 + bigFont->ascent / 2;

                    // Outline (draw at offsets)
                    XSetFont(dpy, gc, bigFont->fid);
                    XSetForeground(dpy, gc, outlineColor);
                    for (int dx = -1; dx <= 1; dx++)
                        for (int dy = -1; dy <= 1; dy++)
                            if (dx || dy)
                                XDrawString(dpy, buffer, gc, tx + dx, ty + dy, letter, strlen(letter));

                    // Main piece
                    XSetForeground(dpy, gc, pieceColor);
                    XDrawString(dpy, buffer, gc, tx, ty, letter, strlen(letter));
                }
            }
        }

        // Rank labels (1-8)
        for (int i = 0; i < 8; i++) {
            std::string label = std::to_string(8 - i);
            int lx = MARGIN / 2 - XTextWidth(labelFont, label.c_str(), label.length()) / 2;
            int ly = MARGIN + i * SQ + SQ / 2 + labelFont->ascent / 2;
            drawText(dpy, buffer, gc, labelFont, lx, ly, label, colors.labelText);
        }

        // File labels (a-h)
        for (int i = 0; i < 8; i++) {
            std::string label(1, 'a' + i);
            int lx = MARGIN + i * SQ + SQ / 2 - XTextWidth(labelFont, label.c_str(), 1) / 2;
            int ly = MARGIN + BOARD_PX + labelFont->ascent + 4;
            drawText(dpy, buffer, gc, labelFont, lx, ly, label, colors.labelText);
        }

        // Status bar
        std::string status;
        if (!statusMsg.empty())
            status = statusMsg;
        else if (game.result() == GameResult::InProgress)
            status = game.getTurn() ? "White to move" : "Black to move";

        drawText(dpy, buffer, gc, smallFont, MARGIN, MARGIN + BOARD_PX + 30, status, colors.text);

        // Help text
        std::string help = "Z=Undo  Esc=Quit  Right-click=Deselect";
        int hw = XTextWidth(smallFont, help.c_str(), help.length());
        drawText(dpy, buffer, gc, smallFont, WIN_W - MARGIN - hw, MARGIN + BOARD_PX + 30, help, colors.labelText);

        // Copy buffer to window
        XCopyArea(dpy, buffer, win, gc, 0, 0, WIN_W, WIN_H, 0, 0);
        XFlush(dpy);
    }

done:
    XFreePixmap(dpy, buffer);
    if (bigFont) XFreeFont(dpy, bigFont);
    if (smallFont && smallFont != bigFont) XFreeFont(dpy, smallFont);
    if (labelFont && labelFont != smallFont && labelFont != bigFont) XFreeFont(dpy, labelFont);
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
