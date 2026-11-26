// Native Windows GUI (Win32 GDI). Build with `make gui` on Windows.
// Click a piece to select it, click a highlighted square to move.
// Right-click deselects, Z (or Ctrl+Z) undoes, Esc quits. The window can be resized.
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <windowsx.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include "chess.h"
#include "posn.h"

namespace {

// --- Colors ---
const COLORREF LIGHT_SQ   = RGB(240, 217, 181);
const COLORREF DARK_SQ    = RGB(181, 136, 99);
const COLORREF SELECTED   = RGB(246, 200, 60);
const COLORREF LAST_LIGHT = RGB(225, 225, 130);
const COLORREF LAST_DARK  = RGB(190, 180, 80);
const COLORREF CHECK_SQ   = RGB(235, 80, 70);
const COLORREF MOVE_DOT   = RGB(90, 150, 80);
const COLORREF CAPTURE    = RGB(200, 60, 60);
const COLORREF BACKGROUND = RGB(40, 40, 40);
const COLORREF LABEL_TEXT = RGB(190, 190, 190);
const COLORREF STATUS_TXT = RGB(235, 235, 235);
const COLORREF HINT_TEXT  = RGB(140, 140, 140);

const int STATUS_H = 36;   // status bar height below the board

// --- Game / UI state ---
Board game;
posn selected(-1, -1);
std::vector<posn> freeMoves, attackMoves;
posn lastFrom(-1, -1), lastTo(-1, -1);
std::string statusMsg;
bool promoting = false;    // promotion picker is open
posn promoFrom(-1, -1), promoTo(-1, -1);

// Board geometry, recomputed from the window size on every paint / click
struct Layout { int sq, left, top, margin; };

Layout layoutFor(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int w = rc.right - rc.left, h = rc.bottom - rc.top - STATUS_H;
    int margin = std::max(18, std::min(w, h) / 22);
    int sq = std::max(8, (std::min(w, h) - 2 * margin) / 8);
    Layout L;
    L.sq = sq;
    L.margin = margin;
    L.left = (w - sq * 8) / 2;
    L.top = (h - sq * 8) / 2;
    return L;
}

bool contains(const std::vector<posn>& v, posn p) {
    return std::find(v.begin(), v.end(), p) != v.end();
}

void clearSelection() {
    selected = posn(-1, -1);
    freeMoves.clear();
    attackMoves.clear();
}

bool ownPiece(char p) {
    if (p == ' ') return false;
    bool upper = std::isupper(static_cast<unsigned char>(p)) != 0;
    return game.getTurn() ? upper : !upper;
}

void select(posn sq) {
    clearSelection();
    selected = sq;
    game.legalMoves(sq, freeMoves, attackMoves);
}

void afterMove() {
    game.updateBoard();
    clearSelection();
    statusMsg.clear();
    switch (game.result()) {
        case GameResult::WhiteWins: statusMsg = "Checkmate - White wins!"; break;
        case GameResult::BlackWins: statusMsg = "Checkmate - Black wins!"; break;
        case GameResult::Draw:      statusMsg = "Draw.";                  break;
        default: if (game.inCheck(game.getTurn())) statusMsg = "Check!"; break;
    }
}

void doMove(posn from, posn to, char promo) {
    if (game.move(from, to, promo)) {
        lastFrom = from;
        lastTo = to;
        afterMove();
    }
}

// --- Drawing helpers ---

void fill(HDC dc, int x, int y, int w, int h, COLORREF c) {
    RECT r = {x, y, x + w, y + h};
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

HFONT makeFont(int px, const wchar_t* face, int weight = FW_NORMAL) {
    return CreateFontW(-px, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                       OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                       DEFAULT_PITCH, face);
}

void text(HDC dc, const std::wstring& s, RECT r, COLORREF c, UINT fmt) {
    SetTextColor(dc, c);
    DrawTextW(dc, s.c_str(), static_cast<int>(s.size()), &r, fmt | DT_SINGLELINE | DT_NOPREFIX);
}

// Unicode chess glyphs: outline set (U+2654..) and filled set (U+265A..)
wchar_t outlineGlyph(char p) {
    switch (std::toupper(static_cast<unsigned char>(p))) {
        case 'K': return 0x2654; case 'Q': return 0x2655; case 'R': return 0x2656;
        case 'B': return 0x2657; case 'N': return 0x2658; default:  return 0x2659;
    }
}
wchar_t filledGlyph(char p) { return static_cast<wchar_t>(outlineGlyph(p) + 6); }

// Draw a piece centered on (cx, cy) by its actual ink box, not the font's line box,
// so it is centered both ways no matter how the font positions the glyph.
// White pieces: filled glyph in white, then the outline glyph in black on top.
// Black pieces: filled glyph in near-black.
void drawPiece(HDC dc, char piece, int cx, int cy, HFONT font) {
    HGDIOBJ old = SelectObject(dc, font);
    wchar_t outline = outlineGlyph(piece), filled = filledGlyph(piece);

    GLYPHMETRICS gm;
    MAT2 identity = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
    int x, y;
    if (GetGlyphOutlineW(dc, outline, GGO_METRICS, &gm, 0, nullptr, &identity) != GDI_ERROR) {
        x = cx - static_cast<int>(gm.gmBlackBoxX) / 2 - gm.gmptGlyphOrigin.x;
        y = cy - static_cast<int>(gm.gmBlackBoxY) / 2 + gm.gmptGlyphOrigin.y;
    } else {  // fallback: center by text extent
        SIZE sz;
        GetTextExtentPoint32W(dc, &outline, 1, &sz);
        TEXTMETRICW tm;
        GetTextMetricsW(dc, &tm);
        x = cx - sz.cx / 2;
        y = cy + tm.tmAscent / 2;
    }

    UINT oldAlign = SetTextAlign(dc, TA_BASELINE | TA_LEFT);
    if (std::isupper(static_cast<unsigned char>(piece))) {
        SetTextColor(dc, RGB(255, 255, 255));
        TextOutW(dc, x, y, &filled, 1);
        SetTextColor(dc, RGB(20, 20, 20));
        TextOutW(dc, x, y, &outline, 1);
    } else {
        SetTextColor(dc, RGB(25, 25, 25));
        TextOutW(dc, x, y, &filled, 1);
    }
    SetTextAlign(dc, oldAlign);
    SelectObject(dc, old);
}

// Rectangles of the 4 promotion choices (Q R B N), centered over the board
RECT promoChoice(const Layout& L, int i) {
    int size = L.sq;
    int total = 4 * size;
    int x0 = L.left + (8 * L.sq - total) / 2;
    int y0 = L.top + (8 * L.sq - size) / 2;
    RECT r = {x0 + i * size, y0, x0 + (i + 1) * size, y0 + size};
    return r;
}
const char PROMO_PIECES[4] = {'Q', 'R', 'B', 'N'};

void paint(HWND hwnd, HDC target) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int W = rc.right, H = rc.bottom;
    Layout L = layoutFor(hwnd);

    // Draw into an off-screen bitmap, then copy it once (no flicker)
    HDC dc = CreateCompatibleDC(target);
    HBITMAP bmp = CreateCompatibleBitmap(target, std::max(W, 1), std::max(H, 1));
    HGDIOBJ oldBmp = SelectObject(dc, bmp);
    SetBkMode(dc, TRANSPARENT);
    fill(dc, 0, 0, W, H, BACKGROUND);

    HFONT pieceFont = makeFont(L.sq * 4 / 5, L"Segoe UI Symbol");
    HFONT labelFont = makeFont(std::max(11, L.margin * 2 / 3), L"Segoe UI", FW_SEMIBOLD);
    HFONT statusFont = makeFont(17, L"Segoe UI", FW_SEMIBOLD);
    HFONT hintFont = makeFont(13, L"Segoe UI");

    bool inCheck = game.result() == GameResult::InProgress && game.inCheck(game.getTurn());
    posn king = game.locateKing(game.getTurn());
    bool whiteToMove = game.getTurn();

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            posn sq(r, c);
            int x = L.left + c * L.sq, y = L.top + r * L.sq;
            bool light = (r + c) % 2 == 0;

            COLORREF base = light ? LIGHT_SQ : DARK_SQ;
            if (lastFrom.onBoard && (sq == lastFrom || sq == lastTo)) base = light ? LAST_LIGHT : LAST_DARK;
            if (selected.onBoard && sq == selected) base = SELECTED;
            if (inCheck && sq == king) base = CHECK_SQ;
            fill(dc, x, y, L.sq, L.sq, base);

            if (contains(attackMoves, sq)) {  // capture: ring around the square's edge
                HPEN pen = CreatePen(PS_SOLID, std::max(3, L.sq / 14), CAPTURE);
                HGDIOBJ op = SelectObject(dc, pen), ob = SelectObject(dc, GetStockObject(NULL_BRUSH));
                int inset = std::max(2, L.sq / 28);
                Rectangle(dc, x + inset, y + inset, x + L.sq - inset + 1, y + L.sq - inset + 1);
                SelectObject(dc, op); SelectObject(dc, ob);
                DeleteObject(pen);
            }

            char piece = game.pieceAt(r, c);
            if (piece != ' ')
                drawPiece(dc, piece, x + L.sq / 2, y + L.sq / 2, pieceFont);

            if (contains(freeMoves, sq)) {  // quiet move: dot in the middle
                int rad = std::max(4, L.sq / 7);
                HBRUSH b = CreateSolidBrush(MOVE_DOT);
                HGDIOBJ ob = SelectObject(dc, b), op = SelectObject(dc, GetStockObject(NULL_PEN));
                Ellipse(dc, x + L.sq / 2 - rad, y + L.sq / 2 - rad, x + L.sq / 2 + rad + 1, y + L.sq / 2 + rad + 1);
                SelectObject(dc, ob); SelectObject(dc, op);
                DeleteObject(b);
            }
        }
    }

    // Rank (8..1) and file (a..h) labels in the margins
    HGDIOBJ oldFont = SelectObject(dc, labelFont);
    for (int i = 0; i < 8; i++) {
        RECT rr = {L.left - L.margin, L.top + i * L.sq, L.left, L.top + (i + 1) * L.sq};
        text(dc, std::wstring(1, static_cast<wchar_t>(L'8' - i)), rr, LABEL_TEXT, DT_CENTER | DT_VCENTER);
        RECT fr = {L.left + i * L.sq, L.top + 8 * L.sq, L.left + (i + 1) * L.sq, L.top + 8 * L.sq + L.margin};
        text(dc, std::wstring(1, static_cast<wchar_t>(L'a' + i)), fr, LABEL_TEXT, DT_CENTER | DT_VCENTER);
    }

    // Status bar: whose turn / check / result on the left, controls on the right
    std::string status = statusMsg;
    if (status.empty()) status = whiteToMove ? "White to move" : "Black to move";
    if (promoting) status = "Promote to: click a piece (or press Q / R / B / N)";
    RECT sr = {16, H - STATUS_H, W - 16, H};
    SelectObject(dc, statusFont);
    text(dc, std::wstring(status.begin(), status.end()), sr, STATUS_TXT, DT_LEFT | DT_VCENTER);
    SelectObject(dc, hintFont);
    text(dc, L"Z: undo   Right-click: deselect   Esc: quit", sr, HINT_TEXT, DT_RIGHT | DT_VCENTER);
    SelectObject(dc, oldFont);

    // Promotion picker drawn over the board
    if (promoting) {
        RECT all = promoChoice(L, 0), last = promoChoice(L, 3);
        int pad = L.sq / 6;
        fill(dc, all.left - pad, all.top - pad, last.right - all.left + 2 * pad, all.bottom - all.top + 2 * pad, RGB(60, 60, 60));
        for (int i = 0; i < 4; i++) {
            RECT b = promoChoice(L, i);
            fill(dc, b.left + 2, b.top + 2, b.right - b.left - 4, b.bottom - b.top - 4, i % 2 ? DARK_SQ : LIGHT_SQ);
            char p = whiteToMove ? PROMO_PIECES[i] : static_cast<char>(std::tolower(PROMO_PIECES[i]));
            drawPiece(dc, p, (b.left + b.right) / 2, (b.top + b.bottom) / 2, pieceFont);
        }
    }

    BitBlt(target, 0, 0, W, H, dc, 0, 0, SRCCOPY);

    DeleteObject(pieceFont);
    DeleteObject(labelFont);
    DeleteObject(statusFont);
    DeleteObject(hintFont);
    SelectObject(dc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dc);
}

