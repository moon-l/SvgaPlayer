#pragma once

class SvgaPath
{
public:
	SvgaPath();
	~SvgaPath();

	void setPath(const QString& path);
	QPainterPath& getPath();
	QPixmap clip(const QPixmap& pix);
	QImage clipAsImage(const QPixmap& pix, QImage::Format format = QImage::Format_ARGB32_Premultiplied);

private:
	void _buildPath();
	char _popChar();
	float _popFloat();

private:
	QStringList		m_pathItems;
	QPainterPath	m_path;
	bool			m_built;
};
