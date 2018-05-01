#pragma once

#include "windows.h"
#include "WindowDevice.h"
#include "Capture.h"
#include <vector>

/// <summary>
///  TODO - Summmary for all classes and functions
///  TODO - Sort out public and private variables and add a '_' signature to privates!
///  TODO - When swapping out the screen at run time. Make sure the two buffers are bothed altered equally
/// </summary>
class CaptureStreamer
{
public:
	CaptureStreamer();

	/// <summary>
	/// Initializes Capture Streamer.
	/// </summary>
	/// <param name="window_">The target window_.</param>
	/// <param name="targetHeight">Height of the output capture.</param>
	/// <param name="targetWidth">Width of the  output capture.</param>
	CaptureStreamer(WindowDevice *window_, int targetWidth, int targetHeight)
		:captureOne_(window_, targetWidth, targetHeight),
		captureTwo_(window_, targetWidth, targetHeight)
	{
		currentCapture_ = &captureOne_;
		previousCapture_ = &captureTwo_;
	}

	/// <summary>
	/// Initializes Capture Streamer. Automatically sets the targetWidth and targetHeight to window_ dimensions
	/// </summary>
	/// <param name="window_">The target window_.</param>
	CaptureStreamer(WindowDevice *window_)
		:captureOne_(window_, window_->width, window_->height),
		captureTwo_(window_, window_->width, window_->height)
	{
		currentCapture_ = &captureOne_;
		previousCapture_ = &captureTwo_;
	}

	~CaptureStreamer();

	/// <summary>
	/// Captures the current assigned window_.
	/// </summary>
	void CaptureScreen();

	/// <summary>
	/// Encodes the capture to a specified format.
	/// </summary>
	/// <param name="type">PNG or JPEG.</param>
	/// <param name="quality">NULL if PNG. 0-100 if JPG.</param>
	void Encode(ImageFormat type, ULONG *quality);

	/// <summary>
	/// Saves the previous capture to disk.
	/// </summary>
	/// <param name="path">The path, without the file name. File name is 'targetWidth_targetHeight_quality'.</param>
	/// <param name="type">The type.</param>
	/// <param name="quality">NULL if PNG. 0-100 if JPG.</param>
	void SavePreviousCaptureToDisk(std::wstring path, ImageFormat type, ULONG *quality);

	/// <summary>
	/// Alters the size of the output capture.
	/// </summary>
	/// <param name="newWidth">The new target width.</param>
	/// <param name="newHeight">The new target height.</param>
	void AlterSize(int newWidth, int newHeight);

	/// <summary>
	/// Gets the capture count.
	/// </summary>
	/// <returns>Returns the number of captures in this session</returns>
	int GetCaptureCount();

	/// <summary>
	/// Determines whether [has capture changed].
	/// </summary>
	/// <returns>
	///   <c>true</c> if [has capture changed]; otherwise, <c>false</c>.
	/// </returns>
	bool HasCaptureChanged();

	/// <summary>
	/// Gets the encoded data.
	/// </summary>
	/// <returns>Returns the current held encoded data of current capture.</returns>
	std::vector<unsigned char> GetEncodedData();

private:

	Capture captureOne_, captureTwo_;
	Capture *currentCapture_, *previousCapture_;

	int captureCount_ = 0;

	WindowDevice *window_;

	void SwapBuffers();
};