// --- Input ---

void onLeftClick(HWND hwnd, int mx, int my) {
    Layout L = layoutFor(hwnd);

    if (promoting) {
        for (int i = 0; i < 4; i++) {
            RECT b = promoChoice(L, i);
            if (mx >= b.left && mx < b.right && my >= b.top && my < b.bottom) {
                promoting = false;
                doMove(promoFrom, promoTo, PROMO_PIECES[i]);
                return;
            }
        }
        return;  // must pick a piece (Esc cancels)
    }
    if (game.result() != GameResult::InProgress) return;

    if (mx < L.left || my < L.top) return;
    int c = (mx - L.left) / L.sq, r = (my - L.top) / L.sq;
    if (r < 0 || r > 7 || c < 0 || c > 7) return;
    posn clicked(r, c);
    char piece = game.pieceAt(r, c);

    if (selected.onBoard && (contains(freeMoves, clicked) || contains(attackMoves, clicked))) {
        char moving = game.pieceAt(selected.getRow(), selected.getCol());
        if ((moving == 'P' && r == 0) || (moving == 'p' && r == 7)) {
            promoting = true;  // ask which piece first
            promoFrom = selected;
            promoTo = clicked;
            return;
        }
        doMove(selected, clicked, 'Q');
    } else if (ownPiece(piece) && !(selected.onBoard && clicked == selected)) {
        select(clicked);
        statusMsg.clear();
    } else {
        clearSelection();
    }
}

