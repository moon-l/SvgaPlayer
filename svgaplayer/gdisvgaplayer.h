#pragma once

#include "def.h"

struct SvgaSingleFrame;

class GDISvgaPlayer : public SvgaPlayer
{
public:
	GDISvgaPlayer();
	~GDISvgaPlayer();

protected:
	virtual void init();
	virtual void release();
	virtual void draw(QPixmap& image, SvgaResource* resource, int index);

private:
	void _drawSprite(QPainter* painter, SvgaResource* resource, const SvgaSingleFrame& frame);
};