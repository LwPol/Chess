#include "figures.h"
#include "stdafx.h"

//
//	FUNCTION: Absolute(int)
//
//	PURPOSE: Function returns absolute value of an integer
//
int Absolute(int x) {
	return x >= 0 ? x : -x;
}


//			PlayerNames class			//

//
//	FUNCTION: PlayerNames()
//
//	PURPOSE: default class constructor
//
//	COMMENT:
//		Initializes pointers to null
//
PlayerNames::PlayerNames():
	szFirst(nullptr)
	, szSecond(nullptr) {}

//
//	FUNCTION: ~PlayerNames()
//
//	PURPOSE: destructor
//
//	COMMENT:
//		Frees previously allocated memory
//
PlayerNames::~PlayerNames() {
	if (szFirst)
		free(szFirst);
	if (szSecond)
		free(szSecond);
}

//	FUNCTION: SetName(int, LPCWSTR, int)
//
//	PURPOSE: Coppies text to object's appropriate variable
//
//	COMMENT:
//		Function coppies text of szName paramter to one of class buffers
//		The buffer is chosen based on nIndex parameter
//		String from szName must be null-terminated
//		The nLenght parameter specifies lenght of szName text including null character
//		Buffer size is dependent on nLenght parameter
//
bool PlayerNames::SetName(int nIndex, LPCWSTR szName, int nLenght) {
	switch (nIndex) {
	case 0:
		if (szFirst)
			free(szFirst);
		szFirst = (WCHAR*)calloc(nLenght, sizeof(WCHAR));
		if (!szFirst)
			return false;
		lstrcpy(szFirst, szName);
		break;
	case 1:
		if (szSecond)
			free(szSecond);
		szSecond = (WCHAR*)calloc(nLenght, sizeof(WCHAR));
		if (!szSecond)
			return false;
		lstrcpy(szSecond, szName);
		break;
	default:
		return false;
	}
	return true;
}


//			Figure class			//

//
//	FUNCTION: Figure()
//
//	PURPOSE: constructor
//
//	COMMENT:
//		bWhite parameter specifies if created figure should be white or black
//		x and y parameter specify starting position for the figure
//
Figure::Figure(bool bWhite, int x, int y) :
	bIsWhite(bWhite)
	, ptPos({ x, y })
{
	// associates object with the control
	SetWindowLongPtr(GameData::hFieldsTable[x][y], sizeof(LONG_PTR), (LONG_PTR)this);
}

//
//	FUNCTION: ~Figure()
//
//	PURPOSE: destructor
//
Figure::~Figure() {
	Figure *figInObjectsPos = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y], sizeof(LONG_PTR));

	if (figInObjectsPos == this)
		SetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y], sizeof(LONG_PTR), (LONG_PTR)nullptr);

	Figure **figuresColor;
	std::list<Figure*> *figuresPromoted;
	if (bIsWhite) {
		figuresColor = GameData::whiteFigures;
		figuresPromoted = &GameData::whitePromoted;
	}
	else {
		figuresColor = GameData::blackFigures;
		figuresPromoted = &GameData::blackPromoted;
	}

	int i;
	for (i = 0; i < 16; ++i) {
		if (figuresColor[i] == this) {
			figuresColor[i] = nullptr;
			break;
		}
	}

	if (i < 8) {
		if (!dynamic_cast<Pawn*>(this) && !dynamic_cast<Knight*>(this)) {
			std::list<Figure*>::iterator it;
			for (it = figuresPromoted->begin(); it != figuresPromoted->end(); ++it) {
				if (*it == this) {
					figuresPromoted->erase(it);
					break;
				}
			}
		}
	}
}

