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
	m_pathItems = path.trimmed().split(QRegExp("[,\\s+]"));
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

bool SvgaPath::isValid()
{
	return m_built;
}

void SvgaPath::_buildPath()
{
	int index = 0;
	int count = m_pathItems.size();

	float posX = 0;
	float posY = 0;
	float posX1 = 0;
	float posY1 = 0;
	float posX2 = 0;
	float posY2 = 0;

	while (index < count)
	{
		switch (_popChar(index)) 
		{
		case 'M':
			posX = _popFloat(index);
			posY = _popFloat(index);
			m_path.moveTo(posX, posY);
			break;
		case 'm':
			posX += _popFloat(index);
			posY += _popFloat(index);
			m_path.moveTo(posX, posY);
			break;
		case 'L':
			posX = _popFloat(index);
			posY = _popFloat(index);
			m_path.lineTo(posX, posY);
			break;
		case 'l':
			posX += _popFloat(index);
			posY += _popFloat(index);
			m_path.lineTo(posX, posY);
			break;
		case 'H':
			posX = _popFloat(index);
			m_path.lineTo(posX, posY);
			break;
		case 'h':
			posX += _popFloat(index);
			m_path.lineTo(posX, posY);
			break;
		case 'V':
			posY = _popFloat(index);
			m_path.lineTo(posX, posY);
			break;
		case 'v':
			posY += _popFloat(index);
			m_path.lineTo(posX, posY);
			break;
		case 'C':
			posX1 = _popFloat(index);
			posY1 = _popFloat(index);
			posX2 = _popFloat(index);
			posY2 = _popFloat(index);
			posX = _popFloat(index);
			posY = _popFloat(index);
			m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			break;
		case 'c':
			posX1 = posX + _popFloat(index);
			posY1 = posY + _popFloat(index);
			posX2 = posX + _popFloat(index);
			posY2 = posY + _popFloat(index);
			posX += _popFloat(index);
			posY += _popFloat(index);
			m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			break;
		case 'S':
			if (posX1 && posY1 && posX2 && posY2)
			{
				posX1 = posX - posX2 + posX;
				posY1 = posY - posY2 + posY;
				posX2 = _popFloat(index);
				posY2 = _popFloat(index);
				posX = _popFloat(index);
				posY = _popFloat(index);
				m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			}
			else
			{
				posX1 = _popFloat(index);
				posY1 = _popFloat(index);
				posX = _popFloat(index);
				posY = _popFloat(index);
				m_path.quadTo(posX1, posY1, posX, posY);
			}
			break;
		case 's':
			if (posX1 && posY1 && posX2 && posY2)
			{
				posX1 = posX - posX2 + posX;
				posY1 = posY - posY2 + posY;
				posX2 = posX + _popFloat(index);
				posY2 = posY + _popFloat(index);
				posX += _popFloat(index);
				posY += _popFloat(index);
				m_path.cubicTo(posX1, posY1, posX2, posY2, posX, posY);
			}
			else
			{
				posX1 = posX + _popFloat(index);
				posY1 = posY + _popFloat(index);
				posX += _popFloat(index);
				posY += _popFloat(index);
				m_path.quadTo(posX1, posY1, posX, posY);
			}
			break;
		case 'Q':
			posX1 = _popFloat(index);
			posY1 = _popFloat(index);
			posX = _popFloat(index);
			posY = _popFloat(index);
			m_path.quadTo(posX1, posY1, posX, posY);
			break;
		case 'q':
			posX1 = posX + _popFloat(index);
			posY1 = posY + _popFloat(index);
			posX += _popFloat(index);
			posY += _popFloat(index);
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

char SvgaPath::_popChar(int& index)
{
	QString item = m_pathItems.value(index++);
	if (item.isEmpty())
		return 0;
	
	return item.at(0).toAscii();
}

float SvgaPath::_popFloat(int& index)
{
	return m_pathItems.value(index++).toFloat();
}