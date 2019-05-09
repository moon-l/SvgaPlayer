#pragma  once

class SvgaResource;

struct SvgaPlayer
{
	virtual void init() = 0;
	virtual void release() = 0;
	virtual void draw(QPixmap& image, SvgaResource* resource, int index) = 0;
};