//	
//	FUNCTION: draw(HBITMAP, HBITMAP)
//
//	PURPOSE: Function draws the figure with specified bitmaps
//
void Figure::draw(HBITMAP hbmFigure, HBITMAP hbmMask) {
	// gets handles to the control and its DC where drawing should be performed
	HWND hWnd = GameData::hFieldsTable[ptPos.x][ptPos.y];
	HDC hdc = GetDC(hWnd);

	// gets size of the control and figure's bitmap
	RECT rc;
	GetClientRect(hWnd, &rc);
	BITMAP bm;
	GetObject(hbmFigure, sizeof(bm), &bm);

	HDC hdcMem = CreateCompatibleDC(nullptr);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMask);

	// blits figure's mask on the control
	StretchBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, bm.bmWidth / 2, bm.bmHeight, SRCAND);

	// determines which part of the bitmap should be painted
	// it depends on the figure's color
	int nX;
	if (bIsWhite)
		nX = 0;
	else
		nX = bm.bmWidth / 2;

	// blits figure's bitmap on the control
	SelectObject(hdcMem, hbmFigure);
	StretchBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, nX, 0, bm.bmWidth / 2, bm.bmHeight, SRCPAINT);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
	ReleaseDC(hWnd, hdc);
}

//
//	FUNCTION: move(POINT)
//
//	PURPOSE: moves figure to the specified spot on the chessboard
//
//	NOTE: this method is virtual
//
void Figure::move(POINT ptTo) {
	// gets figure object that is on the spot before the move
	Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[ptTo.x][ptTo.y], sizeof(LONG_PTR));

	// if there is any figure it should be deleted
	if (fig)
		delete fig;

	// sets object's new position and redraws the figure
	ptPos = ptTo;
	draw();

	// associates the object to the new spot
	SetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y], sizeof(LONG_PTR), (LONG_PTR)this);
	
	// if there's no possibility for en passant in next move then zero the global variable
	if (GameData::enPassant != this)
		GameData::enPassant = nullptr;
}


//			Pawn class			//

//
//	FUNCTION: Pawn(bool, int, int)
//
//	PURPOSE: constructor
//
Pawn::Pawn(bool bWhite, int x, int y):
	Figure(bWhite, x, y) {}

//
//	FUNCTION: draw()
//
//	PURPOSE: draws pawn
//
void Pawn::draw() {
	Figure::draw(hbmPawn, hbmMask);
}

//
//	FUCTION: showmoves()
//
//	PURPOSE: examines possible moves for the object
//
void Pawn::showmoves() {
	// checks if pawn's color
	switch (bIsWhite) {
	case true: {
		Figure *front;
		int i = 2;

		// loop where we check if a diagonal move is possible
		do {
			--i;
			int nx = ptPos.x + i;

			// if coordinates don't match any of the squares
			if (nx < 0 || nx > 7) {
				--i;
				continue;
			}

			// gets the object on the examined square
			front = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ptPos.y - 1], sizeof(LONG_PTR));

			// if there is an enemy figure it's possible to move
			if (front && front->bIsWhite != bIsWhite)
				SendMessage(GameData::hFieldsTable[nx][ptPos.y - 1], CM_ALLOWSELECT, TRUE, 0);

			--i;
		} while (i != -2);

		// checks if one square forward is possible
		front = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y - 1], sizeof(LONG_PTR));
		if (!front)
			SendMessage(GameData::hFieldsTable[ptPos.x][ptPos.y - 1], CM_ALLOWSELECT, TRUE, 0);
		else
			break;

		// checks if pawn can move two squares forward
		if (ptPos.y == 6) {
			front = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y - 2], sizeof(LONG_PTR));
			if (!front)
				SendMessage(GameData::hFieldsTable[ptPos.x][ptPos.y - 2], CM_ALLOWSELECT, TRUE, 0);
		}
		break;
		}
	case false: {
		// the same as above, just for a black pawn
		Figure *front;
		int i = 2;
		do {
			--i;
			int nx = ptPos.x + i;
			if (nx < 0 || nx > 7) {
				--i;
				continue;
			}
			front = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ptPos.y + 1], sizeof(LONG_PTR));
			if (front && front->bIsWhite != bIsWhite)
				SendMessage(GameData::hFieldsTable[nx][ptPos.y + 1], CM_ALLOWSELECT, TRUE, 0);
			--i;
		} while (i != -2);

		front = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y + 1], sizeof(LONG_PTR));
		if (!front)
			SendMessage(GameData::hFieldsTable[ptPos.x][ptPos.y + 1], CM_ALLOWSELECT, TRUE, 0);
		else
			break;
		if (ptPos.y == 1) {
			front = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[ptPos.x][ptPos.y + 2], sizeof(LONG_PTR));
			if (!front)
				SendMessage(GameData::hFieldsTable[ptPos.x][ptPos.y + 2], CM_ALLOWSELECT, TRUE, 0);
		}
		}
	}

	// checks if en passant is possible
	// possibility of en passant is indicated by a non-zero global pointer
	if (GameData::enPassant) {
		// checks if 'en passant' pawn is in the same row as the selected one
		if (GameData::enPassant->ptPos.y == ptPos.y) {
			// checks if 'en passant' pawn is right next to the selected pawn
			if (GameData::enPassant->ptPos.x == ptPos.x - 1 || GameData::enPassant->ptPos.x == ptPos.x + 1) {
				// in this stage en passant is possible to perform
				// gets handle to a control that the object can move to
				HWND hEnPassant;
				if (GameData::enPassant->bIsWhite)
					hEnPassant = GameData::hFieldsTable[GameData::enPassant->ptPos.x][GameData::enPassant->ptPos.y + 1];
				else
					hEnPassant = GameData::hFieldsTable[GameData::enPassant->ptPos.x][GameData::enPassant->ptPos.y - 1];

				SendMessage(hEnPassant, CM_ALLOWSELECT, TRUE, ST_ENPASSANT);
			}
		}
	}
}

