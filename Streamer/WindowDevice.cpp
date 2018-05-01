#include "stdafx.h"
#include "WindowDevice.h"
#include "string";

WindowDevice::WindowDevice()
{
	hwnd = GetDesktopWindow();
	GetDataFromHWND();
}

WindowDevice::WindowDevice(int screenNumber)
{
	hwnd = GetDesktopWindow();
	GetDataFromHWND(screenNumber);
}


WindowDevice::WindowDevice(const LPCWSTR windowName)
{
	hwnd = ::FindWindow(0, windowName);
	GetDataFromHWND();
}

WindowDevice::~WindowDevice()
{
	DeleteDC(hscreen);
	DeleteObject(hwnd);
}

int WindowDevice::IsValid()
{
	if (!fetchedData)
	{
		return 0;
	}

	if (width < 0 || height < 0 || bpp < 0)
	{
		return 0;
	}

	if (hscreen == NULL)
	{
		return 0;
	}

	return 1;
}

void WindowDevice::GetDataFromHWND(int screenNumber = 0)
{
	if (screenNumber == 0)
		hscreen = GetDC(hwnd);
	else
	{
		std::wstring monitor = std::wstring(L"\\\\.\\DISPLAY") + std::to_wstring(screenNumber);
		hscreen = CreateDCW(L"DISPLAY", monitor, NULL, NULL);
	}

	bpp = GetDeviceCaps(hscreen, BITSPIXEL);

	if (bpp >= 32) {
		bpp = 32;
	}
	else if (bpp >= 24 && bpp < 32) {
		bpp = 24;
	}
	else if (bpp >= 16 && bpp < 24) {
		bpp = 16;
	}
	else if (bpp >= 8 && bpp < 16) {
		bpp = 8;
	}
	else if (bpp >= 4 && bpp < 8) {
		bpp = 4;
	}
	else {
		bpp = 2;
	}

	RECT rect;
	GetWindowRect(hwnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	fetchedData = true;
}
