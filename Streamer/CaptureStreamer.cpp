#include "stdafx.h"
#include "CaptureStreamer.h"


CaptureStreamer::CaptureStreamer()
{

}

CaptureStreamer::~CaptureStreamer()
{
	delete currentCapture_;
	delete previousCapture_;
	delete window_;
}

void CaptureStreamer::CaptureScreen()
{
	currentCapture_->CaptureScreen();
}

bool CaptureStreamer::HasCaptureChanged()
{
	if (GetCaptureCount() <= 1)
	{
		// can't compare if only one capture has occured
		return true;
	}
	else
	{
		std::vector<unsigned char> bufferOne(currentCapture_->frame.pbits + 40, currentCapture_->frame.pbits + currentCapture_->frame.size);
		std::vector<unsigned char> bufferTwo(previousCapture_->frame.pbits + 40, previousCapture_->frame.pbits + previousCapture_->frame.size);

		return (bufferOne != bufferTwo);
	}
}

void CaptureStreamer::SavePreviousCaptureToDisk(std::wstring path, ImageFormat type, ULONG *quality)
{
	currentCapture_->SaveToDisk(path, type, quality);
}

void CaptureStreamer::Encode(ImageFormat type, ULONG *quality)
{
	currentCapture_->Encode(type, quality);
}

std::vector<unsigned char> CaptureStreamer::GetEncodedData()
{
	if (!currentCapture_->IsValid())
	{
		if (!previousCapture_->IsValid())
		{
			return previousCapture_->getEncodedBuffer();
		}
		else
		{
			throw std::exception("No encoded buffers are valid");
		}
	}

	SwapBuffers();
	return previousCapture_->getEncodedBuffer();
}

int CaptureStreamer::GetCaptureCount()
{
	return currentCapture_->GetCaptureCount() + previousCapture_->GetCaptureCount();
}

void CaptureStreamer::AlterSize(int newWidth, int newHeight)
{
	captureOne_.AlterSize(newWidth, newHeight);
	captureTwo_.AlterSize(newWidth, newHeight);
}

void CaptureStreamer::SwapBuffers()
{
	if (currentCapture_ == &captureOne_) {
		currentCapture_ = &captureTwo_;
		previousCapture_ = &captureOne_;
		return;
	}

	currentCapture_ = &captureOne_;
	previousCapture_ = &captureTwo_;
}

