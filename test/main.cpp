#include <QApplication>
#include "svgaplayer.h"
#include "svgagdicanvas.h"

int main(int argc, char * argv[])
{
	QApplication app(argc, argv);

	SvgaPlayer player;
	SvgaGDICanvas w;
	w.setFixedSize(500, 500);
	player.setCanvas(&w);
	player.setLoops(true);
	player.play("test.svga");
	w.show();

	return app.exec();
}
