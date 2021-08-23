/*
 * @Author: Zhou Zishun
 * @Date: 2021-04-07 20:41:08
 * @LastEditors: Zhou Zishun
 * @LastEditTime: 2021-04-11 12:50:32
 * @Description: file content
 */

#include "ScrollScale.h"
#include <QLabel>
#include <QDebug>
#include <opencv.hpp>
#include<opencv2/imgproc/types_c.h>
#include <qtooltip.h>
#include <qaction.h>
#include <qfiledialog.h>
#include <qurl.h>
#include <qpainter.h>
#include <QtWidgets/QApplication>
#include <QDrag>
#include <qmimedata.h>
#include <qpainterpath.h>
#include <qtranslator.h>
#include <qmessagebox.h>

ScrollScale::ScrollScale(QWidget *parents)
    : QLabel(parents)
{
	setTranslation();
	SetDefaultText();
    this->setAlignment(Qt::AlignCenter);
    this->setCursor(QCursor(Qt::CrossCursor));
	this->AsynProcess = new SetPixmapAsyn(this, QThreadPool::globalInstance());
	this->AllowPopOut = true;
	this->isMouseClicked = false;
	this->setMouseTracking(true);
	this->MousePosition.setX(0);
	this->MousePosition.setY(0);

	this->InitContextMenu();
	this->DrawStatus = NoDrawing;
	DrawingPen.setColor(QColor(Qt::red));
	DrawingPen.setWidth(3);

	this->PictureClipboard = qApp->clipboard();
	this->setAcceptDrops(true);
	
}

ScrollScale::~ScrollScale()
{
	delete this->AsynProcess;
}

void ScrollScale::setTranslation()
{
	QTranslator* Trans = new QTranslator(this);
	QLocale local;
	qDebug() << local.language();
	if (local.language() == QLocale::Chinese)
	{
		//Trans->load("./translation/scrollzoomwidget_zh.qm");

		qApp->installTranslator(Trans);
	}
	else
	{
		//Trans->load("./translation/scrollzoomwidget_zh.qm");
		qApp->installTranslator(Trans);
	}
}

void ScrollScale::SelectPicture()
{
    this->ScrolledPicture = this->OriginalPicture.copy(this->SelectedSize.x(), SelectedSize.y(), SelectedSize.width(), SelectedSize.height());
    this->setPixmap(this->ScrolledPicture.scaled(this->size(), Qt::KeepAspectRatio));
    //qDebug() << "x=" << SelectedSize.x() << "y=" << SelectedSize.y();
    //qDebug() << "rx=" << SelectedSize.width() << "ry=" << SelectedSize.height();
	//qDebug() << "SetRatio=" << Ratio << "Now Ratio=" << (double)SelectedSize.height() / SelectedSize.width();
}

void ScrollScale::MouseMoveNormal(QMouseEvent * event)
{
	int width = this->ClickedPoint.x() - event->x() * (double)this->ScrolledPicture.width() / this->pixmap()->width();
	int height = this->ClickedPoint.y() - event->y() * (double)this->ScrolledPicture.height() / this->pixmap()->height();

	if ((width + this->SelectedSize.width()) > OriginalPicture.width())
		width = OriginalPicture.width() - this->SelectedSize.width();

	if ((height + this->SelectedSize.height()) > OriginalPicture.height())
		height = OriginalPicture.height() - this->SelectedSize.height();

	if (width < 0)
		width = 0;
	if (height < 0)
		height = 0;

	this->SelectedSize.setX(width);
	this->SelectedSize.setY(height);

	this->SelectPicture();
}

QPoint ScrollScale::OnScreenToOnPicture(QPoint event)
{
	int x, y;
	if ((double)this->height() / this->width() > Ratio) //竖着的情况
	{
		x = event.x();
		y = event.y() - (this->height() - this->pixmap()->height()) / 2;
	}
	else //横着的情况
	{
		x = event.x() - (this->width() - this->pixmap()->width()) / 2;
		y = event.y();
	}

	return QPoint(x, y);
}

