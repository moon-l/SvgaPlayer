#pragma once

#include <QWidget>
#include "common.h"

class SvgaWidgetPrivate;

class SVGA_EXPORT SvgaWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(SvgaWidget)

public:
	Q_PROPERTY(QString imagePath READ getImagePath WRITE setImagePath)
	Q_PROPERTY(bool loops READ getLoops WRITE setLoops)
	Q_PROPERTY(quint32 frameIndex READ getFrameIndex WRITE setFrameIndex)

	Q_ENUMS(SvgaImageScalePolicy)
	enum SvgaImageScalePolicy 
	{
		ScaleWidgetCenter = 0x01,
		ScaleToLeftTop = 0x02,		//Õº∆¨ΩÙÃ˘◊Û…œΩ«
		ScaleToWidget = 0x04,               
		ScaleNone = 0x08,
		WidgetCenter = 0x10			//≤ª∑≈Àıæ”÷–	
	};

	Q_DECLARE_FLAGS(SvgaImageScalePolicys, SvgaImageScalePolicy)
	Q_PROPERTY(SvgaImageScalePolicy scalePolicy READ getScalePolicy WRITE setScalePolicy)

public:
	explicit SvgaWidget(QWidget *parent = NULL);
	~SvgaWidget();

public:
	void play(const QString& path, int index = 0);
	bool start();
	void stop();

	bool isPlaying();

	void setFrameIndex(quint32 index);
	quint32 getFrameIndex();

	void setImagePath(const QString &imageName);
	QString getImagePath();

	void setScalePolicy(SvgaImageScalePolicy scalePolicy);
	SvgaImageScalePolicy getScalePolicy();

	void setLoops(bool bLoop);
	bool getLoops();

signals:
	void finished();
	void startTimeout();

protected slots:
	void on_timeout();
	void on_restart();

protected:
	virtual void paintEvent(QPaintEvent*);

private:
	SvgaWidgetPrivate*	m_ptr;
	Q_DECLARE_PRIVATE_D(m_ptr, SvgaWidget)
};
