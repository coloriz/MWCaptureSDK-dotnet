// HDMICapturer.h

#pragma once

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;

namespace InsLab
{
    namespace Capture
    {
        public ref class HDMICaptureException : public Exception
        {
        public:
            HDMICaptureException(String^ msg) : Exception(msg) { }
        };

        public ref class HDMICapturer
        {
        private:
            HCHANNEL hChannel = NULL;
            int width;
            int height;
            double fps;
            bool interlaced;
        public:
            property double Fps { double get() { return fps; } }
            property bool Interlaced { bool get() { return interlaced; } }
            HDMICapturer();
            HDMICapturer(int width, int height);
            ~HDMICapturer();
            Bitmap^ CaptureFrame();
        };
    }
}
