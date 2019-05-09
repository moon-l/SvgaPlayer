#include <QApplication>
#include "../svgaplayer/svgawidget.h"

int main(int argc, char * argv[])
{
	QApplication app(argc, argv);

	SvgaWidget w;
	w.setLoops(true);
	w.play("test.svga");
	w.show();

	return app.exec();
}
