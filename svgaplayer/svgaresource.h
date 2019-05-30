#pragma once

#include "svgavideoentity.h"
#include "svgapath.h"

namespace com
{
	namespace opensource
	{
		namespace svga
		{
			class MovieEntity;
		}
	}
}

class SvgaResource
{
public:
	SvgaResource();
	~SvgaResource();

	bool load(const std::wstring& path);
	void clear();

	QPixmap getImage(const QString& key);
	QPixmap getDynamicImage(const QString& key);
	SvgaVideoEntity* getVideoEntity();

	QSize getItemSize(const QString& key);
	bool addDynamicItem(const QString& key, QPixmap& image);
	void removeDynamicItem(const QString& key);
	void clearAllDynamicItems();

private:
	void _parseImage(const com::opensource::svga::MovieEntity& obj);

private:
	QMap<QString, QPixmap>	m_dynamicImages;
	QMap<QString, QPixmap>	m_images;
	SvgaVideoEntity			m_video;
};
