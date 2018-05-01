#include "stdafx.h"
#include "Capture.h"
#include <chrono>

Capture::Capture()
{
	bitmapInfo_ = { 0 };
	bitmapInfo_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo_.bmiHeader.biPlanes = 1;
	bitmapInfo_.bmiHeader.biCompression = BI_RGB;
}

Capture::~Capture()
{
	delete window;
	captureCount_ = 0;
	frame = {};
	bitmapInfo_ = {};
	SelectObject(hMemory_, hbitmapOLD_);
	DeleteObject(hbitmap_);
	DeleteDC(hMemory_);
}

void Capture::Initialize(WindowDevice *windowDC, const int width, const int height)
{

	if (width < 0 || height < 0)
	{
		frame.targetHeight = frame.targetWidth = 0;
	}

	frame.targetWidth = width;
	frame.targetHeight = height;

	ChangeWindowDevice(windowDC);
}

void Capture::AlterSize(int newWidth, int newHeight)
{
	if (frame.targetWidth == newWidth && frame.targetHeight == newHeight)
	{
		return;
	}

	if (newWidth < 0 || newHeight < 0)
	{
		return;
	}

	Initialize(window, newWidth, newHeight);
}

void Capture::InitDIB()
{
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
}

void Capture::AssignHeaderInfo()
{
	bitmapInfo_.bmiHeader.biWidth = frame.targetWidth;
	bitmapInfo_.bmiHeader.biHeight = frame.targetHeight;
	bitmapInfo_.bmiHeader.biBitCount = frame.bpp;
}

void Capture::ChangeWindowDevice(WindowDevice *windowDC)
{
	if (!windowDC->IsValid())
	{
		throw std::exception("Attempting to attach an invalid WindowDevice");
	}

	window = windowDC;

	UpdateFrame();
	AssignHeaderInfo();
	InitDIB();

	hasCapturedWindow_ = false;
}

void Capture::UpdateFrame()
{
	frame.bpp = window->bpp;
	frame.stride = ((frame.targetWidth * frame.bpp + 31) / 32) * 4;
	frame.size = frame.stride * frame.targetHeight;
}

void Capture::CaptureScreen()
{
	if (!hasCapturedWindow_)
		hasCapturedWindow_ = true;

	if (frame.targetHeight == window->height && frame.targetWidth == window->width)
	{
		if (!BitBlt(
			hMemory_,
			0,
			0,
			window->width,
			window->height,
			window->hscreen,
			0,
			0,
			SRCCOPY
		))
		{
			throw std::exception("BitBlt failed");
		}
	}
	else
	{
		SetStretchBltMode(hMemory_, HALFTONE);
		if (!StretchBlt(
			hMemory_,
			0,
			0,
			frame.targetWidth,
			frame.targetHeight,
			window->hscreen,
			0,
			0,
			window->width,
			window->height,
			SRCCOPY
		))
		{
			throw std::exception("StretchBlt failed");
		}
	}

	++captureCount_;
}

int Capture::IsValid()
{
	if (!window->IsValid())
	{
		return 0;
	}

	if (!hasCapturedWindow_)
	{
		return 0;
	}

	if (!hasEncodedData_)
	{
		return 0;
	}

	return 1;
}

int Capture::SaveToDisk(std::wstring path, ImageFormat type, ULONG *quality)
{

	if (!hasCapturedWindow_)
	{
		return 0;
	}

	Gdiplus::Status stat;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID clisd;

	Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(hbitmap_, (HPALETTE)NULL);

	GetEncoderClsid(type, &clisd);

	std::wstring fileType;
	std::wstring fileName =
		L"\\Capture_WIDTH_" +
		std::to_wstring(frame.targetWidth) +
		L"_HEIGHT" +
		std::to_wstring(frame.targetHeight);

	switch (type) {
	case ImageFormat::PNG:
		fileType = L".png";
		break;
	case ImageFormat::BMP:
		fileType = L".bmp";
		break;
	case ImageFormat::JPG:
		fileType = L".jpg";

		// encoding parameters [needed for quality]
		Gdiplus::EncoderParameters encoderParameters;
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		encoderParameters.Parameter[0].Value = &quality;

		if (quality == NULL)
			quality = 0;

		fileName += L"_QUALITY_" + std::to_wstring((int)quality);
		stat = bmp->Save((path + fileName + fileType).c_str(), &clisd, &encoderParameters);

		delete bmp;
		Gdiplus::GdiplusShutdown(gdiplusToken);

		if (stat != Gdiplus::Ok) {
			throw std::exception("Encoding failed. Failed to save bmp to memory stream.");
		}

		return 1;
		break;
	}

	stat = bmp->Save((path + fileName + fileType).c_str(), &clisd, NULL);

	delete bmp;
	Gdiplus::GdiplusShutdown(gdiplusToken);

	if (stat != Gdiplus::Ok) {
		throw std::exception("Encoding failed. Failed to save bmp to memory stream.");
	}

	return 1;
}

int Capture::Encode(ImageFormat type, ULONG *quality)
{
	if (type == BMP)
	{
		return 0;
	}

	if (!hasCapturedWindow_)
	{
		return 0;
	}

	Gdiplus::Status stat;

	bufferEncoded_.resize(0);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CLSID clisd;

	IStream *stream = NULL;
	CreateStreamOnHGlobal(NULL, FALSE, (LPSTREAM*)&stream);

	Gdiplus::Bitmap *bmp = new  Gdiplus::Bitmap(hbitmap_, (HPALETTE)NULL);

	GetEncoderClsid(type, &clisd);

	switch (type)
	{
	case JPG:
		Gdiplus::EncoderParameters encoderParameters;
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		encoderParameters.Parameter[0].Value = &quality;

		stat = bmp->Save(stream, &clisd, &encoderParameters);

	case PNG:
		stat = bmp->Save(stream, &clisd, NULL);
		break;
	}

	const LARGE_INTEGER largeIntegerBeginning = { 0 };
	ULARGE_INTEGER largeIntEnd = { 0 };
	stream->Seek(largeIntegerBeginning, STREAM_SEEK_CUR, &largeIntEnd);
	int bufferSize = (ULONG)largeIntEnd.QuadPart;

	if (bufferSize == 0)
	{
		delete bmp;
		Gdiplus::GdiplusShutdown(gdiplusToken);
		throw std::exception("Encoding failed. Buffer size is equal to 0");
	}

	HGLOBAL pngInMemory;
	const HRESULT hResult = GetHGlobalFromStream(stream, &pngInMemory);

	LPVOID lpPngStreamBytes = GlobalLock(pngInMemory);

	char *charBuf = (char*)lpPngStreamBytes;

	bufferEncoded_ = std::vector<unsigned char>(charBuf, charBuf + bufferSize);

	delete bmp;
	Gdiplus::GdiplusShutdown(gdiplusToken);
	hasEncodedData_ = true;

	return 1;
}

int Capture::CaptureScreen_TEST()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	CaptureScreen();
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
}

int Capture::Encode_TEST(ImageFormat type, ULONG *quality)
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	Encode(type, quality);
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
}


// modified way of implementing encode fetching
inline int GetEncoderClsid(ImageFormat imgage_format, CLSID* pClsid)
{
	const wchar_t* format = NULL;

	switch (imgage_format)
	{
	case ImageFormat::BMP:
		format = L"image/bmp";
		break;
	case ImageFormat::JPG:
		format = L"image/jpeg";
		break;
	case ImageFormat::PNG:
		format = L"image/png";
		break;
	default:
		break;
	}

	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