//
//	FUNCTION: isinrange(POINT)
//
//	PURPOSE: examines if a particular spot can be captured by the object
//
bool Pawn::isinrange(POINT pt) {
	// checks pawn's color
	switch (bIsWhite) {
	case true:
		// if pawn is in the end of the chessboard returns false
		if (ptPos.y == 0)
			return false;

		// if the spot isn't one row ahead returns false
		if (pt.y != ptPos.y - 1)
			return false;
		break;
	case false:
		// the same as above, just for a black pawn
		if (ptPos.y == 7)
			return false;
		if (pt.y != ptPos.y + 1)
			return false;
	}

	// checks if the spot is in a column right next to the object's column
	if (pt.x == ptPos.x + 1 || pt.x == ptPos.x - 1)
		return true;
	return false;
}

//
//	FUNCTION: move(POINT)
//
//	PURPOSE: moves the object to a particular point on the chessboard
//
void Pawn::move(POINT ptTo) {
	// if the pawn moves two steps ahead it might be captured by 'en passant'
	// so the program sets a global pointer to the pawn
	if (Absolute(ptTo.y - ptPos.y) == 2)
		GameData::enPassant = this;

	// checks if the move is 'en passant'
	CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hFieldsTable[ptTo.x][ptTo.y], 2 * sizeof(LONG_PTR));
	if (pcfd->dwState == ST_ENPASSANT) {
		// captures the 'en passant' pawn
		HWND hEnPassant = GameData::hFieldsTable[GameData::enPassant->ptPos.x][GameData::enPassant->ptPos.y];
		delete GameData::enPassant;

		// redraws captured pawn's square
		RECT rcEnPassant;
		GetClientRect(hEnPassant, &rcEnPassant);
		POINT ptLeft = { 0, 0 };
		POINT ptRight = { rcEnPassant.right, rcEnPassant.bottom };
		ClientToScreen(hEnPassant, &ptLeft);
		ClientToScreen(hEnPassant, &ptRight);
		ScreenToClient(GetParent(hEnPassant), &ptLeft);
		ScreenToClient(GetParent(hEnPassant), &ptRight);
		SetRect(&rcEnPassant, ptLeft.x, ptLeft.y, ptRight.x, ptRight.y);
		RedrawWindow(GetParent(hEnPassant), &rcEnPassant, nullptr, RDW_INVALIDATE);
	}

	// moves figure to the spot
	Figure::move(ptTo);

	if (ptTo.y == 0 || ptTo.y == 7)
		PostMessage(GameData::hFieldsTable[ptTo.x][ptTo.y], CM_PAWNPROMOTION, 0, 0);
}

