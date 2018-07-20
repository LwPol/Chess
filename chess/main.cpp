// Chess.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "chess.h"

#define MAX_LOADSTRING 100

// static variables initiation
PlayerNames GameData::pnNames;
BYTE GameData::flags = CHESS_WHITESTURN;
HBITMAP GameData::hbmChessboard = nullptr;
HBITMAP Pawn::hbmPawn		= nullptr;
HBITMAP Pawn::hbmMask		= nullptr;
HBITMAP Rook::hbmRook		= nullptr;
HBITMAP Rook::hbmMask		= nullptr;
HBITMAP Knight::hbmKnight	= nullptr;
HBITMAP Knight::hbmMask		= nullptr;
HBITMAP Bishop::hbmBishop	= nullptr;
HBITMAP Bishop::hbmMask		= nullptr;
HBITMAP Queen::hbmQueen		= nullptr;
HBITMAP Queen::hbmMask		= nullptr;
HBITMAP King::hbmKing		= nullptr;
HBITMAP King::hbmMask		= nullptr;
HWND** GameData::hFieldsTable = nullptr;
HWND GameData::hSelected = nullptr;
std::stack<HWND> GameData::PossibleToMove;
Figure** GameData::whiteFigures = new Figure*[16]();
Figure** GameData::blackFigures = new Figure*[16]();
HWND GameData::hShahing = nullptr;
Figure* GameData::enPassant = nullptr;
std::list<Figure*> GameData::whitePromoted;
std::list<Figure*> GameData::blackPromoted;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WNDPROC g_StaticProc;							// pointer to default static window class procedure

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	NamesProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	WhoBeginsProc(HWND, UINT, WPARAM, LPARAM);
HBITMAP				CreateBitmapMask(HBITMAP, COLORREF);
ATOM				RegisterChessFields(HINSTANCE);
LRESULT CALLBACK	ChessFieldProc(HWND, UINT, WPARAM, LPARAM);
BOOL				InitChessFields(HWND, HBITMAP);
BOOL				LoadClassBitmaps();
void				DeleteClassBitmaps();
BOOL				InitFigures();
BOOL				CheckShah(Figure*);
bool				TestIfMoveIsValid(HWND);
BOOL				IsMat();
LRESULT CALLBACK	MatCheckingProc(HWND, UINT, WPARAM, LPARAM);
BOOL				InitNamesTable(HWND);
LRESULT CALLBACK	NamesTableProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	PromotionProc(HWND, UINT, WPARAM, LPARAM);
void				DeleteAllExistingFigures();
BOOL				ResetGame();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

    // TODO: Place code here.

    // Initialize global strings
	lstrcpyW(szTitle, L"Chess");
	lstrcpyW(szWindowClass, L"MainWindow");
    MyRegisterClass(hInstance);
	RegisterChessFields(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CROWN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDM_MENU);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CROWN));

    return RegisterClassExW(&wcex);
}

//
//	FUNCTION: RegisterChessFields(HINSTANCE)
//
//	PURPOSE: Registers ChessFields window class.
//
//	COMMENTS:
//		ChessField window class is an interactive area that may contain chess figures
//
ATOM RegisterChessFields(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc	= ChessFieldProc;
	wcex.cbWndExtra		= 3 * sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszClassName	= L"ChessField";

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_SHOWMAXIMIZED);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	// initialize game data
	case WM_CREATE: {
		INT_PTR res = 0;
		// loop where we get players' names and let users choose who begins
		do {
			// retrieving names dialog box
			res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_START), hWnd, NamesProc, res);
			if (res == IDCANCEL)
				break;
			// turn priority dialog box
			res = DialogBox(hInst, MAKEINTRESOURCE(IDD_WHOBEGINS), hWnd, WhoBeginsProc);
		} while (res == IDC_BACK); // until we get necessary information
		if (res == IDCANCEL) {
			// if user choosed clicked cancel button in any of the dialog boxes
			DestroyWindow(hWnd);
			return 0;
		}
		
		// loading chessboard bitmap
		GameData::hbmChessboard = (HBITMAP)LoadImage(nullptr,
			L"chess_graphics\\chessboard.bmp",
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		if (!GameData::hbmChessboard) {
			MessageBox(hWnd, L"B³¹d przy ³adowaniu bitmapy t³a", nullptr, MB_ICONEXCLAMATION);
			DestroyWindow(hWnd);
			return 0;
		}

		// loading bitmaps for figures
		if (!LoadClassBitmaps()) {
			// if program failed to load any bitmap
			DeleteClassBitmaps();
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}

		// initializes ChessField controls
		// Note that main window should be maximized cause chessfields initialization is based
		// on the main window's size
		ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		if (!InitChessFields(hWnd, GameData::hbmChessboard)) {
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}

		// creates static control containing names of players
		if (!InitNamesTable(hWnd))
			DestroyWindow(hWnd);

		// creates chess figures
		if (!InitFigures())
			DestroyWindow(hWnd);
		break;
	}
	// if any figure is selected WM_RBUTTONUP cancels the selection
	// the message is sent to a ChessField control where it's properly processed
	case WM_RBUTTONUP:
		SendMessage(GameData::hFieldsTable[0][0], WM_RBUTTONUP, 0, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		// restarts the game
		case IDM_OPCJE_NEWGAME: {
			if (!ResetGame())
				DestroyWindow(hWnd);
			HWND hNewGameBtn = GetDlgItem(hWnd, IDM_OPCJE_NEWGAME);
			if (hNewGameBtn)
				DestroyWindow(hNewGameBtn);
			break;
			}
		case IDM_OPCJE_CHANGENAME: {
			INT_PTR ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_START), hWnd, NamesProc, TRUE);
			if (ret == IDOK)
				RedrawWindow(GetDlgItem(hWnd, ID_NAMESTABLE), nullptr, nullptr, RDW_INVALIDATE);
			break;
			}
		case IDM_OPCJE_CHENAGETURN:
			if (MessageBox(hWnd, L"Ta operacja spowoduje restart gry. Czy na pewno chcesz kontynuowaæ?",
				L"Zmiana kolejnoœci", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				if (GameData::flags & CHESS_FIRSTSTART)
					GameData::flags &= ~CHESS_FIRSTSTART;
				else
					GameData::flags |= CHESS_FIRSTSTART;

				if (!ResetGame())
					DestroyWindow(hWnd);
				HWND hNewGameBtn = GetDlgItem(hWnd, IDM_OPCJE_NEWGAME);
				if (hNewGameBtn)
					DestroyWindow(hNewGameBtn);
			}
		}
		break;
	// repaints the chessboard
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, GameData::hbmChessboard);

			// retrieves main window's and chessboard bitmap's size
			BITMAP bmInfo;
			GetObject(GameData::hbmChessboard, sizeof(BITMAP), &bmInfo);
			RECT rcWnd;
			GetClientRect(hWnd, &rcWnd);

			// blits chessboard bitmap into the main window
			BitBlt(hdc, (rcWnd.right - bmInfo.bmWidth) / 2, (rcWnd.bottom - bmInfo.bmHeight) / 2,
				bmInfo.bmWidth, bmInfo.bmHeight, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, hbmOld);
			DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
        }
        break;
	// frees all the resources and posts quit message for the application
    case WM_DESTROY:
		for (int i = 0; i < 16; ++i) {
			delete GameData::whiteFigures[i];
			delete GameData::blackFigures[i];
		}
		delete[] GameData::whiteFigures;
		delete[] GameData::blackFigures;

		if (GameData::hFieldsTable) {
			for (int i = 0; i < 8; ++i) {
				delete[] GameData::hFieldsTable[i];
			}
			delete[] GameData::hFieldsTable;
		}

		DeleteObject(GameData::hbmChessboard);
		DeleteClassBitmaps();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//
