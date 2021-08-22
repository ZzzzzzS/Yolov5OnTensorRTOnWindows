#pragma once

#include <QObject>
#include "Detector.h"
#include <qmutex.h>

class DetectorWithQt : public QObject , Detector
{
	Q_OBJECT

public:
	DetectorWithQt(QString FileName,QObject *parent=Q_NULLPTR);
	~DetectorWithQt();
	QMutex Locker;

public slots:
	void AsynDetect(cv::Mat Input, int Timestamp);

signals:
	void Detected(std::vector<Yolo::Detection> result, int Timestamp);
};