void onKey(HWND hwnd, WPARAM key) {
    if (promoting) {
        for (char p : PROMO_PIECES) {
            if (key == static_cast<WPARAM>(p)) {
                promoting = false;
                doMove(promoFrom, promoTo, p);
                return;
            }
        }
        if (key == VK_ESCAPE) promoting = false;  // cancel the promotion move
        return;
    }
    if (key == VK_ESCAPE) {
        DestroyWindow(hwnd);
    } else if (key == 'Z' || key == 'U') {
        if (game.revert()) {
            clearSelection();
            lastFrom = lastTo = posn(-1, -1);
            statusMsg = "Move undone";
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            paint(hwnd, dc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;  // everything is painted in WM_PAINT; avoids flicker
        case WM_SIZE:
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_GETMINMAXINFO: {
            auto* mm = reinterpret_cast<MINMAXINFO*>(lp);
            mm->ptMinTrackSize.x = 360;
            mm->ptMinTrackSize.y = 400;
            return 0;
        }
        case WM_LBUTTONDOWN:
            onLeftClick(hwnd, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_RBUTTONDOWN:
            if (!promoting) { clearSelection(); InvalidateRect(hwnd, nullptr, FALSE); }
            return 0;
        case WM_KEYDOWN:
            onKey(hwnd, wp);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    SetProcessDPIAware();  // crisp text on high-DPI screens instead of blurry scaling

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpszClassName = L"ChessWindow";
    RegisterClassW(&wc);

    // Size the window so the client area fits a board of 80px squares plus margins
    int sq = 80, margin = 32;
    RECT r = {0, 0, sq * 8 + margin * 2, sq * 8 + margin * 2 + STATUS_H};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowW(L"ChessWindow", L"Chess", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                              nullptr, nullptr, inst, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    return 0;
}