QPoint ScrollScale::OnPictureToOnImage(QPoint In)
{
	double RatioX = In.x() * SelectedSize.width() / this->pixmap()->width();
	double RatioY = In.y() * SelectedSize.height() / this->pixmap()->height();

	return QPoint(SelectedSize.x() + RatioX, SelectedSize.y() + RatioY);
}

void ScrollScale::LoadPicture(QPixmap &Picture)
{
    this->OriginalPicture = Picture;
    this->ScrolledPicture = this->OriginalPicture;
    this->setPixmap(Picture.scaled(this->size(), Qt::KeepAspectRatio));
    this->SelectedSize.setX(0);
    this->SelectedSize.setY(0);
    this->SelectedSize.setWidth(this->ScrolledPicture.width());
    this->SelectedSize.setHeight(this->ScrolledPicture.height());
    this->Ratio = (double)(this->ScrolledPicture.height()) / (double)(this->ScrolledPicture.width());
}

void ScrollScale::LoadPicture(QImage &Picture)
{
    this->LoadPicture(QPixmap::fromImage(Picture));
}

void ScrollScale::LoadPictureAsyn(cv::Mat & Picture)
{
	if (Picture.empty())
	{
		QPixmap a;
		this->OriginalPicture = a;
		this->ScrolledPicture = a; //清空
		this->setText(this->DefaultText);
		return;
	}

	//行列数都相等则是老图
	if ((Picture.rows==OriginalPicture.height()) && (Picture.cols==OriginalPicture.width()))
	{
		this->AsynProcess->StartProcess(Picture, this->size(), this->SelectedSize, true);//旧图可以剪切
	}
	else
	{
		this->SelectedSize.setX(0);
		this->SelectedSize.setY(0);
		this->SelectedSize.setWidth(Picture.cols);
		this->SelectedSize.setHeight(Picture.rows);
		this->Ratio = (double)(Picture.rows) / (double)(Picture.cols);

		this->AsynProcess->StartProcess(Picture, this->size(), this->SelectedSize, false); //新图不能剪切
	}
	
}

void ScrollScale::LoadPictureAsyn(QPixmap & Picture)
{
	this->OriginalPicture = Picture;

	if (Picture.isNull())
	{
		this->ScrolledPicture = Picture;
		this->setText(this->DefaultText);
		return;
	}

	if (Picture.size() == this->OriginalPicture.size()) //分辨率不相等，证明载入新图
	{
		this->AsynProcess->StartProcess(Picture, this->size(), this->SelectedSize, true);//旧图可以剪切
	}
	else
	{
		this->SelectedSize.setX(0);
		this->SelectedSize.setY(0);
		this->SelectedSize.setWidth(Picture.width());
		this->SelectedSize.setHeight(Picture.height());
		this->Ratio = (double)(Picture.height()) / (double)(Picture.width());

		this->AsynProcess->StartProcess(Picture, this->size(), this->SelectedSize, false);//新图不能剪切
	}
}

void ScrollScale::UpdateSyncThreadPool(QThreadPool * ThreadPool)
{
	delete this->AsynProcess;
	this->AsynProcess = new SetPixmapAsyn(this, ThreadPool);
}

void ScrollScale::SetDefaultText(QString Text)
{
	this->DefaultText = Text;
}

void ScrollScale::resizeEvent(QResizeEvent *event)
{
    if (this->ScrolledPicture.isNull())
        this->setText(this->DefaultText);
    else
        this->setPixmap(this->ScrolledPicture.scaled(event->size(), Qt::KeepAspectRatio));
}