//	FUNCTION: NamesProc(HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE: Processes messages for the dialog box retrieving players' names
//
INT_PTR CALLBACK NamesProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		// limit text for edit controls to 25 characters
		SendMessage(GetDlgItem(hDlg, IDC_FIRSTPLAYER), EM_LIMITTEXT, 25, 0);
		SendMessage(GetDlgItem(hDlg, IDC_SECONDPLAYER), EM_LIMITTEXT, 25, 0);

		// lParam is non-zero if the dialog box was called from the other dialog
		// that retrieves turn priority
		// in this case application sets text of edit controls to the one that was
		// written in previous call
		if (lParam) {
			SetWindowText(GetDlgItem(hDlg, IDC_FIRSTPLAYER), (LPCWSTR)GameData::pnNames.szFirst);
			SetWindowText(GetDlgItem(hDlg, IDC_SECONDPLAYER), (LPCWSTR)GameData::pnNames.szSecond);
		}
		SetFocus(GetDlgItem(hDlg, IDC_FIRSTPLAYER));
		return FALSE;
	case WM_CTLCOLORSTATIC: {
		HWND hCtrl = (HWND)lParam;
		HDC hdcCtrl = (HDC)wParam;
		if (hCtrl == GetDlgItem(hDlg, IDC_INFORMATION)) {
			// this text appers when user clicks OK button but at least one
			// of the edit controls are empty
			// The text of the information is displayed in red
			SetTextColor(hdcCtrl, RGB(255, 0, 0));
		}
		SetBkMode(hdcCtrl, TRANSPARENT);
		return (LRESULT)GetStockObject(NULL_BRUSH);
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		case IDOK: {
			// calculate lenght of strings in both edit controls
			int nFirstLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_FIRSTPLAYER));
			int nSecondLength = GetWindowTextLength(GetDlgItem(hDlg, IDC_SECONDPLAYER));

			// if any of the edit controls are empty shows appropriate information to fulfill it
			if (!nFirstLength || !nSecondLength)
				ShowWindow(GetDlgItem(hDlg, IDC_INFORMATION), SW_SHOW);
			else {
				HGLOBAL hMem;
				LPWSTR buf;
				// allocates memory for buffer to retrieve text from edit controls
				// Note that there is only one buffer so it must be big enought for both texts
				if (nFirstLength > nSecondLength)
					hMem = GlobalAlloc(GPTR, 2 * nFirstLength + 2);
				else
					hMem = GlobalAlloc(GPTR, 2 * nSecondLength + 2);
				buf = (LPWSTR)GlobalLock(hMem);

				// copies text from edit controls to buffer and then to global variables
				GetWindowText(GetDlgItem(hDlg, IDC_FIRSTPLAYER), buf, nFirstLength + 1);
				GameData::pnNames.SetName(0, (LPCWSTR)buf, nFirstLength + 1);
				GetWindowText(GetDlgItem(hDlg, IDC_SECONDPLAYER), buf, nSecondLength + 1);
				GameData::pnNames.SetName(1, (LPCWSTR)buf, nSecondLength + 1);

				// deallocates memory of the buffer
				GlobalUnlock(hMem);
				GlobalFree(hMem);
				
				EndDialog(hDlg, IDOK);
			}
			break;
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//
//	FUNCTION: WhoBeginsProc(HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE: Processes messages for the dialog box which retrives information
//		about players' order
//
INT_PTR CALLBACK WhoBeginsProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		// sets buttons' text to appropriate players' names retrieved in the previous dialog
		SetWindowText(GetDlgItem(hDlg, IDC_RADIOFIRST), (LPCWSTR)GameData::pnNames.szFirst);
		SetWindowText(GetDlgItem(hDlg, IDC_RADIOSECOND), (LPCWSTR)GameData::pnNames.szSecond);
		break;
	case WM_CTLCOLORSTATIC: {
		HWND hCtrl = (HWND)lParam;
		HDC hdcCtrl = (HDC)wParam;
		if (hCtrl == GetDlgItem(hDlg, IDC_INFORMATION))
			// this text appears when user clicks OK button but neither of the buttons is selected
			// The text of the information is displayed in red
			SetTextColor(hdcCtrl, 0x0000FF);
		SetBkMode(hdcCtrl, TRANSPARENT);
		return (LRESULT)GetStockObject(NULL_BRUSH);
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			// determines which button is checked
			if (IsDlgButtonChecked(hDlg, IDC_RADIOFIRST) == BST_CHECKED)
				GameData::flags |= CHESS_FIRSTSTART;
			else if (IsDlgButtonChecked(hDlg, IDC_RADIOSECOND) == BST_CHECKED);

			// if no button is checked shows appropriate information
			else {
				ShowWindow(GetDlgItem(hDlg, IDC_INFORMATION), SW_SHOW);
				break;
			}
			// lack of 'break' here is intentional
		case IDCANCEL:
		case IDC_BACK:
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//
//	FUNCTION: CreateBitmapMask(HBITMAP, COLORREF)
//
//	PURPOSE: Function creates mask for a particular bitmap.
//		crTransparent parameter specifies color on the bitmap that should be transparent
//
HBITMAP CreateBitmapMask(HBITMAP hbmColor, COLORREF crTransparent) {
	HDC hdcMem, hdcMem2;
	hdcMem = CreateCompatibleDC(NULL);
	hdcMem2 = CreateCompatibleDC(NULL);

	// retrieves bitmap's size
	BITMAP bmInfo;
	GetObject(hbmColor, sizeof(BITMAP), &bmInfo);

	HBITMAP hbmMask, hbmOld, hbmOld2;
	hbmMask = CreateBitmap(bmInfo.bmWidth, bmInfo.bmHeight, 1, 1, NULL);

	// selects bitmaps to dcs and set background color in hdcMem2
	// to the one specified in function's parameter
	hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMask);
	hbmOld2 = (HBITMAP)SelectObject(hdcMem2, hbmColor);
	SetBkColor(hdcMem2, crTransparent);

	BitBlt(hdcMem, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hdcMem2, 0, 0, SRCCOPY);
	BitBlt(hdcMem2, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hdcMem, 0, 0, SRCINVERT);

	SelectObject(hdcMem, hbmOld);
	SelectObject(hdcMem2, hbmOld2);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);
	return hbmMask;
}

