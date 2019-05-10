#pragma  once

#ifdef BUILD_SVGA
#define SVGA_EXPORT __declspec(dllexport)
#else
#define SVGA_EXPORT __declspec(dllimport)
#endif

struct SvgaCanvas
{
	virtual void setVideoSize(int width, int height) = 0;
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void draw(QPixmap& pix, QRect& layout, QTransform& transform, float alpha) = 0;
};