//
//	FUNCTION: promote(int)
//
//	PURPOSE: promotes pawn to another figure
//
Figure* Pawn::promote(int nNewFigure) {
	Figure *newFig = nullptr;	// this pointer will receive address of newly allocated object

	// creates new object based on the value of an integer
	switch (nNewFigure) {
	case PROMOTION_ROOK:
		newFig = new Rook(bIsWhite, ptPos.x, ptPos.y);
		break;
	case PROMOTION_KNIGHT:
		newFig = new Knight(bIsWhite, ptPos.x, ptPos.y);
		break;
	case PROMOTION_BISHOP:
		newFig = new Bishop(bIsWhite, ptPos.x, ptPos.y);
		break;
	case PROMOTION_QUEEN:
		newFig = new Queen(bIsWhite, ptPos.x, ptPos.y);
	}

	if (!dynamic_cast<Knight*>(newFig)) {
		if (bIsWhite)
			GameData::whitePromoted.push_back(newFig);
		else
			GameData::blackPromoted.push_back(newFig);
	}

	// checks figure's color to set new object's pointer into figures' table and
	// sets a global variable
	Figure **blackorwhite;
	if (bIsWhite) {
		blackorwhite = GameData::whiteFigures;
		
	}
	else {
		blackorwhite = GameData::blackFigures;
	}
	for (int i = 0; i < 16; ++i) {
		if (blackorwhite[i] == this) {
			blackorwhite[i] = newFig;
			break;
		}
	}

	// returns a pointer to the newly created object
	return newFig;
}

// Rook
Rook::Rook(bool bWhite, int x, int y):
	Figure(bWhite, x, y)
	, bWasMoved(false) {}

void Rook::draw() {
	Figure::draw(hbmRook, hbmMask);
}

void Rook::showmoves() {
	Figure *fig;
	for (int i = 0; i < 4; ++i) {
		int nx = ptPos.x;
		int ny = ptPos.y;
		while (1) {
			switch (i) {
			case 0:
				++nx;
				break;
			case 1:
				++ny;
				break;
			case 2:
				--nx;
				break;
			case 3:
				--ny;
			}
			if (nx < 0 || nx > 7 || ny < 0 || ny > 7)
				break;
			fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ny], sizeof(LONG_PTR));
			if (!fig)
				SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
			else {
				if (fig->bIsWhite != bIsWhite)
					SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
				break;
			}
		}
	}
}

bool Rook::isinrange(POINT pt) {
	if (pt.x == ptPos.x) {
		int i;
		if (ptPos.y > pt.y)
			i = 1;
		else
			i = -1;
		for (pt.y += i; pt.y != ptPos.y; pt.y += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	if (pt.y == ptPos.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i; pt.x != ptPos.x; pt.x += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	return false;
}

void Rook::move(POINT ptTo) {
	Figure::move(ptTo);
	if (!bWasMoved)
		bWasMoved = true;
}

// Knight
Knight::Knight(bool bWhite, int x, int y):
	Figure(bWhite, x, y) {}

void Knight::draw() {
	Figure::draw(hbmKnight, hbmMask);
}

void Knight::showmoves() {
	Figure *fig;
	int nx, ny;
	for (int i = 0; i < 8; ++i) {
		switch (i) {
		case 0:
			nx = ptPos.x + 2;
			ny = ptPos.y + 1;
			break;
		case 1:
			--nx;
			++ny;
			break;
		case 2:
			nx -= 2;
			break;
		case 3:
			--nx;
			--ny;
			break;
		case 4:
			ny -= 2;
			break;
		case 5:
			++nx;
			--ny;
			break;
		case 6:
			nx += 2;
			break;
		case 7:
			++nx;
			++ny;
		}
		if (nx < 0 || nx > 7 || ny < 0 || ny > 7)
			continue;
		fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ny], sizeof(LONG_PTR));
		if (!fig || fig->bIsWhite != bIsWhite)
			SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
	}
}

bool Knight::isinrange(POINT pt) {
	if (pt.x == ptPos.x || pt.y == ptPos.y)
		return false;
	int nDist = Absolute(ptPos.x - pt.x) + Absolute(ptPos.y - pt.y);
	if (nDist == 3)
		return true;
	return false;
}

// Bishop
Bishop::Bishop(bool bWhite, int x, int y):
	Figure(bWhite, x, y) {}

void Bishop::draw() {
	Figure::draw(hbmBishop, hbmMask);
}

void Bishop::showmoves() {
	Figure *fig;
	for (int i = 0; i < 4; ++i) {
		int nx = ptPos.x;
		int ny = ptPos.y;
		while (1) {
			switch (i) {
			case 0:
				++nx;
				++ny;
				break;
			case 1:
				--nx;
				++ny;
				break;
			case 2:
				--nx;
				--ny;
				break;
			case 3:
				++nx;
				--ny;
			}
			if (nx < 0 || nx > 7 || ny < 0 || ny > 7)
				break;
			fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ny], sizeof(LONG_PTR));
			if (!fig)
				SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
			else {
				if (fig->bIsWhite != bIsWhite)
					SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
				break;
			}
		}
	}
}

