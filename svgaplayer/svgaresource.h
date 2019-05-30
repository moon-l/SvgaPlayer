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
	bool _load1_x(const std::wstring& path);
	bool _load2_x(const std::wstring& path);
	
	bool _parseImage(const std::string& buffer, const QString& name);
	bool _parseMovie(const std::string& buffer);
	void _parseImage(const com::opensource::svga::MovieEntity& obj);

private:
	QMap<QString, QPixmap>	m_dynamicImages;
	QMap<QString, QPixmap>	m_images;
	SvgaVideoEntity			m_video;
};
