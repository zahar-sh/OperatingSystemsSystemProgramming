#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

typedef struct _MRECT {
	DOUBLE x;
	DOUBLE y;
	DOUBLE dx;
	DOUBLE dy;
	LONG width;
	LONG height;
	HBRUSH brush;
} MRECT;

RECT ToRECT(MRECT& rect) {
	RECT r{ (LONG)rect.x,(LONG)rect.y, (LONG)(rect.x + rect.width), (LONG)(rect.y + rect.height) };
	return r;
}


// Global variables
static TCHAR szWindowClass[] = _T("Front end");
static TCHAR szTitle[] = _T("Better than HTML");

HINSTANCE hInst;
HWND hWnd;
HBRUSH bgBrush;

BOOL showImage = 0;
HBITMAP image;
MRECT mrect;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void repaint(HDC, RECT);
void update(DOUBLE);

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), szTitle, NULL);
		return 1;
	}

	hInst = hInstance;

	hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		900, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), szTitle, NULL);
		return 1;
	}

	image = (HBITMAP)LoadImage(hInst, _T("image.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!image)
	{
		MessageBox(NULL, _T("Load image failed!"), szTitle, NULL);
		return 1;
	}
	mrect.x = 120;
	mrect.y = 70;
	mrect.width = 150;
	mrect.height = 170;

	LOGBRUSH br;
	br.lbStyle = BS_SOLID;
	br.lbColor = 0x7799aa;
	mrect.brush = CreateBrushIndirect(&br);

	br.lbStyle = BS_SOLID;
	br.lbColor = 0xFDFDFD;
	bgBrush = CreateBrushIndirect(&br);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	DOUBLE deltaTime;
	LONG lastTime = GetCurrentTime();
	// Main message loop:
	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

			LONG time = GetCurrentTime();
			deltaTime = ((DOUBLE)time - lastTime) / 1000.;
			lastTime = time;
			update(deltaTime);
		}
	}

	return (int)msg.wParam;
}

LONG randInt() {
	LONG v = (rand() % 250) + 100;
	return (((v & 1) == 0) ? v : -v);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int dx = -1, dy = -1;
	switch (message)
	{
	case WM_LBUTTONDOWN:
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);

		if (mrect.x < xPos && xPos < mrect.x + mrect.width &&
			mrect.y < yPos && yPos < mrect.y + mrect.height)
		{
			dx = xPos - mrect.x;
			dy = yPos - mrect.y;

			mrect.dx = mrect.dy = 0;
			break;
		}
	}
	case WM_LBUTTONUP:
		if (dx > 0 && dy > 0) {
			mrect.dx = mrect.dy = 0;
			dx = dy = -1;
		}
		break;
	case WM_MOUSEMOVE:
	{
		if (dx > 0 && dy > 0) {
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			mrect.x = xPos - dx;
			mrect.y = yPos - dy;
		}
	}
	break;
	case WM_MOUSEWHEEL:
	{
		mrect.dx = mrect.dy = 0;
		SHORT zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (GetAsyncKeyState(VK_SHIFT) != 0) {
			mrect.x += zDelta * 0.05;
		}
		else
		{
			mrect.y += zDelta * 0.05;
		}
	}
	break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT:
			mrect.dx = -200;
			break;
		case VK_RIGHT:
			mrect.dx = 200;
			break;
		case VK_UP:
			mrect.dy = -200;
			break;
		case VK_DOWN:
			mrect.dy = 200;
			break;
		case VK_SPACE:
			mrect.dx = randInt();
			mrect.dy = randInt();
			break;
		case VK_CONTROL:
			showImage = !showImage;
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
			mrect.dx = 0;
			break;
		case VK_UP:
		case VK_DOWN:
			mrect.dy = 0;
			break;
		default:
			break;
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rect;
		HDC hdc, hCmpDC;
		HBITMAP hBmp, oldBitmap;

		GetClientRect(hWnd, &rect);
		hdc = BeginPaint(hWnd, &ps);
		hCmpDC = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
		oldBitmap = (HBITMAP)SelectObject(hCmpDC, hBmp);

		repaint(hCmpDC, rect);

		SetStretchBltMode(hdc, COLORONCOLOR);
		BitBlt(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hCmpDC, rect.left, rect.top, SRCCOPY);

		DeleteDC(hCmpDC);
		DeleteObject(hBmp);
		DeleteObject(oldBitmap);

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void repaint(HDC hdc, RECT clientRect) {
	FillRect(hdc, &clientRect, bgBrush);

	RECT rect = ToRECT(mrect);
	FillRect(hdc, &rect, mrect.brush);

	if (showImage) {
		HDC hCmpDC = CreateCompatibleDC(hdc);
		HGDIOBJ old = SelectObject(hCmpDC, image);
		SetStretchBltMode(hCmpDC, STRETCH_HALFTONE);
		StretchBlt(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			hCmpDC, 0, 0, 512, 603, SRCCOPY);
		//BitBlt(hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hCmpDC, 0, 0, SRCCOPY);
		SelectObject(hCmpDC, old);
		DeleteDC(hCmpDC);
	}
}

void update(DOUBLE deltaTime) {

	RECT rect;
	GetClientRect(hWnd, &rect);
	InvalidateRect(hWnd, &rect, FALSE);

	mrect.x += mrect.dx * deltaTime;
	mrect.y += mrect.dy * deltaTime;

	if (mrect.x < rect.left) {
		mrect.x = rect.left;
		mrect.dx = -mrect.dx;
	}
	else if (mrect.x + mrect.width > rect.right) {
		mrect.x = (DOUBLE)rect.right - mrect.width;
		mrect.dx = -mrect.dx;
	}
	if (mrect.y < rect.top) {
		mrect.y = rect.top;
		mrect.dy = -mrect.dy;
	}
	else if (mrect.y + mrect.height > rect.bottom) {
		mrect.y = (DOUBLE)rect.bottom - mrect.height;
		mrect.dy = -mrect.dy;
	}
}