void ScrollScale::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		return;

    if (this->ScrolledPicture.isNull())
        return;

    this->ClickedPoint.setX(this->SelectedSize.x() + event->x() * (double)this->ScrolledPicture.width() / this->pixmap()->width());
    this->ClickedPoint.setY(this->SelectedSize.y() + event->y() * (double)this->ScrolledPicture.height() / this->pixmap()->height());

    this->SelectPicture();
	this->isMouseClicked = true;

	this->ClickedPointOnScreen.setX(event->x());
	this->ClickedPointOnScreen.setY(event->y());
	this->MovePointOnScreen.setX(0);
	this->MovePointOnScreen.setY(0);

	switch (this->DrawStatus)
	{
	case WillDrawCircle:
		DrawStatus = DrawingCircle;
		break;
	case WillDrawPoints:
		DrawStatus = DrawingPoints;
		break;
	case WillDrawRectangle:
		DrawStatus = DrawingRectangle;
		break;
	case WillDrawSinglePoint:
		DrawStatus = DrawingSinglePoint;
		break;

	default:
		this->setCursor(QCursor(Qt::ClosedHandCursor));
		DrawStatus = NoDrawing;
		break;
	}
}

void ScrollScale::mouseMoveEvent(QMouseEvent *event)
{

    if (this->ScrolledPicture.isNull())
        return;

	if (isMouseClicked) //鼠标被按下时执行拖拽逻辑
	{
		this->MovePointOnScreen.rx() = (event->x() - this->ClickedPointOnScreen.x());
		this->MovePointOnScreen.ry() = (event->y() - this->ClickedPointOnScreen.y());

		switch (DrawStatus)
		{
		case DrawingRectangle:
			this->update();
			break;
		case DrawingCircle:
			this->update();
			break;
		case DrawingPoints:
			this->DrawingPointGroup << event->pos();
			this->update();
			break;
		case DrawingSinglePoint:
			break;
		default:
			this->MouseMoveNormal(event);
			break;
		}
	}
	else
	{
		QPoint PositionOnPicture = this->OnScreenToOnPicture(event->pos());

		QImage image = this->pixmap()->toImage();
		if (!image.valid(PositionOnPicture)) //更新前先验证位置合法性
			return;

		QPoint OnImage = this->OnPictureToOnImage(PositionOnPicture);

		if ((MousePosition.x() != OnImage.x()) || (MousePosition.y() != OnImage.y()))
		{
			MousePosition.setX(OnImage.x());
			MousePosition.setY(OnImage.y());

			emit MousePositionChanged(MousePosition);
		}

		//qDebug() << "x size" << RatioX << "y size" << RatioY;

		if (!AllowPopOut) //在弹出的窗口中才启用这个功能
		{
			QRgb PixalColor = image.pixel(PositionOnPicture.x(), PositionOnPicture.y());
			//qDebug() << "R=" << qRed(PixalColor) << "G=" << qGreen(PixalColor) << "B=" << qBlue(PixalColor);
			QString OutString = "X=" + QString::number(MousePosition.x()) + "\nY=" + QString::number(MousePosition.y()) + "\nR=" + QString::number(qRed(PixalColor)) + "\nG=" + QString::number(qGreen(PixalColor)) + "\nB=" + QString::number(qBlue(PixalColor));
			QToolTip::hideText();
			QToolTip::showText(event->globalPos(), OutString, this);
		}
	}
}

