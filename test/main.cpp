#include <QApplication>
#include "svgaplayer.h"
#include "svgagdicanvas.h"
#include "svgadx9canvas.h"
#include "svgaglcanvas.h"

//#define TEST_GDI
#define TEST_DX

int main(int argc, char * argv[])
{
	QApplication app(argc, argv);

#ifdef TEST_GDI
	SvgaPlayer player;
	SvgaGDICanvas w;
	w.setFixedSize(500, 500);
	player.setCanvas(&w);
	player.setLoops(true);
	player.play("angel.svga");
	w.show();
#else
#ifdef TEST_DX
	QWidget widget;
	SvgaPlayer player;
	SvgaDx9Canvas w;
	w.init(widget.winId());
	player.setCanvas(&w);
	player.setLoops(true);
	player.play("posche.svga");
	w.setPos(0, 0);
	w.setVisible(true);
	widget.show();
#else
	QWidget widget;
	SvgaPlayer player;
	SvgaGLCanvas w;
	w.init(widget.winId());
	player.setCanvas(&w);
	player.setLoops(true);
	player.play("angel.svga");
	w.setPos(0, 0);
	w.setVisible(true);
	widget.show();
#endif
#endif

	return app.exec();
}