//
//	FUNCTION: ChessFieldProc(HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE: Processes messages for ChessField controls
//
LRESULT CALLBACK ChessFieldProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		// associate CHESSFIELDDATA structure from lParam parameter to the window
		CREATESTRUCT *pcs = (LPCREATESTRUCT)lParam;
		SetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR), (LONG_PTR)pcs->lpCreateParams);
		break;
		}
	case WM_PAINT: {
		// painting operation depends on the state of the control
		// note that for default state no painting operation occurs
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// retrieve control's size
		RECT rcWnd;
		GetClientRect(hWnd, &rcWnd);

		// get control associated data
		Figure *fig = (Figure*)GetWindowLongPtr(hWnd, sizeof(LONG_PTR));
		CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));

		// brush to draw control's background
		// its style depends on the state of the control
		HBRUSH hbrBackground = nullptr;

		switch (pcfd->dwState) {
		case ST_SELECTED:
			// selected controls have yellow background
			hbrBackground = CreateSolidBrush(RGB(255, 255, 128));
			break;
		// both states bellow have blue or red background:
		// ST_SELECTALLOWED has blue background if there's no figure in the particular spot and
		// red background otherwise
		// ST_ENPASSANT has always red background
		// in both styles the background color is mixed with the chessboard color
		case ST_ENPASSANT:
		case ST_SELECTALLOWED: {
			// retrieves color of the background
			// it can be either white or gray (RGB(128, 128, 128))
			COLORREF crBackground = GetPixel(hdc, 1, 1);

			BYTE btRed, btGreen, btBlue;
			if (fig || pcfd->dwState == ST_ENPASSANT) {
				// red background
				btRed = ((int)GetRValue(crBackground) + 255) / 2;
				btGreen = GetGValue(crBackground) / 2;
				btBlue = GetBValue(crBackground) / 2;
			}
			else {
				// blue background
				btRed = GetRValue(crBackground) / 2;
				btGreen = GetGValue(crBackground) / 2;
				btBlue = ((int)GetBValue(crBackground) + 255) / 2;
			}

			// creates brush with the specified color
			hbrBackground = CreateSolidBrush(RGB(btRed, btGreen, btBlue));
			break;
			}
		case ST_SHAH:
			// orange background
			hbrBackground = CreateSolidBrush(RGB(255, 165, 0));
			break;
		case ST_SHAHING:
			// red background
			hbrBackground = CreateSolidBrush(RGB(255, 0, 0));
		}

		if (hbrBackground) {
			// paints the background if needed and frees brush resources
			FillRect(hdc, &rcWnd, hbrBackground);
			DeleteObject(hbrBackground);
		}
		EndPaint(hWnd, &ps);

		// draws the figure bitmap if necessary
		if (fig)
			fig->draw();
		break;
		}
	case WM_LBUTTONDOWN: {
		// sets mouse input to the particular control
		SetCapture(hWnd);
		break;
		}
	// reaction for this message depends on the control's state
	case WM_LBUTTONUP: {
		// retrieves control's size and releases mouse input set with WM_LBUTTONDOWN
		RECT rc;
		GetClientRect(hWnd, &rc);
		ReleaseCapture();
		
		// if mouse is released outside control's rectangle
		if (LOWORD(lParam) > rc.right || HIWORD(lParam) > rc.bottom)
			break;

		// gets control associated data
		Figure *fig = (Figure*)GetWindowLongPtr(hWnd, sizeof(LONG_PTR));
		CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));
		
		switch (pcfd->dwState) {
		case ST_SHAH:
		case ST_DEFAULT: {
			// if there is a figure on the spot and it belongs to the player making move
			bool bIsWhitesTurn = false;
			if (GameData::flags & CHESS_WHITESTURN)
				bIsWhitesTurn = true;
			if (fig && fig->bIsWhite == bIsWhitesTurn) {
				// if there is another control that was selected
				// application sends message to cancel that selection
				if (GameData::hSelected) {
					Figure *prevSelected = (Figure*)GetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR));
					SendMessage(GameData::hSelected, CM_SELECTED, FALSE, (LPARAM)prevSelected);
				}

				// selects the actual control
				SendMessage(hWnd, CM_SELECTED, TRUE, (LPARAM)fig);
			}
			break;
			}
		case ST_ENPASSANT:
		case ST_SELECTALLOWED: {
			// if there's a check
			if (GameData::flags & CHESS_CHECK) {
				// gets handle to the control containing the king
				HWND hShah;
				if (GameData::flags & CHESS_WHITESTURN)
					hShah = GameData::hFieldsTable[GameData::whiteFigures[15]->ptPos.x][GameData::whiteFigures[15]->ptPos.y];
				else
					hShah = GameData::hFieldsTable[GameData::blackFigures[15]->ptPos.x][GameData::blackFigures[15]->ptPos.y];

				// sets state of king's and checking figure's control to default
				CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hShah, 2 * sizeof(LONG_PTR));
				pcfd->dwState = ST_DEFAULT;
				pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hShahing, 2 * sizeof(LONG_PTR));
				pcfd->dwState = ST_DEFAULT;

				// gets controls' size
				RECT rcShah, rcShahing;
				GetWindowRect(hShah, &rcShah);
				GetWindowRect(GameData::hShahing, &rcShahing);

				// tells main window to redraw previously checked and checking controls
				POINT ptLeft = { rcShah.left, rcShah.top };
				POINT ptRight = { rcShah.right, rcShah.bottom };
				ScreenToClient(GetParent(hShah), &ptLeft);
				ScreenToClient(GetParent(hShah), &ptRight);
				SetRect(&rcShah, ptLeft.x, ptLeft.y, ptRight.x, ptRight.y);
				RedrawWindow(GetParent(hShah), &rcShah, nullptr, RDW_INVALIDATE);
				ptLeft = { rcShahing.left, rcShahing.top };
				ptRight = { rcShahing.right, rcShahing.bottom };
				ScreenToClient(GetParent(hShah), &ptLeft);
				ScreenToClient(GetParent(hShah), &ptRight);
				SetRect(&rcShahing, ptLeft.x, ptLeft.y, ptRight.x, ptRight.y);
				RedrawWindow(GetParent(hShah), &rcShahing, nullptr, RDW_INVALIDATE);

				// sets global variables to state without check
				GameData::flags &= ~CHESS_CHECK;
				GameData::hShahing = nullptr;
			}

			// moves the figure from selected item to choosen position
			fig = (Figure*)GetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR));
			fig->move(pcfd->ptPos);
			
			// sets previously selected control to default state without figure on it
			SetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR), (LONG_PTR)nullptr);
			pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hSelected, 2 * sizeof(LONG_PTR));
			pcfd->dwState = ST_DEFAULT;

			// redraws previously selected control and zeros global handle to a selected control
			// note that there's no call to main window to redraw it because
			// control must be redrawed immediately
			RECT rcPrev;
			GetClientRect(GameData::hSelected, &rcPrev);
			HBRUSH hbrChessboard;
			if ((pcfd->ptPos.x + pcfd->ptPos.y) % 2)
				hbrChessboard = CreateSolidBrush(RGB(128, 128, 128));
			else
				hbrChessboard = CreateSolidBrush(0xFFFFFF);
			HDC hdc = GetDC(GameData::hSelected);
			FillRect(hdc, &rcPrev, hbrChessboard);
			DeleteObject(hbrChessboard);
			ReleaseDC(GameData::hSelected, hdc);
			GameData::hSelected = nullptr;
			
			// pops all items from the stack and sends message to all the controls
			// from stack so they get back to default state
			while (!GameData::PossibleToMove.empty()) {
				SendMessage(GameData::PossibleToMove.top(), CM_ALLOWSELECT, FALSE, 0);
				GameData::PossibleToMove.pop();
			}

			// changes players' turn
			if (GameData::flags & CHESS_WHITESTURN)
				GameData::flags &= ~CHESS_WHITESTURN;
			else
				GameData::flags |= CHESS_WHITESTURN;
			
			// checks if there is a check
			if (CheckShah(fig)) {
				// if there is set global variable
				GameData::flags |= CHESS_CHECK;

				// checks if there is checkmate
				if (IsMat()) {
					// if there is, prints 'Checkmate' above or below names' table
					//
					HFONT hfCheckmate = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Arial");
					HWND hMainWindow = GetParent(hWnd);
					HDC hdc = GetDC(hMainWindow);
					HFONT hfOld = (HFONT)SelectObject(hdc, hfCheckmate);
					RECT rcWnd;
					GetClientRect(hMainWindow, &rcWnd);
					BITMAP bmChessfield;
					GetObject(GameData::hbmChessboard, sizeof(BITMAP), &bmChessfield);

					// if white player won then text is positioned below names' table
					// otherwise it's positioned above
					POINT ptNamesTableEdge = { 0, 0 };
					if (!(GameData::flags & CHESS_WHITESTURN)) {
						RECT rc;
						GetClientRect(GetDlgItem(hMainWindow, ID_NAMESTABLE), &rc);
						ptNamesTableEdge.y = rc.bottom;
					}
					ClientToScreen(GetDlgItem(hMainWindow, ID_NAMESTABLE), &ptNamesTableEdge);
					ScreenToClient(hMainWindow, &ptNamesTableEdge);

					RECT rcCheckmate;
					if (GameData::flags & CHESS_WHITESTURN)
						SetRect(&rcCheckmate, 0, ptNamesTableEdge.y - 50,
							(rcWnd.right - bmChessfield.bmWidth) / 2, ptNamesTableEdge.y - 10);
					else
						SetRect(&rcCheckmate, 0, ptNamesTableEdge.y + 10,
							(rcWnd.right - bmChessfield.bmWidth) / 2, ptNamesTableEdge.y + 50);

					SetBkMode(hdc, TRANSPARENT);
					SetTextColor(hdc, RGB(255, 0, 0));
					DrawText(hdc, L"Checkmate!", -1, &rcCheckmate, DT_CENTER | DT_VCENTER);

					SelectObject(hdc, hfOld);
					DeleteObject(hfCheckmate);
					ReleaseDC(hMainWindow, hdc);

					// on the right side of the window displays new button to restart the game
					int nMargin = (rcWnd.right - bmChessfield.bmWidth) / 2;
					int nRightChessfieldEdge = (rcWnd.right + bmChessfield.bmWidth) / 2;
					CreateWindow(L"button", L"Nowa gra", WS_CHILD | WS_VISIBLE,
						nRightChessfieldEdge + (nMargin - 100) / 2, (rcWnd.bottom - 30) / 2,
						100, 30, hMainWindow, (HMENU)IDM_OPCJE_NEWGAME, hInst, nullptr);
				}
			}

			// updates names' table
			RedrawWindow(GetDlgItem(GetParent(hWnd), ID_NAMESTABLE), nullptr, nullptr, RDW_INVALIDATE);
			break;
			}
		}
		break;
		}
	// if there is a selected control then this message cancels that selection
	case WM_RBUTTONUP:
		// if there is a selected control...
		if (GameData::hSelected) {
			// ...unselects it and zeros global handle to selected item
			Figure *fig = (Figure*)GetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR));
			SendMessage(GameData::hSelected, CM_SELECTED, FALSE, (LPARAM)fig);
			GameData::hSelected = nullptr;
		}
		break;
	// indicates that player wants to move the figure contained in this control
	case CM_SELECTED: {
		// retrieves control's size and saves pointer to figure from the control in a local variable
		RECT rcWnd;
		GetClientRect(hWnd, &rcWnd);
		Figure *fig = (Figure*)lParam;

		// wParam is non-zero if the control is to be selected
		// wParam is zero if the control must be unselected
		if (wParam) {
			// sets a global handle to selected control and
			// changes state of the control to ST_SELECTED
			GameData::hSelected = hWnd;
			CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));
			pcfd->dwState = ST_SELECTED;

			// background of the selected control is painted in yellow
			HDC hdc = GetDC(hWnd);
			HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 128));
			FillRect(hdc, &rcWnd, hbr);
			DeleteObject(hbr);
			ReleaseDC(hWnd, hdc);

			// shows possible moves for the selected figure and redraws the figure
			fig->draw();
			fig->showmoves();
		}
		else {
			// clears the global stack of handles to controls being in ST_SELECTALLOWED state and
			// changes their state
			while (!GameData::PossibleToMove.empty()) {
				SendMessage(GameData::PossibleToMove.top(), CM_ALLOWSELECT, FALSE, 0);
				GameData::PossibleToMove.pop();
			}

			// retrieves control associated data
			CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));

			// if there's a check
			if (GameData::flags & CHESS_CHECK) {
				// if the figure in the control is king then it's state is set back to ST_SHAH
				if (dynamic_cast<King*>(fig)) {
					pcfd->dwState = ST_SHAH;
					RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
					return 0;
				}
			}

			// if the figure is not a checked king then control's state is set to default
			pcfd->dwState = ST_DEFAULT;

			// tells main window to redraw the field
			POINT ptLeft = { rcWnd.left, rcWnd.top };
			POINT ptRight = { rcWnd.right, rcWnd.bottom };
			ClientToScreen(hWnd, &ptLeft);
			ClientToScreen(hWnd, &ptRight);
			ScreenToClient(GetParent(hWnd), &ptLeft);
			ScreenToClient(GetParent(hWnd), &ptRight);
			RECT rcTemp;
			SetRect(&rcTemp, ptLeft.x, ptLeft.y, ptRight.x, ptRight.y);
			RedrawWindow(GetParent(hWnd), &rcTemp, nullptr, RDW_INVALIDATE);
		}
		break;
		}
	// this message indicates that a selected figure might move to the particular field
	case CM_ALLOWSELECT: {
		// retrieves data associated with the control
		CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));
		HDC hdc = GetDC(hWnd);
		Figure *fig = (Figure*)GetWindowLongPtr(hWnd, sizeof(LONG_PTR));
		RECT rcWnd;
		GetClientRect(hWnd, &rcWnd);

		// wParam is non-zero if the control's state is to be set to
		// ST_SELECTALLOWED or ST_ENPASSANT
		if (wParam) {
			// tests if potencial move will not put the king in check state
			// if it will then the message returns
			if (!TestIfMoveIsValid(hWnd))
				return 0;

			// pushes to the global stack handles to all controls that will be in state
			// ST_SELECTALLOWED or ST_ENPASSANT
			GameData::PossibleToMove.push(hWnd);

			// lParam can have value ST_ENPASSANT indicating that figure moving to that control
			// will perform en passant
			// in this case control's state is set to ST_ENPASSANT
			// else control's state is set to ST_SELECTALLOWED
			if (lParam == ST_ENPASSANT)
				pcfd->dwState = ST_ENPASSANT;
			else
				pcfd->dwState = ST_SELECTALLOWED;

			// draws the background of the control
			// the background is red when figure would capture enemy figure
			// and it's blue if square is free
			// note that background color is mixed with the chessboard's color
			COLORREF crBackground = GetPixel(hdc, 1, 1);
			BYTE btRed, btGreen, btBlue;
			bool bIsWhitesTurn = false;
			if (GameData::flags & CHESS_WHITESTURN)
				bIsWhitesTurn = true;
			if ((fig && fig->bIsWhite != bIsWhitesTurn) || lParam == ST_ENPASSANT) {
				btRed = ((int)GetRValue(crBackground) + 255) / 2;
				btGreen = GetGValue(crBackground) / 2;
				btBlue = GetBValue(crBackground) / 2;
			}
			else {
				btRed = GetRValue(crBackground) / 2;
				btGreen = GetGValue(crBackground) / 2;
				btBlue = ((int)GetBValue(crBackground) + 255) / 2;
			}
			HBRUSH hbr = CreateSolidBrush(RGB(btRed, btGreen, btBlue));
			FillRect(hdc, &rcWnd, hbr);
			DeleteObject(hbr);
		}
		// wParam is zero if control's state should be set back to the state before ST_SELECTALLOWED
		else {
			// checking figure's state turns back to ST_SHAHING
			if ((GameData::flags & CHESS_CHECK) && hWnd == GameData::hShahing) {
				pcfd->dwState = ST_SHAHING;
				return 0;
			}

			// else control's state is set back to default
			pcfd->dwState = ST_DEFAULT;

			// redraws background of the control
			HBRUSH hbrChessboard;
			if ((pcfd->ptPos.x + pcfd->ptPos.y) % 2)
				hbrChessboard = CreateSolidBrush(RGB(128, 128, 128));
			else
				hbrChessboard = CreateSolidBrush(0xFFFFFF);
			FillRect(hdc, &rcWnd, hbrChessboard);
			DeleteObject(hbrChessboard);
		}
		ReleaseDC(hWnd, hdc);

		// redraws the figure
		if (fig)
			fig->draw();
		break;
		}
	// this message is sent when pawn reaches end of the chessboard and it is promoted to another figure
	case CM_PAWNPROMOTION: {
		// displays pawn promotion dialog box
		INT_PTR ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_PROMOTION), GetParent(hWnd), PromotionProc);

		// gets pointer to the pawn being promoted and the figure being created and promotes it to figure
		// selected by user in dialog
		Pawn *promoted = (Pawn*)GetWindowLongPtr(hWnd, sizeof(LONG_PTR));
		Figure *newFigure = promoted->promote((int)ret);
		delete promoted;

		// redraws control's background
		CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));
		HBRUSH hbrErase;
		if ((pcfd->ptPos.x + pcfd->ptPos.y) % 2)
			hbrErase = CreateSolidBrush(RGB(128, 128, 128));
		else
			hbrErase = CreateSolidBrush(0xFFFFFF);
		HDC hdc = GetDC(hWnd);
		RECT rcField;
		GetClientRect(hWnd, &rcField);
		FillRect(hdc, &rcField, hbrErase);
		DeleteObject(hbrErase);
		ReleaseDC(hWnd, hdc);

		// draws new figure
		newFigure->draw();

		// checks if the new figure checks the opposite king
		if (CheckShah(newFigure)) {
			GameData::flags |= CHESS_CHECK;
			// checks if it's checkmate
			if (IsMat()) {
				///
				MessageBox(GetParent(hWnd), L"Game Over", L"Test", MB_ICONINFORMATION);
			}
		}
		break;
		}
	// deallocates memory associated with the control
	case WM_DESTROY: {
		CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));
		delete pcfd;
		break;
		}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

