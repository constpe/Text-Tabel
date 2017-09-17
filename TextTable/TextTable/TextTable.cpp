#include <windowsx.h>
#include "TextTable.h"
#include <fstream>
#include <vector>
#include <string>

int areaWidth, areaHeight;
std::vector<std::vector<std::string>> table;
HMENU hMenu = CreateMenu();
HMENU hFilePopupMenu = CreatePopupMenu();
OPENFILENAME file;
wchar_t fileName[255] = TEXT("");
bool isTableSet = false;

void FormMenu(HWND hWnd)
{
	AppendMenu(hFilePopupMenu, MFT_RADIOCHECK, 0, TEXT("Open"));

	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hFilePopupMenu, TEXT("File"));

	SetMenu(hWnd, hMenu);
}

ATOM RegWindowClass(HINSTANCE hInstance, LPCTSTR lpzClassName)
{
	WNDCLASS wcWindowClass = { 0 };

	wcWindowClass.lpfnWndProc = WndProc;
	wcWindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcWindowClass.hInstance = hInstance;
	wcWindowClass.lpszClassName = lpzClassName;
	wcWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcWindowClass.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;

	return RegisterClass(&wcWindowClass);
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

std::wstring ToWideStringConvert(std::string s)
{
	int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.length() + 1, 0, 0);
	wchar_t* wideS = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.length() + 1, wideS, len);
	std::wstring res(wideS);
	delete[] wideS;
	return res;
}

void DrawTable(HDC hdc)
{
	int columnCount = table[0].size();
	int columnWidth = (areaWidth - 20) / columnCount;
	int ySlip = 10;
	for (int i = 0; i < table.size(); i++)
	{
		DrawLine(hdc, 10, ySlip, areaWidth - 10, ySlip);
		int prevSlip = 0;
		for (int j = 0; j < table[i].size(); j++)
		{
			int recordHeight = 2;
			std::wstring currentLine;
			std::wstring text = ToWideStringConvert(table[i][j].c_str());
			int currentTextLength = 0;
			for (int k = 0; k < text.length(); k++)
			{
				SIZE size;
				std::wstring symbol;
				symbol = text.at(k);
				GetTextExtentPoint(hdc, symbol.c_str(), 1, &size);
				if (currentTextLength + size.cx > columnWidth - 4)
				{
					TextOut(hdc, 12 + columnWidth * j, ySlip + recordHeight, currentLine.c_str(), currentLine.length());
					recordHeight += size.cy;
					if (k == text.length() - 1 && size.cx <= columnWidth - 4)
					{
						TextOut(hdc, 12 + columnWidth * j, ySlip + recordHeight, symbol.c_str(), symbol.length());
						recordHeight += size.cy;
					}
					else
					{
						currentLine.clear();
						currentLine = symbol;
						currentTextLength = size.cx;
					}
				}
				else if (k == text.length() - 1)
				{
					currentLine += symbol;
					TextOut(hdc, 12 + columnWidth * j, ySlip + recordHeight, currentLine.c_str(), currentLine.length());
					recordHeight += size.cy;
				}
				else
				{
					currentTextLength += size.cx;
					currentLine += symbol;
				}
			}

			if (recordHeight + 2 > prevSlip)
				prevSlip = recordHeight + 2;
		}
		ySlip += prevSlip;
	}
	DrawLine(hdc, 10, ySlip, areaWidth - 10, ySlip);

	for (int j = 0; j <= columnCount; j++)
	{
		DrawLine(hdc, 10 + columnWidth * j, 10, 10 + columnWidth * j, ySlip);
	}

	/*
	DrawLine(hdc, 10, 10, areaWidth - 10, 10);
	DrawLine(hdc, 10, 50, areaWidth - 10, 50);
	DrawLine(hdc, 10, 90, areaWidth - 10, 90);
	*/
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = GetDC(hWnd);

	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);
		SetBkColor(hdc, RGB(244, 247, 252));
		if (isTableSet)
			DrawTable(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 0:
			file.lpstrTitle = TEXT("Open table from file");
			file.Flags = OFN_HIDEREADONLY;
			if (!GetOpenFileName(&file))
				return 1;
			if (ReadTable(file.lpstrFile))
			{
				isTableSet = true;
				RECT clearRect;
				clearRect.left = 0;
				clearRect.top = 0;
				clearRect.right = areaWidth;
				clearRect.bottom = areaHeight;
				InvalidateRect(hWnd, &clearRect, 1);
				UpdateWindow(hWnd);
				ValidateRect(hWnd, &clearRect);
			}
			break;
		}
		break;
	case WM_SIZE:
		areaWidth = LOWORD(lParam);
		areaHeight = HIWORD(lParam);
		ReleaseDC(hWnd, hdc);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		ReleaseDC(hWnd, hdc);
		break;
	default:
		ReleaseDC(hWnd, hdc);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool ReadTable(LPWSTR fileName)
{
	table.clear();

	std::string record;
	std::vector<std::string> row;
	std::ifstream in;

	in.open(fileName);
	while (!in.eof())
	{
		wchar_t c = in.get();
		if (c != ';' && (c > 32 || c == ' '))
		{
			record += c;
		}
		else if (c == ';')
		{
			row.insert(row.end(), record);
			record = "";
		}
		else if (c == '\n'  && row.size() != 0)
		{
			row.insert(row.end(), record);
			record = "";
			table.insert(table.end(), row);
			row.clear();
		}
	}

	if (row.size() != 0)
	{
		table.insert(table.end(), row);
	}

	if (table.size() != 0)
		return true;
	else
		return false;
}

void InitOpenFileStruct(HINSTANCE hInstance)
{
	file.lStructSize = sizeof(OPENFILENAME);
	file.hInstance = hInstance;
	file.lpstrFilter = TEXT("Text\0*.txt\0CSV Table\0*.csv");
	file.lpstrFile = fileName;
	file.nMaxFile = 256;
	file.lpstrInitialDir = TEXT(".\\");
	file.lpstrDefExt = TEXT("txt");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR nCmdLine, int cmdShow)
{	
	LPCTSTR lpzClassName = TEXT("MyWindowClass");

	if (!RegWindowClass(hInstance, lpzClassName))
		return 1;

	HWND hWnd = CreateWindow(lpzClassName, TEXT("TextTable"), WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU, 20, 20, 50, 600, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return 2;

	FormMenu(hWnd);
	InitOpenFileStruct(hInstance);
	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);
	
	MSG msg;
	int iGetOk = 0;
	while ((iGetOk = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (iGetOk == -1)
			return 3;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}