#include "svgapath.h"

SvgaPath::SvgaPath()
: m_built(false)
{

}

SvgaPath::~SvgaPath()
{

}

void SvgaPath::setPath(const QString& path)
{
	QString temp = path.trimmed();
	if (!temp.isEmpty())
	{
		m_pathItems = temp.split(QRegExp("[,\\s+]"));
	}
	m_built = false;
}

QPainterPath& SvgaPath::getPath()
{
	if (!m_built)
	{
		_buildPath();
		m_built = true;
	}

	return m_path;
}

QPixmap SvgaPath::clip(const QPixmap& pix)
{
	QPainterPath& clipPath = getPath();
	if (clipPath.isEmpty())
	{
		return pix;
	}

	QPixmap temp = QPixmap(pix.width(), pix.height());
	temp.fill(Qt::transparent);

	QPainter imagePainter(&temp);
	imagePainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	imagePainter.setClipping(true);
	imagePainter.setClipPath(clipPath);
	imagePainter.drawPixmap(0, 0, pix);
	imagePainter.setClipping(false);
	imagePainter.end();

	return temp;
}

QImage SvgaPath::clipAsImage(const QPixmap& pix)
{
	QImage image(pix.width(), pix.height(), QImage::Format_ARGB32);
	image.fill(Qt::transparent);
	QPainter imagePainter(&image);
	imagePainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (getPath().isEmpty())
	{
		imagePainter.drawPixmap(0, 0, pix);
	}
	else
	{
		imagePainter.setClipping(true);
		imagePainter.setClipPath(getPath());
		imagePainter.drawPixmap(0, 0, pix);
		imagePainter.setClipping(false);
	}

	return image;
}

void SvgaPath::_buildPath()
{
	float posX = 0;
	float posY = 0;
	float posX1 = 0;
	float posY1 = 0;
	float posX2 = 0;
	float posY2 = 0;

	while (!m_pathItems.isEmpty())
	{
		switch (_popChar()) 
		{
		case 'M':
			posX = _popFloat();
			posY = _popFloat();
			m_path.moveTo(posX, posY);
			break;
		case 'm':
			posX += _popFloat();
			posY += _popFloat();
			m_path.moveTo(posX, posY);
			break;
		case 'L':
			posX = _popFloat();
			posY = _popFloat();
			m_path.lineTo(posX, posY);
			break;
		case 'l':
			posX += _popFloat();
			posY += _popFloat();
			m_path.lineTo(posX, posY);
			break;
		case 'H':
			posX = _popFloat();
			m_path.lineTo(posX, posY);
			break;
		case 'h':
			posX += _popFloat();
			m_path.lineTo(posX, posY);
			break;
		case 'V':
			posY = _popFloat();
			m_path.lineTo(posX, posY);
			break;
		case 'v':
			posY += _popFloat();
			m_path.lineTo(posX, posY);
			break;
		case 'C':
			posX1 = _popFloat();
			posY1 = _popFloat();
			posX2 = _popFloat();
			posY2 = _popFloat();
			posX = _popFloat();
			posY = _popFloat();
			m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			break;
		case 'c':
			posX1 = posX + _popFloat();
			posY1 = posY + _popFloat();
			posX2 = posX + _popFloat();
			posY2 = posY + _popFloat();
			posX += _popFloat();
			posY += _popFloat();
			m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			break;
		case 'S':
			if (posX1 && posY1 && posX2 && posY2)
			{
				posX1 = posX - posX2 + posX;
				posY1 = posY - posY2 + posY;
				posX2 = _popFloat();
				posY2 = _popFloat();
				posX = _popFloat();
				posY = _popFloat();
				m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			}
			else
			{
				posX1 = _popFloat();
				posY1 = _popFloat();
				posX = _popFloat();
				posY = _popFloat();
				m_path.quadTo(posX1, posY1, posX, posY);
			}
			break;
		case 's':
			if (posX1 && posY1 && posX2 && posY2)
			{
				posX1 = posX - posX2 + posX;
				posY1 = posY - posY2 + posY;
				posX2 = posX + _popFloat();
				posY2 = posY + _popFloat();
				posX += _popFloat();
				posY += _popFloat();
				m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			}
			else
			{
				posX1 = posX + _popFloat();
				posY1 = posY + _popFloat();
				posX += _popFloat();
				posY += _popFloat();
				m_path.quadTo(posX1, posY1, posX, posY);
			}
			break;
		case 'Q':
			posX1 = _popFloat();
			posY1 = _popFloat();
			posX = _popFloat();
			posY = _popFloat();
			m_path.quadTo(posX1, posY1, posX, posY);
			break;
		case 'q':
			posX1 = posX + _popFloat();
			posY1 = posY + _popFloat();
			posX += _popFloat();
			posY += _popFloat();
			m_path.quadTo(posX1, posY1, posX, posY);
			break;
		case 'A':
			break;
		case 'a':
			break;
		case 'Z':
		case 'z':
			m_path.closeSubpath();
			break;
		default:
			break;
		}

	}
}

char SvgaPath::_popChar()
{
	if (m_pathItems.isEmpty())
	{
		return 0;
	}

	QString& item = m_pathItems.front();
	if (item.isEmpty())
	{
		m_pathItems.pop_front();
		return 0;
	}

	char cmd = item.at(0).toAscii();
	if (item.size() > 1)
	{
		item.remove(0, 1);
	}
	else
	{
		m_pathItems.pop_front();
	}

	return cmd;
}

float SvgaPath::_popFloat()
{
	if (m_pathItems.isEmpty())
	{
		return 0;
	}

	QString v = m_pathItems.takeFirst();
	return ::atof(v.toStdString().c_str());
}