bool Bishop::isinrange(POINT pt) {
	if (ptPos.x - pt.x == ptPos.y - pt.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i, pt.y += i; pt.x != ptPos.x; pt.x += i, pt.y += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	if (ptPos.x - pt.x == pt.y - ptPos.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i, pt.y -= i; pt.x != ptPos.x; pt.x += i, pt.y -= i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	return false;
}

// Queen
Queen::Queen(bool bWhite, int x, int y):
	Figure(bWhite, x, y) {}

void Queen::draw() {
	Figure::draw(hbmQueen, hbmMask);
}

void Queen::showmoves() {
	Figure *fig;
	for (int i = 0; i < 8; ++i) {
		int nx = ptPos.x;
		int ny = ptPos.y;
		while (1) {
			switch (i) {
			case 0:
				++nx;
				break;
			case 1:
				++ny;
				break;
			case 2:
				--nx;
				break;
			case 3:
				--ny;
				break;
			case 4:
				++nx;
				++ny;
				break;
			case 5:
				--nx;
				++ny;
				break;
			case 6:
				--nx;
				--ny;
				break;
			case 7:
				++nx;
				--ny;
			}
			if (nx < 0 || nx > 7 || ny < 0 || ny > 7)
				break;
			fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ny], sizeof(LONG_PTR));
			if (!fig)
				SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
			else {
				if (fig->bIsWhite != bIsWhite)
					SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
				break;
			}
		}
	}
}

