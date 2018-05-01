#pragma once
#include "stdafx.h"
#include <windows.h>
#include <string>

class WindowDevice
{
public:

	/// <summary>
	/// Create a window device of current monitor.
	/// </summary>
	WindowDevice();

	/// <summary>
	/// Create a window device of specific window name.
	/// </summary>
	WindowDevice(const LPCWSTR windowName);

	~WindowDevice();

	/// <summary>
	/// Checks if window device is valid.
	/// </summary>
	int IsValid();

	HWND hwnd;
	HDC hscreen;
	int width;
	int height;
	int bpp;


private:
	void GetDataFromHWND();

	bool fetchedData;
};