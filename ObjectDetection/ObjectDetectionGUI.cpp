#include "ObjectDetectionGUI.h"
#include <QTime>
#include <QMetaType>
#include <ctime>
using namespace cv;
ObjectDetectionGUI::ObjectDetectionGUI(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->Detector = new DetectorWithQt("test.engine");
	this->DetectorThread = new QThread();
	this->Detector->moveToThread(this->DetectorThread);
	this->DetectorThread->start();

	this->Camera = new UVCCamera(this);

	QObject::connect(this, &ObjectDetectionGUI::TriggerDetect, this->Detector, &DetectorWithQt::AsynDetect,Qt::QueuedConnection);
	QObject::connect(this->Detector, &DetectorWithQt::Detected, this, &ObjectDetectionGUI::Detected);

	QObject::connect(this->ui.pushButton, &QPushButton::clicked, [this](){
		//this->Camera->StartCamera("C:/Users/ZhouZishun/Videos/288840339/1/1.mp4");
		this->Camera->StartCamera("0");
	});

	QObject::connect(this->Camera, &UVCCamera::CameraStarted, [this]() {
		cv::Mat frame;
		while (frame.empty())
		{
			this->Camera->ReadFromBuffer(frame);
		}
		clock_t stamp = clock();
		emit this->TriggerDetect(frame, stamp);
	});
	
}


void ObjectDetectionGUI::Detected(std::vector<Yolo::Detection> result, int Timestamp)
{
	clock_t detecttime = clock();
	cv::Mat frame;
	this->Camera->ReadFromBuffer(frame);
	
	clock_t stamp = clock();
	emit this->TriggerDetect(frame, stamp);
	cv::Mat paint;
	frame.copyTo(paint);
	this->Detector->DrawRectangle(paint, result);
	this->ui.label->LoadPictureAsyn(paint);

	int interval = detecttime - Timestamp;
	this->ui.label_2->setText("\t"+QString::number(interval) + "ms");
}