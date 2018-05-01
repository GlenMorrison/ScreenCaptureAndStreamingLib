#pragma once
#include "WindowDevice.h"

#include <vector>

//GDI
#include <objidl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

enum ImageFormat {
	BMP,
	JPG,
	PNG
};

inline int GetEncoderClsid(ImageFormat imgage_format, CLSID* pClsid);

class Capture {
public:
	WindowDevice * window;

	struct Frame {
		int targetWidth;
		int targetHeight;
		int bpp;
		char* pbits;
		int size;
		int stride;
	};
	Frame frame{ 0,0,0,nullptr,0,0 };

	Capture();


	/// <summary>
	/// Captures the contents of the current window device.
	/// </summary>
	Capture(WindowDevice *windowDC, const int width, const int height)
	{
		window = windowDC;

		frame.targetWidth = width;
		frame.targetHeight = height;
		frame.bpp = window->bpp;
		frame.stride = ((frame.targetWidth * frame.bpp + 31) / 32) * 4;
		frame.size = frame.stride * frame.targetHeight;

		bitmapInfo_ = { 0 };
		bitmapInfo_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo_.bmiHeader.biPlanes = 1;
		bitmapInfo_.bmiHeader.biCompression = BI_RGB;
		bitmapInfo_.bmiHeader.biWidth = frame.targetWidth;
		bitmapInfo_.bmiHeader.biHeight = frame.targetHeight;
		bitmapInfo_.bmiHeader.biBitCount = frame.bpp;

		hMemory_ = CreateCompatibleDC(window->hscreen);
		if (!hMemory_)
		{
			throw std::exception("CreateCompatibleDC failed");
		}

		hbitmap_ = CreateDIBSection(window->hscreen, &bitmapInfo_, DIB_RGB_COLORS, (void**)&frame.pbits, NULL, 0);
		if (!hbitmap_)
		{
			throw std::exception("CreateDIBSection failed");
		}

		hbitmapOLD_ = (HBITMAP)SelectObject(hMemory_, hbitmap_);
		if (!hbitmapOLD_)
		{
			throw std::exception("SelectObject failed");
		}

		hasCapturedWindow_ = false;
	}
	~Capture();


	/// <summary>
	/// Initialize the class.
	/// <param name="windowDC">Window device reference.</param>
	/// <param name="width">Desired width</param>
	/// <param name="width">Desired height</param>
	/// </summary>
	void Initialize(WindowDevice *windowDC, const int width, const int height);

	/// <summary>
	/// Encodes then formats the captured data to a location on disk.
	/// </summary>
	/// <param name="path">Path on disk (with no name specified)</param>
	/// <param name="type">ImageFormat::PNG, BMP, JPG</param>
	/// <param name="quality">Quality 1-100 as ULONG*</param>
	int SaveToDisk(std::wstring path, ImageFormat type, ULONG *quality);



	/// <summary>
	/// Encodes the captured data.
	/// </summary>
	/// <param name="type">ImageFormat::PNG, BMP, JPG</param>
	/// <param name="quality">Quality 1-100 as ULONG*</param>
	int Encode(ImageFormat type, ULONG *quality);
	int Encode_TEST(ImageFormat type, ULONG *quality);

	/// <summary>
	/// Captures the contents of the current capture reference.
	/// </summary>
	void CaptureScreen();
	int CaptureScreen_TEST();

	/// <summary>
	/// Changes the dimensions of the capture.
	/// </summary>
	/// <param name="newWidth">Alters the width</param>
	/// <param name="newHeight">Alters the height</param>
	void AlterSize(int newWidth, int newHeight);

	/// <summary>
	/// Returns a unsigned char vector containing the encoded capture data.
	/// </summary>
	std::vector<unsigned char>getEncodedBuffer() { return bufferEncoded_; }

	/// <summary>
	/// Returns total captures made.
	/// </summary>
	int GetCaptureCount() { return captureCount_; }


	/// <summary>
	/// Changes the current window device.
	/// </summary>
	void ChangeWindowDevice(WindowDevice *windowDC);

	/// <summary>
	/// Check if capture is valid.
	/// </summary>
	int IsValid();

private:
	void InitDIB();
	void AssignHeaderInfo();
	void UpdateFrame();

	int captureCount_ = 0;

	HDC hMemory_;
	HBITMAP hbitmap_;
	HBITMAP hbitmapOLD_;
	BITMAPINFO bitmapInfo_;

	std::vector<unsigned char> bufferEncoded_{};

	bool hasCapturedWindow_;
	bool hasEncodedData_;

};