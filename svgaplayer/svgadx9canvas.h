#pragma once

#include <QWidget>
#include "svgadef.h"

class SvgaDx9CanvasPrivate;

class SVGA_EXPORT SvgaDx9Canvas : public SvgaCanvas
{
public:
	explicit SvgaDx9Canvas();
	~SvgaDx9Canvas();

	void init(HWND parent);
	void setPos(int x, int y);
	void setVisible(bool visible);

protected:
	virtual void setVideoSize(int width, int height);
	virtual void begin();
	virtual void end();
	virtual void draw(const QString& key, QPixmap& pix, QRect& layout, QTransform& transform, float alpha);

private:
	SvgaDx9CanvasPrivate*	m_ptr;
	Q_DECLARE_PRIVATE_D(m_ptr, SvgaDx9Canvas)
};
