#pragma  once

#ifdef BUILD_SVGA
#define SVGA_EXPORT __declspec(dllexport)
#else
#define SVGA_EXPORT __declspec(dllimport)
#endif

#include <QPixmap>

struct DrawItem
{
	QString		key;
	QPixmap		pix;
	bool		dynamic;
	float		alpha;
	QRect		layout;
	QTransform	transform;
	QString		clipPath;
};

struct SvgaCanvas
{
	virtual void setVideoSize(int width, int height) = 0;
	virtual bool begin() = 0;
	virtual void end() = 0;
	virtual void draw(DrawItem* item) = 0;
};