void ScrollScale::wheelEvent(QWheelEvent *event)
{
    if (this->ScrolledPicture.isNull())
        return;

    int PreviousWidth = this->SelectedSize.width();
    int PreviousHeight = this->SelectedSize.height();

    this->SelectedSize.setWidth(this->SelectedSize.width() - ((double)(event->angleDelta().y()) * SCROLL_SCALE));
    this->SelectedSize.setHeight(this->SelectedSize.width()*Ratio+0.5); //保证四舍五入，更好的保证尺寸

    if (this->SelectedSize.width() > this->OriginalPicture.width())
        this->SelectedSize.setWidth(this->OriginalPicture.width());
    else if (this->SelectedSize.width() < MIN_SELECTED_AREA)
        this->SelectedSize.setWidth(MIN_SELECTED_AREA);

    if (this->SelectedSize.height() > this->OriginalPicture.height())
        this->SelectedSize.setHeight(this->OriginalPicture.height());
    else if (this->SelectedSize.height() < (MIN_SELECTED_AREA * (double)(this->Ratio)))
        this->SelectedSize.setHeight(MIN_SELECTED_AREA * (double)(this->Ratio));

    int width = this->SelectedSize.x() + (PreviousWidth - this->SelectedSize.width()) / 2;
    int height = this->SelectedSize.y() + (PreviousHeight - this->SelectedSize.height()) / 2;

    if ((width + this->SelectedSize.width()) > OriginalPicture.width())
        width = OriginalPicture.width() - this->SelectedSize.width();

    if ((height + this->SelectedSize.height()) > OriginalPicture.height())
        height = OriginalPicture.height() - this->SelectedSize.height();

    if (width < 0)
        width = 0;
    if (height < 0)
        height = 0;

    this->SelectedSize.setX(width);
    this->SelectedSize.setY(height);

    this->SelectPicture();
}

void ScrollScale::mouseReleaseEvent(QMouseEvent *ev)
{
    this->setCursor(QCursor(Qt::CrossCursor));
	this->isMouseClicked = false;

	switch (DrawStatus)
	{
	case DrawingRectangle:
		{
			QPoint EndPoint;
			EndPoint.setX(ClickedPointOnScreen.x() + MovePointOnScreen.x());
			EndPoint.setY(ClickedPointOnScreen.y() + MovePointOnScreen.y());
			QPoint Begin = this->OnPictureToOnImage(OnScreenToOnPicture(this->ClickedPointOnScreen));
			QPoint End = this->OnPictureToOnImage(OnScreenToOnPicture(EndPoint));
			qDebug() << Begin << End;
			emit this->DrawRectangleSignal(Begin, End);
			this->DrawStatus = DrawStatus_e::NoDrawing;
			break;
		}
	case DrawingCircle:
		{
			QPoint EndPoint;
			EndPoint.setX(ClickedPointOnScreen.x() + MovePointOnScreen.x());
			EndPoint.setY(ClickedPointOnScreen.y() + MovePointOnScreen.y());
			QPoint Begin = this->OnPictureToOnImage(OnScreenToOnPicture(this->ClickedPointOnScreen));
			QPoint End = this->OnPictureToOnImage(OnScreenToOnPicture(EndPoint));
			QPoint Center = QPoint(((Begin.x() + End.x()) / 2), ((Begin.y() + End.y()) / 2));
			int Radius =sqrt(pow(Begin.x()- Center.x(),2)+ pow(Begin.y() - Center.y(), 2));
			qDebug()<< "Begin=" << Begin << "End=" << End << "Center=" << Center;
			emit this->DrawCircleSignal(Center, Radius);
			this->DrawStatus = DrawStatus_e::NoDrawing;
		}
		break;
	case DrawingPoints:
	{
		std::vector<QPoint> PointOnImage;
		this->DrawingPointArray.push_back(this->DrawingPointGroup);
		qDebug() << this->DrawingPointArray.size();
		for (auto Point : this->DrawingPointGroup)
		{
			QPoint temp = OnPictureToOnImage(OnScreenToOnPicture(Point));
			PointOnImage.push_back(temp);
		}
		this->DrawingPointsOnPictureArray.push_back(PointOnImage);
		emit this->DrawPointsSignal(PointOnImage);
		this->DrawingPointGroup.clear();
		this->DrawStatus = DrawStatus_e::WillDrawPoints;
		break;
	}
	case DrawingSinglePoint:
	{
		emit this->DrawSinglePointSignal(OnPictureToOnImage(OnScreenToOnPicture(ClickedPointOnScreen)));
		this->DrawStatus = DrawStatus_e::NoDrawing;
		break;
	}

	default:
		this->DrawStatus = DrawStatus_e::NoDrawing;
		break;
	}
}

