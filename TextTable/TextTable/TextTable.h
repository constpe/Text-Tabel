#include <Windows.h>

#ifndef __TextTable__
#define __TextTable__

ATOM RegWindowClass(HINSTANCE hInstance, LPCTSTR lpzClassName);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool ReadTable(LPWSTR fileName);

void Redraw(HWND hWnd);

#endif