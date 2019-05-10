#pragma once

class SvgaPath
{
public:
	SvgaPath();
	~SvgaPath();

	void setPath(const QString& path);
	QPainterPath& getPath();
	bool isValid();

private:
	void _buildPath();
	char _popChar(int& index);
	float _popFloat(int& index);

private:
	QStringList		m_pathItems;
	QPainterPath	m_path;
	bool			m_built;
};