void ScrollScale::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (AllowPopOut == true && OriginalPicture.isNull() == false)
	{
		ScrollScale* Window = new ScrollScale();
		Window->LoadPicture(this->OriginalPicture);
		Window->setAttribute(Qt::WA_DeleteOnClose);
		Window->AllowPopOut = false;
		Window->setWindowTitle(tr("Quick Look"));
		QObject::connect(Window, &ScrollScale::MousePositionChanged, [this](QPoint P) {
			emit this->MousePositionChanged(P);
		});

		QObject::connect(Window, &ScrollScale::DrawRectangleSignal, [this](QPoint Begin, QPoint End) {
			emit this->DrawRectangleSignal(Begin, End);
		});

		QObject::connect(Window, &ScrollScale::DrawCircleSignal, [this](QPoint Center, int Radius) {
			emit this->DrawCircleSignal(Center, Radius);
		});

		QObject::connect(Window, &ScrollScale::DrawPointsSignal, [this](std::vector<QPoint> Points) {
			emit this->DrawPointsSignal(Points);
		});

		QObject::connect(Window, &ScrollScale::DrawSinglePointSignal, [this](QPoint Point) {
			emit this->DrawSinglePointSignal(Point);
		});


		Window->show();
	}
}

void ScrollScale::paintEvent(QPaintEvent * event)
{
	QLabel::paintEvent(event); //先显式调用父类绘图事件，先把图画出来

	switch (DrawStatus)
	{
	case DrawingRectangle:
		DrawingPainter.begin(this);
		DrawingPainter.setPen(this->DrawingPen);
		DrawingPainter.drawRect(ClickedPointOnScreen.x(), ClickedPointOnScreen.y(), MovePointOnScreen.x(), MovePointOnScreen.y());
		DrawingPainter.end();
		break;
	case DrawingCircle:
		DrawingPainter.begin(this);
		DrawingPainter.setPen(this->DrawingPen);
		DrawingPainter.drawEllipse(ClickedPointOnScreen.x(), ClickedPointOnScreen.y(), (MovePointOnScreen.x() + MovePointOnScreen.y()) / 2, (MovePointOnScreen.x() + MovePointOnScreen.y()) / 2);
		DrawingPainter.end();
		break;
	case DrawingPoints:
	{
		QPainterPath Path;
		for (auto i : this->DrawingPointArray)
			Path.addPolygon(i);
		Path.addPolygon(this->DrawingPointGroup);
		DrawingPainter.begin(this);
		DrawingPainter.setPen(this->DrawingPen);
		DrawingPainter.drawPath(Path);
		//DrawingPainter.drawPoints(this->DrawingPointGroup);
		DrawingPainter.end();
		break;
	}
	case DrawingSinglePoint:
		DrawingPen.setWidth(7);
		DrawingPainter.begin(this);
		DrawingPainter.setPen(this->DrawingPen);
		DrawingPainter.drawPoint(this->ClickedPointOnScreen);
		DrawingPainter.end();
		DrawingPen.setWidth(3);
		break;
	default:
		break;
	}
}

void ScrollScale::keyPressEvent(QKeyEvent * event)
{
	QLabel::keyPressEvent(event); //处理父类的事件
	if (event->matches(QKeySequence::Copy))
	{
		this->PictureClipboard->setPixmap(this->OriginalPicture);
	}
}

void ScrollScale::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasUrls())
	{
		event->accept();
	}
}

void ScrollScale::dragLeaveEvent(QDragLeaveEvent * event)
{
	qDebug() << "DragLeaveEvent";
}

void ScrollScale::dragMoveEvent(QDragMoveEvent * event)
{
	qDebug() << "dragMoveEvent";
}

