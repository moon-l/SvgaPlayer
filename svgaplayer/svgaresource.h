#pragma once

#include "svgavideoentity.h"
#include "svgapath.h"

class SvgaResource
{
public:
	SvgaResource();
	~SvgaResource();

	bool load(const std::wstring& path);
	void clear();

	QPixmap getImage(const QString& key, SvgaPath& clipPath);
	SvgaVideoEntity* getVideoEntity();

private:
	bool _parseImage(const std::string& buffer, const QString& name);
	bool _parseMovie(const std::string& buffer);

private:
	QMap<QString, QPixmap>	m_images;
	SvgaVideoEntity			m_video;
};
