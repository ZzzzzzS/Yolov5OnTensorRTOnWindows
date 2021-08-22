#include "ObjectDetectionGUI.h"
#include <QTime>
#include <QMetaType>
ObjectDetectionGUI::ObjectDetectionGUI(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->Detector = new DetectorWithQt("test.engine");
	this->DetectorThread = new QThread();
	//this->Detector->moveToThread(this->DetectorThread);
	this->DetectorThread->start();
	qRegisterMetaType<cv::Mat>("CvMat");
	QObject::connect(this, &ObjectDetectionGUI::TriggerDetect, this->Detector, &DetectorWithQt::AsynDetect,Qt::QueuedConnection);

	QObject::connect(this->ui.pushButton, &QPushButton::clicked,[this]() {
		cv::Mat a;
		emit this->TriggerDetect(a, QTime::currentTime().second());
	});
	cv::Mat a;
	emit this->TriggerDetect(a, QTime::currentTime().second());
}
