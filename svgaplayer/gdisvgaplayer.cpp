#include "gdisvgaplayer.h"
#include "svgaresource.h"

static char popListChar(QStringList& list)
{
	if (list.isEmpty())
	{
		return '0';
	}

	QString str = list.at(0);
	if (str.isEmpty() || str.size() > 1)
	{
		list.pop_front();
		return '0';
	}

	char tmp = str.at(0).toAscii();
	list.pop_front();

	return tmp;
}

static float popListNumber(QStringList& list)
{
	if (list.isEmpty())
	{
		return 0;
	}

	float val = list.at(0).toFloat();
	list.pop_front();
	return val;
}

static void stringToCliPath(const QString& clipPath, QPainterPath& painterPath)
{
	QString temp = clipPath.trimmed();
	QStringList nodeList = temp.split(" ");

	float posX = 0;
	float posY = 0;
	float posX1 = 0;
	float posY1 = 0;
	float posX2 = 0;
	float posY2 = 0;
	while (!nodeList.isEmpty())
	{
		switch (popListChar(nodeList)) 
		{
		case 'M':
			posX = popListNumber(nodeList);
			posY = popListNumber(nodeList);
			painterPath.moveTo(posX, posY);
			break;
		case 'm':
			posX += popListNumber(nodeList);
			posY += popListNumber(nodeList);
			painterPath.moveTo(posX, posY);
			break;
		case 'L':
			posX = popListNumber(nodeList);
			posY = popListNumber(nodeList);
			painterPath.lineTo(posX, posY);
			break;
		case 'l':
			posX += popListNumber(nodeList);
			posY += popListNumber(nodeList);
			painterPath.lineTo(posX, posY);
			break;
		case 'H':
			posX = popListNumber(nodeList);
			painterPath.lineTo(posX, posY);
			break;
		case 'h':
			posX += popListNumber(nodeList);
			painterPath.lineTo(posX, posY);
			break;
		case 'V':
			posY = popListNumber(nodeList);
			painterPath.lineTo(posX, posY);
			break;
		case 'v':
			posY += popListNumber(nodeList);
			painterPath.lineTo(posX, posY);
			break;
		case 'C':
			posX1 = popListNumber(nodeList);
			posY1 = popListNumber(nodeList);
			posX2 = popListNumber(nodeList);
			posY2 = popListNumber(nodeList);
			posX = popListNumber(nodeList);
			posY = popListNumber(nodeList);
			painterPath.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			break;
		case 'c':
			posX1 = posX + popListNumber(nodeList);
			posY1 = posY + popListNumber(nodeList);
			posX2 = posX + popListNumber(nodeList);
			posY2 = posY + popListNumber(nodeList);
			posX += popListNumber(nodeList);
			posY += popListNumber(nodeList);
			painterPath.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			break;
		case 'S':
			if (posX1 && posY1 && posX2 && posY2) {
				posX1 = posX - posX2 + posX;
				posY1 = posY - posY2 + posY;
				posX2 = popListNumber(nodeList);
				posY2 = popListNumber(nodeList);
				posX = popListNumber(nodeList);
				posY = popListNumber(nodeList);
				painterPath.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			} else {
				posX1 = popListNumber(nodeList);
				posY1 = popListNumber(nodeList);
				posX = popListNumber(nodeList);
				posY = popListNumber(nodeList);
				painterPath.quadTo(posX1, posY1, posX, posY);
			}
			break;
		case 's':
			if (posX1 && posY1 && posX2 && posY2) {
				posX1 = posX - posX2 + posX;
				posY1 = posY - posY2 + posY;
				posX2 = posX + popListNumber(nodeList);
				posY2 = posY + popListNumber(nodeList);
				posX += popListNumber(nodeList);
				posY += popListNumber(nodeList);
				painterPath.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			} else {
				posX1 = posX + popListNumber(nodeList);
				posY1 = posY + popListNumber(nodeList);
				posX += popListNumber(nodeList);
				posY += popListNumber(nodeList);
				painterPath.quadTo(posX1, posY1, posX, posY);
			}
			break;
		case 'Q':
			posX1 = popListNumber(nodeList);
			posY1 = popListNumber(nodeList);
			posX = popListNumber(nodeList);
			posY = popListNumber(nodeList);
			painterPath.quadTo(posX1, posY1, posX, posY);
			break;
		case 'q':
			posX1 = posX + popListNumber(nodeList);
			posY1 = posY + popListNumber(nodeList);
			posX += popListNumber(nodeList);
			posY += popListNumber(nodeList);
			painterPath.quadTo(posX1, posY1, posX, posY);
			break;
		case 'A':
			break;
		case 'a':
			break;
		case 'Z':
		case 'z':
			painterPath.closeSubpath();
			break;
		default:
			break;
		}

	}
}

GDISvgaPlayer::GDISvgaPlayer()
{

}

GDISvgaPlayer::~GDISvgaPlayer()
{

}

void GDISvgaPlayer::init()
{

}

void GDISvgaPlayer::release()
{

}

void GDISvgaPlayer::draw(QPixmap& image, SvgaResource* resource, int index)
{
	int width = resource->getWidth();
	int height = resource->getHeight();

	if (width <= 0 || height <= 0)
	{
		return;
	}

	if (image.isNull() || image.width() != width || image.height() != height)
	{
		image = QPixmap(width, height);
	}
	// clear image
	image.fill(Qt::transparent);

	QPainter painter(&image);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	std::vector<SvgaSingleFrame> frames;
	resource->getFrame(index, frames);

	for (int i = 0; i < frames.size(); i++)
	{
		_drawSprite(&painter, resource, frames[i]);
	}
}

void GDISvgaPlayer::_drawSprite(QPainter* painter, SvgaResource* resource, const SvgaSingleFrame& frame)
{
	QPixmap spriteImage = resource->getImage(frame.name);
	if (spriteImage.isNull())
	{
		return;
	}

	painter->save();

	painter->setOpacity(frame.frame->alpha);
	painter->setMatrix(frame.frame->transform);

	if (!frame.frame->clipPath.isEmpty())
	{
		QPainterPath painterPath;
		stringToCliPath(frame.frame->clipPath, painterPath);
		painter->setClipping(true);
		painter->setClipPath(painterPath);
	}

	painter->drawPixmap(frame.frame->layout, spriteImage);

	painter->restore();
}
