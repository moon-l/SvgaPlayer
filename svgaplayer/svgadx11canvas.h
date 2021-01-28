#pragma once

#include "svgadef.h"

class SvgaDx11CanvasPrivate;

class SVGA_EXPORT SvgaDx11Canvas : public SvgaCanvas
{
public:
	explicit SvgaDx11Canvas();
	~SvgaDx11Canvas();

	void init(HWND parent);
	void setPos(int x, int y);
	void setVisible(bool visible);

protected:
	virtual void setVideoSize(int width, int height);
	virtual bool begin();
	virtual void end();
	virtual void draw(DrawItem* item);

private:
	SvgaDx11CanvasPrivate*	m_ptr;
	Q_DECLARE_PRIVATE_D(m_ptr, SvgaDx11Canvas)
};
