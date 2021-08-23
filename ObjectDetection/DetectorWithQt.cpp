#include "DetectorWithQt.h"
#include <qdebug.h>
#include <qmutex.h>
#include <QMetaType>

DetectorWithQt::DetectorWithQt(QString FileName,QObject *parent)
	: QObject(parent),
	Detector(FileName.toStdString())
{
	qRegisterMetaType<cv::Mat>("CvMat");
	qRegisterMetaType<std::vector<Yolo::Detection>>("ObjectVector");
}

DetectorWithQt::~DetectorWithQt()
{
	while (this->Locker.tryLock());
}

void DetectorWithQt::AsynDetect(cv::Mat Input, int Timestamp)
{
	QMutexLocker Mutexlocker(&this->Locker);
	std::vector<Yolo::Detection> result = this->Detect(Input);
	emit this->Detected(result, Timestamp);
}