#pragma once

#include "svgavideoentity.h"
#include "svgapath.h"

class SvgaResource
{
public:
	SvgaResource();
	~SvgaResource();

	bool load(const std::wstring& path, bool cache);
	void clear();

	QPixmap getImage(const QString& key);
	QPixmap getDynamicImage(const QString& key);
	SvgaVideoEntity* getVideoEntity();

	QSize getItemSize(const QString& key);
	bool addDynamicItem(const QString& key, QPixmap& image);
	void removeDynamicItem(const QString& key);
	void clearAllDynamicItems();

	static void clearCache(const std::wstring& path);
	static void clearAllCache();

private:
	bool _load1_x(const std::wstring& path);
	bool _load2_x(const std::wstring& path);
	
	bool _parseMovie(const std::string& buffer);

private:
	QMap<QString, QPixmap>	m_dynamicImages;
	SvgaVideoEntity*		m_video;
	bool					m_bCache;

private:
	static QMap<QString, SvgaVideoEntity>	s_videoEntityCache;
};
