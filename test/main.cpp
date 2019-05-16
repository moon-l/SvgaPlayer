#include <QApplication>
#include "svgaplayer.h"
#include "svgagdicanvas.h"
#include "svgadx9canvas.h"

int main(int argc, char * argv[])
{
	QApplication app(argc, argv);

	//SvgaPlayer player;
	//SvgaGDICanvas w;
	//w.setFixedSize(500, 500);
	//player.setCanvas(&w);
	//player.setLoops(true);
	//player.play("test.svga");
	//w.show();

	QWidget widget;
	SvgaPlayer player;
	SvgaDx9Canvas w;
	w.init(widget.winId());
	player.setCanvas(&w);
	player.setLoops(true);
	player.play("test.svga");
	w.setPos(0, 0);
	w.setVisible(true);
	widget.show();

	return app.exec();
}
