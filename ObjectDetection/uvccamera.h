/****************************************************************************
MIT License

Copyright (c) 2020 ZhouZishun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#ifndef UVCCAMERA_H
#define UVCCAMERA_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QTime>
#include <QPixmap>
#include <qmutex.h>
#include <qthread.h>

class UVCCamera : public QThread
{
    Q_OBJECT
public:
    explicit UVCCamera(QObject *parent = nullptr);
    ~UVCCamera();
    void StopCamera();
    void StartRecording(QString dir);
    void StopRecording();

	bool ReadFromBuffer(cv::Mat& Mat);

signals:
    //void RenewImage(QPixmap image);
    void CameraStarted();
    void CameraStopped();
    void CameraStartFailed();
public slots:
    void StartCamera(QString name);

private:
    cv::VideoCapture *Capture;
    cv::VideoWriter *recorder;
    cv::Size CaptureSize;
    int FPS;
    cv::Mat CaptureBuffer;
    bool isCapturing;
    bool isRecording;
    void CameraLoop();
    //QImage cvMat2QImage(const cv::Mat& mat);

	enum CaptureBuffer_e
	{
		Buffer_A,
		Buffer_B
	};
	CaptureBuffer_e CurrentCapturingBuffer;
	cv::Mat BufferA;
	cv::Mat BufferB;
	QMutex BufferALock;
	QMutex BufferBLock;

protected:
	void run();
};

#endif // UVCCAMERA_H
