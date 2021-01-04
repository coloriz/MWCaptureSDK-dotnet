#include "stdafx.h"
#include "HDMICapturer.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;

InsLab::Capture::HDMICapturer::HDMICapturer()
{
    MWCaptureInitInstance();

    int channelCount = MWGetChannelCount();
    int proDevCount = 0;
    int proDevChannel[32] = { -1 };
    for (int i = 0; i < channelCount; i++)
    {
        MWCAP_CHANNEL_INFO info;
        MW_RESULT mr = MWGetChannelInfoByIndex(i, &info);
        if (strcmp(info.szFamilyName, "Pro Capture") == 0)
        {
            proDevChannel[proDevCount] = i;
            proDevCount++;
        }
    }

    if (proDevCount <= 0)
    {
        throw gcnew HDMICaptureException("Can't find channels!");
    }

    MWCAP_CHANNEL_INFO videoInfo;
    if (MWGetChannelInfoByIndex(proDevChannel[0], &videoInfo) != MW_SUCCEEDED)
    {
        throw gcnew HDMICaptureException("Can't get channel info!");
    }

    int devIndex = 0;

    // Open channel
    WCHAR path[128];
    MWGetDevicePath(proDevChannel[devIndex], path);
    hChannel = MWOpenChannelByPath(path);
    if (!hChannel)
    {
        throw gcnew HDMICaptureException(String::Format("Open channel {0} error!\n", devIndex));
    }

    // Get channel info
    if (MWGetChannelInfo(hChannel, &videoInfo) != MW_SUCCEEDED)
    {
        throw gcnew HDMICaptureException("Can't get channel info!");
    }

    // Capture frames on input signal frequency
    HANDLE hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Get video information from the stream
    if (MWStartVideoCapture(hChannel, hCaptureEvent) != MW_SUCCEEDED)
    {
        throw gcnew HDMICaptureException("Can't open video capture!");
    }

    MWCAP_VIDEO_BUFFER_INFO videoBufferInfo;
    MWGetVideoBufferInfo(hChannel, &videoBufferInfo);

    MWCAP_VIDEO_FRAME_INFO videoFrameInfo;
    MWGetVideoFrameInfo(hChannel, videoBufferInfo.iNewestBufferedFullFrame, &videoFrameInfo);

    MWCAP_VIDEO_SIGNAL_STATUS videoSignalStatus;
    MWGetVideoSignalStatus(hChannel, &videoSignalStatus);

    if (videoSignalStatus.state != MWCAP_VIDEO_SIGNAL_LOCKED)
    {
        throw gcnew HDMICaptureException("Video signal is not locked!");
    }

    width = videoSignalStatus.cx;
    height = videoSignalStatus.cy;
    fps = (videoSignalStatus.bInterlaced == TRUE) ? (double)20000000LL / videoSignalStatus.dwFrameDuration : (double)10000000LL / videoSignalStatus.dwFrameDuration;
    interlaced = videoSignalStatus.bInterlaced;

    MWStopVideoCapture(hChannel);
    CloseHandle(hCaptureEvent);
}

InsLab::Capture::HDMICapturer::HDMICapturer(int width, int height) : HDMICapturer()
{
    this->width = width;
    this->height = height;
}

InsLab::Capture::HDMICapturer::~HDMICapturer()
{
    if (hChannel != NULL)
    {
        MWCloseChannel(hChannel);
    }
    MWCaptureExitInstance();
}

Bitmap^ InsLab::Capture::HDMICapturer::CaptureFrame()
{
    Bitmap^ bitmap = gcnew Bitmap(width, height, PixelFormat::Format24bppRgb);
    BitmapData^ bmpData = bitmap->LockBits(Drawing::Rectangle(0, 0, width, height), ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);

    HANDLE hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Start video capture
    if (MWStartVideoCapture(hChannel, hCaptureEvent) != MW_SUCCEEDED)
    {
        throw gcnew HDMICaptureException("Can't open video capture!");
    }

    MWCaptureVideoFrameToVirtualAddress(hChannel, -1, (LPBYTE)bmpData->Scan0.ToPointer(), bmpData->Stride * height, bmpData->Stride, FALSE, NULL, MWFOURCC_BGR24, width, height);
    WaitForSingleObject(hCaptureEvent, INFINITE);

    bitmap->UnlockBits(bmpData);

    MWStopVideoCapture(hChannel);
    CloseHandle(hCaptureEvent);

    return bitmap;
}
