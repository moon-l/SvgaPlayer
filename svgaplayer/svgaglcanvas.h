#pragma once

#include "svgadef.h"

class SvgaGLCanvasPrivate;

class SVGA_EXPORT SvgaGLCanvas : public SvgaCanvas
{
public:
	explicit SvgaGLCanvas();
	~SvgaGLCanvas();

	void init(HWND parent);
	void setPos(int x, int y);
	void setVisible(bool visible);

protected:
	virtual void setVideoSize(int width, int height);
	virtual bool begin();
	virtual void end();
	virtual void draw(DrawItem* item);

private:
	SvgaGLCanvasPrivate*	m_ptr;
	Q_DECLARE_PRIVATE_D(m_ptr, SvgaGLCanvas)
};