bool Queen::isinrange(POINT pt) {
	if (pt.x == ptPos.x) {
		int i;
		if (ptPos.y > pt.y)
			i = 1;
		else
			i = -1;
		for (pt.y += i; pt.y != ptPos.y; pt.y += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	if (pt.y == ptPos.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i; pt.x != ptPos.x; pt.x += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	if (ptPos.x - pt.x == ptPos.y - pt.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i, pt.y += i; pt.x != ptPos.x; pt.x += i, pt.y += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	if (ptPos.x - pt.x == ptPos.y - pt.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i, pt.y += i; pt.x != ptPos.x; pt.x += i, pt.y += i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	if (ptPos.x - pt.x == pt.y - ptPos.y) {
		int i;
		if (ptPos.x > pt.x)
			i = 1;
		else
			i = -1;
		for (pt.x += i, pt.y -= i; pt.x != ptPos.x; pt.x += i, pt.y -= i) {
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[pt.x][pt.y], sizeof(LONG_PTR));
			if (fig)
				return false;
		}
		return true;
	}
	return false;
}

// King
King::King(bool bWhite, int x, int y):
	Figure(bWhite, x, y)
	, bWasMoved(false) {}

void King::showmoves() {
	Figure **figuresToCheck;
	if (bIsWhite)
		figuresToCheck = GameData::blackFigures;
	else
		figuresToCheck = GameData::whiteFigures;

	int nx, ny;
	for (int i = 0; i < 8; ++i) {
		switch (i) {
		case 0:
			nx = ptPos.x - 1;
			ny = ptPos.y - 1;
			break;
		case 1:
		case 2:
			++nx;
			break;
		case 3:
		case 4:
			++ny;
			break;
		case 5:
		case 6:
			--nx;
			break;
		case 7:
			--ny;
		}
		if (nx < 0 || nx > 7 || ny < 0 || ny > 7)
			continue;
		Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[nx][ny], sizeof(LONG_PTR));
		if (fig && fig->bIsWhite == bIsWhite)
			continue;
		int j;
		POINT pt = { nx, ny };
		CHESSFIELDDATA *pcfd = nullptr;
		if (GameData::hShahing) {
			pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hShahing, 2 * sizeof(LONG_PTR));
			fig = (Figure*)GetWindowLongPtr(GameData::hShahing, sizeof(LONG_PTR));
		}
		for (j = 0; j < 16; ++j) {
			if (pcfd && pcfd->ptPos.x == nx && pcfd->ptPos.y == ny) {
				if (figuresToCheck[j] == fig)
					continue;
			}
			if (figuresToCheck[j] && figuresToCheck[j]->isinrange(pt))
				break;
		}
		if (j == 16) {
			SendMessage(GameData::hFieldsTable[nx][ny], CM_ALLOWSELECT, TRUE, 0);
		}
	}

	if (!bWasMoved && !(GameData::flags & CHESS_CHECK)) {
		Rook *castling = nullptr;
		for (int i = 10; i < 12; ++i) {
			switch (bIsWhite) {
			case true:
				castling = (Rook*)GameData::whiteFigures[i];
				break;
			case false:
				castling = (Rook*)GameData::blackFigures[i];
			}
			if (!castling || castling->bWasMoved)
				continue;
			if (castling->bIsWhite) {
				if (castling->ptPos.y != 7)
					continue;
			}
			else
				if (castling->ptPos.y != 0)
					continue;
			int dx;
			if (ptPos.x > castling->ptPos.x)
				dx = -1;
			else
				dx = 1;
			int j;
			for (j = ptPos.x + dx; j != castling->ptPos.x; j += dx) {
				Figure *fig = (Figure*)GetWindowLongPtr(GameData::hFieldsTable[j][ptPos.y], sizeof(LONG_PTR));
				if (fig)
					break;
			}
			if (j != castling->ptPos.x)
				continue;
			POINT ptFlow = { ptPos.x + dx, ptPos.y };
			for (j = 0; j < 16; ++j) {
				if (figuresToCheck[j] && figuresToCheck[j]->isinrange(ptFlow))
					break;
			}
			if (j == 16)
				SendMessage(GameData::hFieldsTable[ptPos.x + 2 * dx][ptPos.y], CM_ALLOWSELECT, TRUE, 0);
		}
	}
}

bool King::isinrange(POINT pt) {
	if (Absolute(ptPos.x - pt.x) == 1 || Absolute(ptPos.y - pt.y) == 1)
		if (Absolute(ptPos.x - pt.x) + Absolute(ptPos.y - pt.y) <= 2)
			return true;
	return false;
}

void King::draw() {
	Figure::draw(hbmKing, hbmMask);
}

void King::move(POINT ptTo) {
	Figure **Rooks;
	if (bIsWhite)
		Rooks = GameData::whiteFigures;
	else
		Rooks = GameData::blackFigures;

	HWND hPrevRook = nullptr;
	POINT ptRook = { 0, ptPos.y };
	if (ptTo.x - ptPos.x == 2) {
		hPrevRook = GameData::hFieldsTable[Rooks[11]->ptPos.x][Rooks[11]->ptPos.y];
		SetWindowLongPtr(hPrevRook, sizeof(LONG_PTR), (LONG_PTR)nullptr);
		ptRook.x = ptTo.x - 1;
		Rooks[11]->move(ptRook);
	}
	else if (ptTo.x - ptPos.x == -2) {
		hPrevRook = GameData::hFieldsTable[Rooks[10]->ptPos.x][Rooks[10]->ptPos.y];
		SetWindowLongPtr(hPrevRook, sizeof(LONG_PTR), (LONG_PTR)nullptr);
		ptRook.x = ptTo.x + 1;
		Rooks[10]->move(ptRook);
	}

	RECT rcPrevRook;
	GetClientRect(hPrevRook, &rcPrevRook);
	POINT ptLeft = { 0, 0 };
	POINT ptRight = { rcPrevRook.right, rcPrevRook.bottom };
	ClientToScreen(hPrevRook, &ptLeft);
	ClientToScreen(hPrevRook, &ptRight);
	ScreenToClient(GetParent(hPrevRook), &ptLeft);
	ScreenToClient(GetParent(hPrevRook), &ptRight);
	SetRect(&rcPrevRook, ptLeft.x, ptLeft.y, ptRight.x, ptRight.y);
	RedrawWindow(GetParent(hPrevRook), &rcPrevRook, nullptr, RDW_INVALIDATE);

	Figure::move(ptTo);
	if (!bWasMoved)
		bWasMoved = true;
}
