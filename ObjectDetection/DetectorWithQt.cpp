#include "DetectorWithQt.h"
#include <qdebug.h>
#include <qmutex.h>
#include <QMetaType>

DetectorWithQt::DetectorWithQt(QString FileName,QObject *parent)
	: QObject(parent),
	Detector(FileName.toStdString())
{
	qRegisterMetaType<cv::Mat>("CvMat");
}

DetectorWithQt::~DetectorWithQt()
{
}

void DetectorWithQt::AsynDetect(cv::Mat Input, int Timestamp)
{
	QMutexLocker Mutexlocker(&this->Locker);
	qDebug() << Timestamp;
	int i = 0;
	while (i<2000000000)
	{
		i++;
	}
}