//
//	FUNCTION: InitChessFields(HWND, HBITMAP)
//
//	PURPOSE: Initializes ChessField controls
//
//	COMMENT: ChessField control reprezents a single square area on the chessboard
//		which can be occupied by chess figures
//
//		Function returns TRUE if no error occured and FALSE otherwise
//
BOOL InitChessFields(HWND hWnd, HBITMAP hbmBitmap) {
	// allocating memory for global two-dimensional table
	// of handles to created controls
	GameData::hFieldsTable = new HWND*[8];
	if (!GameData::hFieldsTable)
		return FALSE;
	for (int i = 0; i < 8; ++i) {
		GameData::hFieldsTable[i] = new HWND[8];
		if (!GameData::hFieldsTable[i]) {
			for (int j = 0; j < i; ++j) {
				delete[] GameData::hFieldsTable[j];
			}
			delete[] GameData::hFieldsTable;
			return FALSE;
		}
	}

	// retrieving window's and bitmap's size
	RECT rcWnd;
	GetClientRect(hWnd, &rcWnd);
	BITMAP bmp;
	GetObject(hbmBitmap, sizeof(bmp), &bmp);

	// counting width and height of ChessField control
	//	NOTE: summary width of left and right margin and summary height
	//		of upper and bottom margin of the chessboard bitmap is 30
	const int nWidth = (bmp.bmWidth - 30) / 8;
	const int nHeight = (bmp.bmHeight - 30) / 8;
	// we save x coordinate of left edge of chessboard in a variable
	const int nLeftCornerX = (rcWnd.right - bmp.bmWidth) / 2 + 15;

	// this variable contains coordiantes (relative to main window) of
	// upper-left corner of ChessField control being created
	// it's updated after each creation
	POINT ptInitPoint;
	ptInitPoint.x = nLeftCornerX;
	ptInitPoint.y = (rcWnd.bottom - bmp.bmHeight) / 2 + 15;

	// loop where we create ChessField controls and save their handles
	// to a global variable
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			CHESSFIELDDATA *pcfd = new CHESSFIELDDATA;
			pcfd->dwState = ST_DEFAULT;
			pcfd->ptPos = { j, i };
			GameData::hFieldsTable[j][i] = CreateWindow(L"ChessField", nullptr, WS_CHILD | WS_VISIBLE,
				ptInitPoint.x, ptInitPoint.y, nWidth, nHeight, hWnd, nullptr, hInst, pcfd);
			if (!GameData::hFieldsTable[j][i]) {
				return FALSE;
			}
			ptInitPoint.x += nWidth;
		}
		ptInitPoint.x = nLeftCornerX;
		ptInitPoint.y += nHeight;
	}
	return TRUE;
}

