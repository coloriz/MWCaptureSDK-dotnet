# Magewell Capture SDK .NET Wrapper

written in C++/CLR.

### Example (C#)

```csharp
using System;
using System.Collections.Concurrent;
using System.Drawing;
using System.Threading;
using InsLab.Capture;

class HDMICapturerExample
{
    static void Main(string[] args)
    {
        // https://gist.github.com/coloriz/7ed97952d6cd72b3733ebcfd9170019f
        // 20000 us = 20 ms = 50 fps
        var timer = new MicroTimer(20000);
        var q = new ConcurrentQueue<Bitmap>();
        
        var cap = new HDMICapturer();
        Console.WriteLine($"Fps: {cap.Fps}");
        Console.WriteLine($"Interlaced: {cap.Interlaced}");

        timer.MicroTimerElapsed += (o, args) =>
        {
            Bitmap bitmap = cap.CaptureFrame();
            q.Enqueue(bitmap);
        }
        timer.Start();

        // Wait for 5s
        Thread.Sleep(5000);
        timer.Stop();

        Console.WriteLine($"Total {q.Count} frames captured.");
    }
}
```
