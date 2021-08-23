/*
 * @Author: Zhou Zishun
 * @Date: 2021-04-07 20:40:48
 * @LastEditors: Zhou Zishun
 * @LastEditTime: 2021-04-14 21:27:33
 * @Description: file content
 */

#pragma once
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QResizeEvent>
#include <QPoint>
#include <QRect>
#include <QCursor>
#include <QRunnable>
#include <QThreadPool>
#include <QMutex>
#include <QMutexLocker>
#include <QPen>
#include <QPainter>
#include <QClipboard>
#include <QVector>
#include <QPolygon>

#include <opencv2/opencv.hpp>

#define SCROLL_SCALE 0.1
#define MIN_SELECTED_AREA 30
#define MOVE_SCALE 1

class ScrollScale : public QLabel
{
    Q_OBJECT
public:
    ScrollScale(QWidget *parents = Q_NULLPTR);
    ~ScrollScale();

    void LoadPicture(QPixmap &Picture);
    void LoadPicture(QImage &Picture);
    void LoadPictureAsyn(cv::Mat &Picture);
	void LoadPictureAsyn(QPixmap &Picture);

	//在已经开始刷图后慎用这个函数
	void UpdateSyncThreadPool(QThreadPool* ThreadPool);
	void SetDefaultText(QString Text=tr("No Picture"));

	bool AllowPopOut; //不允许弹出新窗口

	QPoint MousePosition;

signals:
	void MousePositionChanged(QPoint P);
	void DrawRectangleSignal(QPoint Begin, QPoint End);
	void DrawCircleSignal(QPoint Center, int Radius);
	void DrawPointsSignal(std::vector<QPoint> Points);
	void DrawPointsFinishedSignal(std::vector<std::vector<QPoint>> PointArray);
	void DrawSinglePointSignal(QPoint Point);

private:
	void setTranslation();
private:

    QPixmap OriginalPicture;
    QPixmap ScrolledPicture;

    class MyRect
    {
    public:
        void setX(int a)
        {
            X = a;
        }
        void setY(int a)
        {
            Y = a;
        }
        void setWidth(int a)
        {
            Width = a;
        }
        void setHeight(int a)
        {
            Height = a;
        }
        int x()
        {
            return X;
        }
        int y()
        {
            return Y;
        }
        int width()
        {
            return Width;
        }
        int height()
        {
            return Height;
        }

    private:
        int X, Y, Width, Height;
    };

    MyRect SelectedSize;
    double Ratio;

    QPoint ClickedPoint;
	bool isMouseClicked;

	QPoint ClickedPointOnScreen;
	QPoint MovePointOnScreen;

	QString DefaultText;

private:
    void SelectPicture();
	void MouseMoveNormal(QMouseEvent *event);

	QPoint OnScreenToOnPicture(QPoint In);
	QPoint OnPictureToOnImage(QPoint In);

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseReleaseEvent(QMouseEvent *ev);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);

private:
    class SetPixmapAsyn : public QRunnable
    {
    public:
        SetPixmapAsyn(QObject *Obj, QThreadPool *ThreadPool);
        ~SetPixmapAsyn();

        void StartProcess(QPixmap InputImage, QSize WindowSize, MyRect SelectedSize, bool Cut = false);
		void StartProcess(cv::Mat InputImage, QSize WindowSize, MyRect SelectedSize, bool Cut = false);

    private:
        void run();
        QObject *obj;
        QMutex lock;
        QThreadPool *ThreadPool;

		QPixmap InputImage;
		cv::Mat InputImageCV;
		QSize WindowSize;
		MyRect SelectedSize;
		bool Cut;
		bool isMatType;
    };

	SetPixmapAsyn* AsynProcess;
    Q_INVOKABLE void DisplayPicture(QPixmap Image);
	Q_INVOKABLE void UpdateOriginalPicture(QPixmap Image);
	Q_INVOKABLE void UpdateScrolledPicture(QPixmap Image);

private:
	void InitContextMenu();
	QAction* SaveAction;
	QAction* DrawCircleAction;
	QAction* DrawSquareAction;
	QAction* DrawAnyAction;
	QAction* DrawSinglePointAction;
	QAction* CopyToClipboardAction;
	QAction* ClearScreenAction;
	
	QClipboard* PictureClipboard;
	

	enum DrawStatus_e
	{
		NoDrawing, //普通状态

		DrawingRectangle,  //正在画画的状态
		DrawingCircle,
		DrawingPoints,
		DrawingSinglePoint,

		WillDrawRectangle, //鼠标点了画画模式，但是还没开始画的状态
		WillDrawCircle,
		WillDrawPoints,
		WillDrawSinglePoint
	};

	DrawStatus_e DrawStatus;
	QPen DrawingPen;
	QPainter DrawingPainter;

	QPolygon DrawingPointGroup;
	QVector<QPolygon> DrawingPointArray;
	std::vector<std::vector<QPoint>> DrawingPointsOnPictureArray;
};