//
//	FUCTION: LoadClassBitmaps()
//
//	PURPOSE: Loads from files bitmaps for particular figures
//
//		This bitmaps are displayed in field occupied by a particular figure
//
BOOL LoadClassBitmaps() {
	Pawn::hbmPawn = (HBITMAP)LoadImage(nullptr, L"chess_graphics\\fig\\pionek.bmp",
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!Pawn::hbmPawn)
		return FALSE;
	Pawn::hbmMask = CreateBitmapMask(Pawn::hbmPawn, RGB(255, 0, 255));

	Rook::hbmRook = (HBITMAP)LoadImage(nullptr, L"chess_graphics\\fig\\wieza.bmp",
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!Rook::hbmRook)
		return FALSE;
	Rook::hbmMask = CreateBitmapMask(Rook::hbmRook, RGB(255, 0, 255));

	Knight::hbmKnight = (HBITMAP)LoadImage(nullptr, L"chess_graphics\\fig\\kon.bmp",
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!Knight::hbmKnight)
		return FALSE;
	Knight::hbmMask = CreateBitmapMask(Knight::hbmKnight, RGB(255, 0, 255));

	Bishop::hbmBishop = (HBITMAP)LoadImage(nullptr, L"chess_graphics\\fig\\goniec.bmp",
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!Bishop::hbmBishop)
		return FALSE;
	Bishop::hbmMask = CreateBitmapMask(Bishop::hbmBishop, RGB(255, 0, 255));

	Queen::hbmQueen = (HBITMAP)LoadImage(nullptr, L"chess_graphics\\fig\\dama.bmp",
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!Queen::hbmQueen)
		return FALSE;
	Queen::hbmMask = CreateBitmapMask(Queen::hbmQueen, RGB(255, 0, 255));

	King::hbmKing = (HBITMAP)LoadImage(nullptr, L"chess_graphics\\fig\\krol.bmp",
		IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!King::hbmKing)
		return FALSE;
	King::hbmMask = CreateBitmapMask(King::hbmKing, RGB(255, 0, 255));

	return TRUE;
}

