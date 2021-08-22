#pragma once

#include <QtWidgets/QWidget>
#include "ui_ObjectDetectionGUI.h"
#include <qthread.h>
#include "DetectorWithQt.h"
#include <opencv2/opencv.hpp>

class ObjectDetectionGUI : public QWidget
{
    Q_OBJECT

public:
    ObjectDetectionGUI(QWidget *parent = Q_NULLPTR);

private:
    Ui::ObjectDetectionGUIClass ui;
	DetectorWithQt* Detector;
	QThread* DetectorThread;

signals:
	void TriggerDetect(cv::Mat In, int Timestamp);
};
