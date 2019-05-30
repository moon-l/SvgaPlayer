#pragma once

#include <QWidget>
#include "svgadef.h"

class SvgaGDICanvasPrivate;

class SVGA_EXPORT SvgaGDICanvas : public QWidget, public SvgaCanvas
{
	Q_OBJECT
	Q_DISABLE_COPY(SvgaGDICanvas)

public:
	enum SvgaImageScalePolicy 
	{
		ScaleWidgetCenter = 0x01,
		ScaleToLeftTop = 0x02,		//Õº∆¨ΩÙÃ˘◊Û…œΩ«
		ScaleToWidget = 0x04,               
		ScaleNone = 0x08,
		WidgetCenter = 0x10			//≤ª∑≈Àıæ”÷–	
	};

	Q_ENUMS(SvgaImageScalePolicy)
	Q_DECLARE_FLAGS(SvgaImageScalePolicys, SvgaImageScalePolicy)

public:
	explicit SvgaGDICanvas(QWidget* parent = NULL);
	~SvgaGDICanvas();

	void setScalePolicy(SvgaImageScalePolicy scalePolicy);
	SvgaImageScalePolicy getScalePolicy();

protected:
	virtual void setVideoSize(int width, int height);
	virtual bool begin();
	virtual void end();
	virtual void draw(DrawItem* item);

protected:
	virtual void paintEvent(QPaintEvent*);

private:
	SvgaGDICanvasPrivate*	m_ptr;
	Q_DECLARE_PRIVATE_D(m_ptr, SvgaGDICanvas)
};