//
//	FUNCTION: DeleteClassBitmaps()
//
//	PURPOSE: Frees system resources occupied by bitmaps of chess figures
//
void DeleteClassBitmaps() {
	DeleteObject(Pawn::hbmPawn);
	DeleteObject(Pawn::hbmMask);

	DeleteObject(Rook::hbmRook);
	DeleteObject(Rook::hbmMask);

	DeleteObject(Knight::hbmKnight);
	DeleteObject(Knight::hbmMask);

	DeleteObject(Bishop::hbmBishop);
	DeleteObject(Bishop::hbmMask);

	DeleteObject(Queen::hbmQueen);
	DeleteObject(Queen::hbmMask);

	DeleteObject(King::hbmKing);
	DeleteObject(King::hbmMask);
}

//
//	FUNCTION: InitFigures()
//
//	PURPOSE: Initializes chess figures so the new game might begin
//
//	COMMENT: Figures' positions are the same as defined in chess rules
//		A pointer to each figure is saved in a global variable
//
BOOL InitFigures() {
	GameData::blackFigures[10] = new Rook(false, 0, 0);
	GameData::blackFigures[11] = new Rook(false, 7, 0);
	GameData::whiteFigures[10] = new Rook(true, 0, 7);
	GameData::whiteFigures[11] = new Rook(true, 7, 7);

	GameData::blackFigures[8] = new Knight(false, 1, 0);
	GameData::blackFigures[9] = new Knight(false, 6, 0);
	GameData::whiteFigures[8] = new Knight(true, 1, 7);
	GameData::whiteFigures[9] = new Knight(true, 6, 7);

	GameData::blackFigures[12] = new Bishop(false, 2, 0);
	GameData::blackFigures[13] = new Bishop(false, 5, 0);
	GameData::whiteFigures[12] = new Bishop(true, 2, 7);
	GameData::whiteFigures[13] = new Bishop(true, 5, 7);

	GameData::blackFigures[14] = new Queen(false, 3, 0);
	GameData::whiteFigures[14] = new Queen(true, 3, 7);

	GameData::blackFigures[15] = new King(false, 4, 0);
	GameData::whiteFigures[15] = new King(true, 4, 7);

	for (int i = 0; i < 8; ++i) {
		GameData::blackFigures[i] = new Pawn(false, i, 1);
		GameData::whiteFigures[i] = new Pawn(true, i, 6);
	}

	for (int i = 0; i < 16; ++i) {
		if (!GameData::blackFigures[i] || !GameData::whiteFigures[i])
			return FALSE;
	}
	return TRUE;
}

//
//	FUNCTION: CheckShah(Figure*)
//
//	PURPOSE: Function checks if there is a check
//
//	COMMENT:
//		Function's parameter is pointer to a figure that made last move
//		This figure is first checked if it checks the king
//		Function returns non-zero value if there's a check. Otherwise it returns 0
//
BOOL CheckShah(Figure *fig) {
	// retrieves table of figures that must be checked for checking the king and
	// pointer to the appropriate king
	Figure **figuresToCheck;
	Figure *shah;
	std::list<Figure*> *pointerToPromoted;
	if (fig->bIsWhite) {
		figuresToCheck = GameData::whiteFigures;
		shah = GameData::blackFigures[15];
		pointerToPromoted = &GameData::whitePromoted;
	}
	else {
		figuresToCheck = GameData::blackFigures;
		shah = GameData::whiteFigures[15];
		pointerToPromoted = &GameData::blackPromoted;
	}

	Figure *checking = fig; // pointer to a checking figure

	// checks if last moved figure checks the king
	if (!fig->isinrange(shah->ptPos))
		checking = nullptr;
	// checks if other figures check the king
	// note that we don't check if pawns or knights or the king are checking the opposite king
	// theese figures (except king) can check the enemy king only if there were just moved
	if (!checking) {
		for (int i = 10; i < 15; ++i) {
			// if figure was already tested
			if (figuresToCheck[i] == fig)
				continue;
			if (figuresToCheck[i] && figuresToCheck[i]->isinrange(shah->ptPos)) {
				checking = figuresToCheck[i];
				break;
			}
		}
	}

	if (!pointerToPromoted->empty()) {
		std::list<Figure*>::iterator it;
		for (it = pointerToPromoted->begin(); it != pointerToPromoted->end(); ++it) {
			if (*it == fig)
				continue;
			if ((*it)->isinrange(shah->ptPos)) {
				checking = *it;
				break;
			}
		}
	}

	// if there's no figure that checks the king function returns 0
	if (!checking)
		return FALSE;

	// sets global handle to a checking figure
	GameData::hShahing = GameData::hFieldsTable[checking->ptPos.x][checking->ptPos.y];

	// sets checked king's state to ST_SHAH and redraws its control
	CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hFieldsTable[shah->ptPos.x][shah->ptPos.y],
		2 * sizeof(LONG_PTR));
	pcfd->dwState = ST_SHAH;
	RedrawWindow(GameData::hFieldsTable[shah->ptPos.x][shah->ptPos.y], nullptr, nullptr, RDW_INVALIDATE);

	// sets checking figure's state to ST_SHAHING and redraws its control
	pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hFieldsTable[checking->ptPos.x][checking->ptPos.y],
		2 * sizeof(LONG_PTR));
	pcfd->dwState = ST_SHAHING;
	RedrawWindow(GameData::hFieldsTable[checking->ptPos.x][checking->ptPos.y], nullptr, nullptr, RDW_INVALIDATE);

	return TRUE;
}

