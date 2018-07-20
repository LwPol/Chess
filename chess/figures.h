#pragma once
#include <Windows.h>
#include <stack>
#include <list>

#define CM_ALLOWSELECT 0x8001
#define CM_SELECTED 0x8002
#define CM_MATINDICATOR 0x8003
#define CM_PAWNPROMOTION 0x8004

#define ST_DEFAULT 0
#define ST_SELECTED 1
#define ST_SELECTALLOWED 2
#define ST_SHAH 3
#define ST_SHAHING 4
#define ST_ENPASSANT 5

#define PROMOTION_ROOK 0
#define PROMOTION_KNIGHT 1
#define PROMOTION_BISHOP 2
#define PROMOTION_QUEEN 3

#define ID_NAMESTABLE 200

#define CHESS_FIRSTSTART 0x01
#define CHESS_WHITESTURN 0x02
#define CHESS_CHECK 0x04

int Absolute(int);

typedef struct {
	DWORD dwState;
	POINT ptPos;
} CHESSFIELDDATA, *LPCHESSFIELDDATA;

class PlayerNames {
public:
	LPWSTR szFirst;
	LPWSTR szSecond;

	PlayerNames();
	~PlayerNames();
	bool SetName(int, LPCWSTR, int);
};

class Figure {
public:
	bool bIsWhite;
	POINT ptPos;
	Figure(bool, int, int);
	virtual ~Figure();
	virtual void draw() = 0;
	virtual void showmoves() = 0;
	virtual bool isinrange(POINT) = 0;
	virtual void move(POINT);
	void draw(HBITMAP, HBITMAP);
};

class GameData {
public:
	static PlayerNames pnNames;
	static BYTE flags;
	static HBITMAP hbmChessboard;
	static HWND **hFieldsTable;
	static HWND hSelected;
	static std::stack<HWND> PossibleToMove;
	static Figure **whiteFigures;
	static Figure **blackFigures;
	static HWND hShahing;
	static Figure *enPassant;
	static std::list<Figure*> whitePromoted;
	static std::list<Figure*> blackPromoted;
};

class Pawn : public Figure {
public:
	static HBITMAP hbmPawn;
	static HBITMAP hbmMask;
	Pawn(bool, int, int);
	void draw();
	void showmoves();
	bool isinrange(POINT);
	void move(POINT);
	Figure* promote(int);
};

class Rook : public Figure {
public:
	static HBITMAP hbmRook;
	static HBITMAP hbmMask;
	bool bWasMoved;
	Rook(bool, int, int);
	void draw();
	void showmoves();
	bool isinrange(POINT);
	void move(POINT);
};

class Knight : public Figure {
public:
	static HBITMAP hbmKnight;
	static HBITMAP hbmMask;
	Knight(bool, int, int);
	void draw();
	void showmoves();
	bool isinrange(POINT);
};

class Bishop : public Figure {
public:
	static HBITMAP hbmBishop;
	static HBITMAP hbmMask;
	Bishop(bool, int, int);
	void draw();
	void showmoves();
	bool isinrange(POINT);
};

class Queen : public Figure {
public:
	static HBITMAP hbmQueen;
	static HBITMAP hbmMask;
	Queen(bool, int, int);
	void draw();
	void showmoves();
	bool isinrange(POINT);
};

class King : public Figure {
public:
	static HBITMAP hbmKing;
	static HBITMAP hbmMask;
	bool bWasMoved;
	King(bool, int, int);
	void draw();
	void showmoves();
	bool isinrange(POINT);
	void move(POINT);
};