void ScrollScale::dropEvent(QDropEvent * event)
{
	qDebug() << "DropEvent";
	auto urls = event->mimeData()->urls();
	auto ImageUrl = urls[0];
	qDebug() << ImageUrl;
	QPixmap loadpic;
	if (!loadpic.load(ImageUrl.toLocalFile()))
		QMessageBox::critical(this, tr("Unsupported File Format"), tr("Fail to load this Picture"), QMessageBox::Ok);
	else
		this->LoadPicture(loadpic);
}

Q_INVOKABLE void ScrollScale::DisplayPicture(QPixmap Image)
{
	this->setPixmap(Image);
}

Q_INVOKABLE void ScrollScale::UpdateOriginalPicture(QPixmap Image)
{
	this->OriginalPicture = Image;
}

Q_INVOKABLE void ScrollScale::UpdateScrolledPicture(QPixmap Image)
{
	this->ScrolledPicture = Image;
}

void ScrollScale::InitContextMenu()
{
	this->SaveAction = new QAction(tr("Save Image"), this);
	this->CopyToClipboardAction = new QAction(tr("Copy"), this);
	this->DrawAnyAction = new QAction(tr("Pen"), this);
	this->DrawSquareAction = new QAction(tr("Draw Square"), this);
	this->DrawCircleAction = new QAction(tr("Draw Circle"), this);
	this->DrawSinglePointAction = new QAction(tr("Draw Point"), this);
	this->ClearScreenAction = new QAction(tr("Clear Screen"), this);

	this->addAction(this->SaveAction);
	this->addAction(this->CopyToClipboardAction);
	this->addAction(this->DrawAnyAction);
	this->addAction(this->DrawSinglePointAction);
	this->addAction(this->DrawSquareAction);
	this->addAction(this->DrawCircleAction);
	this->addAction(this->ClearScreenAction);

	this->setContextMenuPolicy(Qt::ActionsContextMenu);
	

	QObject::connect(this->SaveAction, &QAction::triggered, [this]() {
		QPixmap PicToSave = OriginalPicture;
		QString FilePath = QFileDialog::getSaveFileName(this, tr("Save Image"), "./", "*.bmp");
		auto a=PicToSave.save(FilePath);
	});
	
	QObject::connect(this->CopyToClipboardAction, &QAction::triggered,[this]() {
		this->PictureClipboard->setPixmap(this->OriginalPicture);
	});

	qRegisterMetaType<std::vector<QPoint>>("PointGroup");
	qRegisterMetaType<std::vector<std::vector<QPoint>>>("PointGroupArray");
	QObject::connect(this->DrawAnyAction, &QAction::triggered, [this]() {
		if (this->DrawAnyAction->text() == tr("Pen"))
		{
			this->DrawAnyAction->setText(tr("Finish Painting"));
			this->setCursor(QCursor(Qt::CrossCursor));
			this->DrawStatus = DrawStatus_e::WillDrawPoints;
			this->DrawingPointGroup.clear();
			this->DrawingPointArray.clear();
			this->DrawingPointsOnPictureArray.clear();
			this->update();
		}
		else if (this->DrawAnyAction->text() == tr("Finish Painting"))
		{
			this->DrawAnyAction->setText(tr("Pen"));
			qDebug() << "Will Clean Screen";
			this->DrawingPointArray.clear();
			this->DrawingPointGroup.clear();
			this->update();
			emit this->DrawPointsFinishedSignal(this->DrawingPointsOnPictureArray);
			this->DrawingPointsOnPictureArray.clear();
			this->DrawStatus = DrawStatus_e::NoDrawing;
		}
		else
		{
			this->DrawAnyAction->setText(tr("Pen"));
			this->DrawStatus = DrawStatus_e::NoDrawing;
			this->DrawingPointGroup.clear();
			this->DrawingPointArray.clear();
			this->DrawingPointsOnPictureArray.clear();
		}
	});

	QObject::connect(this->DrawSinglePointAction, &QAction::triggered, [this]() {
		this->setCursor(QCursor(Qt::CrossCursor));
		this->DrawStatus = DrawStatus_e::WillDrawSinglePoint;
		this->update();
	});
	
	QObject::connect(this->DrawSquareAction, &QAction::triggered, [this]() {
		this->DrawStatus = DrawStatus_e::WillDrawRectangle;
		qDebug() << "Will Square";
	});

	QObject::connect(this->DrawCircleAction, &QAction::triggered, [this]() {
		this->DrawStatus = DrawStatus_e::WillDrawCircle;
		qDebug() << "Will Circle";
	});

	QObject::connect(this->ClearScreenAction, &QAction::triggered, [this]() {
		qDebug() << "Will Clean Screen";
		this->DrawAnyAction->setText(tr("Pen"));
		this->DrawingPointArray.clear();
		this->DrawingPointGroup.clear();
		this->DrawingPointsOnPictureArray.clear();
		this->update();
		this->DrawStatus = DrawStatus_e::NoDrawing;
	});
}