//
//	FUNCTION: TestIfMoveIsValid(HWND)
//
//	PURPOSE: Function simulates a move of selected figure to a particular square and tests
//		if it leads to a check on figure's owner's king
//
//	COMMENT:
//		Function is called during processing CM_ALLOWSELECT message for
//		ChessField class controls
//		hWnd parameter defines square where figure can potentially move
//		Function returns 0 if move leads to check and non-zero value otherwise
//
bool TestIfMoveIsValid(HWND hWnd) {
	// gets pointers to a selected figure and to a figure currently in hWnd control
	Figure *moving = (Figure*)GetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR));
	Figure *current = (Figure*)GetWindowLongPtr(hWnd, sizeof(LONG_PTR));

	// simulates the move by changing figures' pointers associated with appropriate controls
	SetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR), (LONG_PTR)nullptr);
	SetWindowLongPtr(hWnd, sizeof(LONG_PTR), (LONG_PTR)moving);

	// determines which figures and which king need checking
	Figure **figuresToCheck;
	Figure *shah;
	std::list<Figure*> *pointerToPromoted;
	POINT ptShahPos;
	if (moving->bIsWhite) {
		figuresToCheck = GameData::blackFigures;
		shah = GameData::whiteFigures[15];
		pointerToPromoted = &GameData::blackPromoted;
	}
	else {
		figuresToCheck = GameData::whiteFigures;
		shah = GameData::blackFigures[15];
		pointerToPromoted = &GameData::whitePromoted;
	}

	// gets control's data
	CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(hWnd, 2 * sizeof(LONG_PTR));

	// theese variables indicate how many figures we need to check
	// if we simulate king's move then we must check all enemy's figures
	// else pawns, knights and enemy king don't need checking
	int starting = 0;
	int ending = 16;

	// if we simulate king's moves then we need to update king's position
	// note that in the simulation we don't change figure's positions (ptPos field of figure class)
	if (dynamic_cast<King*>(moving))
		ptShahPos = { pcfd->ptPos.x, pcfd->ptPos.y };
	else {
		ptShahPos = { shah->ptPos.x, shah->ptPos.y };
		if (!(GameData::flags & CHESS_CHECK)) {
			starting = 10;
			ending = 15;
		}
	}

	/*if (GameData::flags & CHESS_CHECK) {
		Figure *checking = (Figure*)GetWindowLongPtr(GameData::hShahing, sizeof(LONG_PTR));
		if ()
	}*/

	int i = 0;

	if (!pointerToPromoted->empty()) {
		std::list<Figure*>::iterator it;
		for (it = pointerToPromoted->begin(); it != pointerToPromoted->end(); ++it) {
			if (*it != current && (*it)->isinrange(ptShahPos))
				i = -1;
		}
	}

	if (!i) {
		// loop where we actually check if there's check
		for (i = starting; i < ending; ++i) {
			// if figure exists, it's not the figure that would be captured in this move and king is in range of this figure
			if (figuresToCheck[i] && figuresToCheck[i] != current && figuresToCheck[i]->isinrange(ptShahPos))
				break;
		}
	}

	// sets game state back to one before simulation
	SetWindowLongPtr(GameData::hSelected, sizeof(LONG_PTR), (LONG_PTR)moving);
	SetWindowLongPtr(hWnd, sizeof(LONG_PTR), (LONG_PTR)current);

	// iterator's value indicates if there is a check
	if (i == ending)
		return true;
	return false;
}

//
//	FUNCTION: IsMat()
//
//	PURPOSE: Function examines if there's a checkmate
//
//	COMMENT:
//		The function counts all moves that are possible for the player
//		If there are no moves available function returns non-zero value which means checkmate
//
BOOL IsMat() {
	// subclasses all ChessField controls
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			SetWindowLongPtr(GameData::hFieldsTable[j][i], GWLP_WNDPROC, (LONG_PTR)MatCheckingProc);
		}
	}

	// determines which figures need checking
	Figure **figuresToCheck;
	if (GameData::flags & CHESS_WHITESTURN)
		figuresToCheck = GameData::whiteFigures;
	else
		figuresToCheck = GameData::blackFigures;

	// this value indicates if there is a possible move for the player
	// pointer to this variable is sent to control via CM_MATINDICATOR message
	// so the control's procedure may modify it
	int nMatIndicator = 0;
	SendMessage(GameData::hFieldsTable[0][0], CM_MATINDICATOR, 0, (LPARAM)&nMatIndicator);

	// loop where we check if any figure can do a valid move
	for (int i = 0; i < 16; ++i) {
		if (figuresToCheck[i]) {
			// sets a handle to seleted control that is needed for showmoves method
			GameData::hSelected = GameData::hFieldsTable[figuresToCheck[i]->ptPos.x][figuresToCheck[i]->ptPos.y];
			figuresToCheck[i]->showmoves();
			// if there are possible moves
			if (nMatIndicator)
				break;
		}
	}

	// zero the global handle
	GameData::hSelected = nullptr;

	// subclass all ChessField controls with their default procedure
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			SetWindowLongPtr(GameData::hFieldsTable[j][i], GWLP_WNDPROC, (LONG_PTR)ChessFieldProc);
		}
	}

	// if it's checkmate function returns TRUE. Else it returns FALSE
	if (nMatIndicator)
		return FALSE;
	return TRUE;
}

//
//	FUNCTION: MatCheckingProc(HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE: Processes messages for ChessField controls when application must test if there's a checkmate.
//
//	CM_ALLOWSELECT: if move is valid then modify appropriate value
//	CM_MATINDICATOR: save in a static variable pointer to a variable that will indicate if it's checkmate
//
LRESULT CALLBACK MatCheckingProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int *nMovePossibilites;
	switch (msg) {
	case CM_ALLOWSELECT:
		if (TestIfMoveIsValid(hWnd))
			++*nMovePossibilites;
		break;
	case CM_MATINDICATOR:
		nMovePossibilites = (int*)lParam;
		break;
	default:
		CallWindowProc(ChessFieldProc, hWnd, msg, wParam, lParam);
	}
	return 0;
}

//
//	FUNCTION: InitNamesTable(HWND)
//
//	PURPOSE: Initializes static control that contains names of players and
//		indicates which player has a current move
//
//	COMMENT: Control's position is calculated based on the main window's and
//		chessboard bitmap's size
//
//		Created static control is being subclassed
//
BOOL InitNamesTable(HWND hWnd) {
	// retrieves window's and chessboard bitmap's size
	RECT rcWnd;
	GetClientRect(hWnd, &rcWnd);
	BITMAP bmInfo;
	GetObject(GameData::hbmChessboard, sizeof(bmInfo), &bmInfo);

	// calculating x coordinate of the left edge of the bitmap
	LONG leftMargin = (rcWnd.right - bmInfo.bmWidth) / 2;

	HWND hTable = CreateWindow(L"static", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER,
		10, rcWnd.bottom / 2 - 50, leftMargin - 10, 100, hWnd, (HMENU)ID_NAMESTABLE, hInst, nullptr);

	if (!hTable)
		return FALSE;

	// subclassing static control
	g_StaticProc = (WNDPROC)SetWindowLongPtr(hTable, GWLP_WNDPROC, (LONG_PTR)NamesTableProc);
	return TRUE;
}