ScrollScale::SetPixmapAsyn::SetPixmapAsyn(QObject * Obj, QThreadPool * ThreadPool)
{
	this->obj = Obj;
	this->ThreadPool = ThreadPool;
	this->setAutoDelete(false);
}

ScrollScale::SetPixmapAsyn::~SetPixmapAsyn()
{
	while (!lock.tryLock()); //等待解除互斥锁，避免直接delete导致的地址访问错误
	lock.unlock();
}

void ScrollScale::SetPixmapAsyn::StartProcess(QPixmap InputImage, QSize WindowSize, MyRect SelectedSize, bool Cut)
{
	this->InputImage = InputImage;
	this->WindowSize = WindowSize;
	this->SelectedSize = SelectedSize;
	this->Cut = Cut;
	this->isMatType = false;

	this->ThreadPool->start(this);
}

void ScrollScale::SetPixmapAsyn::StartProcess(cv::Mat InputImage, QSize WindowSize, MyRect SelectedSize, bool Cut)
{
	this->InputImageCV = InputImage;
	this->WindowSize = WindowSize;
	this->SelectedSize = SelectedSize;
	this->Cut = Cut;
	this->isMatType = true;

	this->ThreadPool->start(this);
}

void ScrollScale::SetPixmapAsyn::run()
{
	QMutexLocker locker(&this->lock);

	//qDebug() << QThread::currentThreadId();
	QPixmap OutputImage;
	QPixmap SelectedPixmap;
	if (isMatType)
	{
		if (this->InputImageCV.empty())
			return;

		cv::Mat QtMat;
		QImage img_L;
		cvtColor(this->InputImageCV, QtMat, CV_BGR2RGB); //BGR通道与RGB通道的转换
		img_L = QImage((const unsigned char *)(QtMat.data), QtMat.cols, QtMat.rows, QtMat.cols * QtMat.channels(), QImage::Format_RGB888);
		this->InputImage = QPixmap::fromImage(img_L);
		QMetaObject::invokeMethod(this->obj, "UpdateOriginalPicture", Q_ARG(QPixmap, InputImage));
	}

	if (InputImage.isNull())
		return;

	if (Cut)
	{
		SelectedPixmap = InputImage.copy(SelectedSize.x(), SelectedSize.y(), SelectedSize.width(), SelectedSize.height());
	}
	else
	{
		SelectedPixmap = InputImage;
	}

	OutputImage = SelectedPixmap.scaled(this->WindowSize, Qt::KeepAspectRatio);
	QMetaObject::invokeMethod(this->obj, "UpdateScrolledPicture", Q_ARG(QPixmap, SelectedPixmap));
	QMetaObject::invokeMethod(this->obj, "DisplayPicture", Q_ARG(QPixmap, OutputImage));
}