//
//	FUNCTION: NamesTableProc(HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE: Processes messages for subclassed static control called in this document names' table.
//
//		WM_PAINT: Draws content of the names' table
//
LRESULT CALLBACK NamesTableProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		// gets size of the control and draws its background
		RECT rcTable;
		GetClientRect(hWnd, &rcTable);
		HBRUSH hbrBknd = CreateSolidBrush(RGB(190, 190, 190));
		FillRect(hdc, &rcTable, hbrBknd);
		DeleteObject(hbrBknd);

		// draws squares next to players' names that indicate color of figures with which 
		// player plays
		RECT rcColor;
		SetRect(&rcColor, 10, rcTable.bottom / 4 - 8, 26, rcTable.bottom / 4 + 8);
		FillRect(hdc, &rcColor, (HBRUSH)GetStockObject(BLACK_BRUSH));
		OffsetRect(&rcColor, 0, rcTable.bottom / 2);
		FillRect(hdc, &rcColor, (HBRUSH)GetStockObject(WHITE_BRUSH));

		// this variables will contain x corrdinate of right end of rectangle containing player's name
		LONG nUpperTextEnd, nBottomTextEnd;

		// creates font with which the program will write players' names and
		// determines which colors a particular player is using
		HFONT hfFont = CreateFont(0, 0, 0, 0, FW_THIN, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, nullptr);
		HFONT hfOld = (HFONT)SelectObject(hdc, hfFont);
		LPCWSTR szWhite, szBlack;
		if (GameData::flags & CHESS_FIRSTSTART) {
			szWhite = (LPCWSTR)GameData::pnNames.szFirst;
			szBlack = (LPCWSTR)GameData::pnNames.szSecond;
		}
		else {
			szWhite = (LPCWSTR)GameData::pnNames.szSecond;
			szBlack = (LPCWSTR)GameData::pnNames.szFirst;
		}

		// writes players' names
		SIZE sizeText;
		RECT rcText;
		SetRect(&rcText, 35, rcTable.bottom / 4 - 10, rcTable.right - 15, rcTable.bottom / 4 + 10);
		SetBkColor(hdc, RGB(190, 190, 190));
		GetTextExtentPoint32(hdc, szBlack, lstrlen(szBlack), &sizeText);
		nUpperTextEnd = sizeText.cx + 35;
		DrawText(hdc, szBlack, -1, &rcText, DT_LEFT | DT_VCENTER);
		OffsetRect(&rcText, 0, rcTable.bottom / 2);
		GetTextExtentPoint32(hdc, szWhite, lstrlen(szWhite), &sizeText);
		nBottomTextEnd = sizeText.cx + 35;
		DrawText(hdc, szWhite, -1, &rcText, DT_LEFT | DT_VCENTER);
		SelectObject(hdc, hfOld);
		DeleteObject(hfFont);

		// draws a green circle that indicates who of the players has the actual move
		HBRUSH hbrGreen = CreateSolidBrush(RGB(0, 200, 0));
		HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, hbrGreen);
		HPEN hpBkng = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
		HPEN hpOld = (HPEN)SelectObject(hdc, hpBkng);
		if (GameData::flags & CHESS_WHITESTURN)
			Ellipse(hdc, nBottomTextEnd + 10, 3 * rcTable.bottom / 4 - 7, nBottomTextEnd + 24, 3 * rcTable.bottom / 4 + 7);
		else
			Ellipse(hdc, nUpperTextEnd + 10, rcTable.bottom / 4 - 7, nUpperTextEnd + 24, rcTable.bottom / 4 + 7);

		// freeing resources
		SelectObject(hdc, hbrOld);
		SelectObject(hdc, hpOld);
		DeleteObject(hbrGreen);
		DeleteObject(hpBkng);

		EndPaint(hWnd, &ps);
		break;
		}
	default:
		return CallWindowProc(g_StaticProc, hWnd, msg, wParam, lParam);
	}
	return 0;
}

//
//	FUNCTION: PromotionProc(HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE: Processes messages for pawn promotion dialog box
//
//	COMMENT:
//		This dialog is displayed after pawn reaches end of the chessboard.
//		Pawn then gets promoted to another figure (rook, knight, bishop or queen)
//		The dialog lets user choose the figure he wants to promote pawn to
//
INT_PTR CALLBACK PromotionProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	// initializes listbox with names of figures to select
	case WM_INITDIALOG: {
		HWND hFiguresList = GetDlgItem(hDlg, IDC_FIGURECHOISE);
		SendMessage(hFiguresList, LB_ADDSTRING, 0, (LPARAM)L"Wie¿a");
		SendMessage(hFiguresList, LB_ADDSTRING, 0, (LPARAM)L"Koñ");
		SendMessage(hFiguresList, LB_ADDSTRING, 0, (LPARAM)L"Goniec");
		SendMessage(hFiguresList, LB_ADDSTRING, 0, (LPARAM)L"Hetman");
		break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		// if user selects an item from listbox OK button is being enabled
		case IDC_FIGURECHOISE:
			if (HIWORD(wParam) == LBN_SELCHANGE)
				EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			break;
		case IDOK:
			// end the dialog with value indicating user's choice
			EndDialog(hDlg,
				SendMessage(GetDlgItem(hDlg, IDC_FIGURECHOISE), LB_GETCURSEL, 0, 0));
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//
//	FUNCTION: DeleteAllExistingFigures()
//
//	PURPOSE: Function deallocates memory for all figures currently on the chessboard.
//
void DeleteAllExistingFigures() {
	for (int i = 0; i < 16; ++i) {
		// for white figures
		Figure *fig = GameData::whiteFigures[i];

		// checks if fig is not null
		if (fig) {
			// sets control's state to default
			CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hFieldsTable[fig->ptPos.x][fig->ptPos.y],
				2 * sizeof(LONG_PTR));
			pcfd->dwState = ST_DEFAULT;

			// rest of vacumming
			delete fig;
		}

		// part for black figures
		fig = GameData::blackFigures[i];
		if (fig) {
			CHESSFIELDDATA *pcfd = (LPCHESSFIELDDATA)GetWindowLongPtr(GameData::hFieldsTable[fig->ptPos.x][fig->ptPos.y],
				2 * sizeof(LONG_PTR));
			pcfd->dwState = ST_DEFAULT;
			delete fig;
		}
	}
}

//
//	FUNCTION: ResetGame()
//
//	PURPOSE: Function restarts the game.
//
BOOL ResetGame() {
	// deallocates memory occupied by remaining figures
	DeleteAllExistingFigures();
	// allocates memory for new figures
	if (!InitFigures())
		return FALSE;
	GameData::flags |= CHESS_WHITESTURN;
	GameData::flags &= ~CHESS_CHECK;

	// forces the main window to redraw itself
	RedrawWindow(GetParent(GameData::hFieldsTable[0][0]), nullptr, nullptr, RDW_INVALIDATE);
	return TRUE;
}
