#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include <fstream>
#include "svgaresource.h"
#include "proto/svga.pb.h"
#include "json/json.h"
#include "zip/unzipex.h"

QMap<QString, SvgaVideoEntity> SvgaResource::s_videoEntityCache;

SvgaResource::SvgaResource()
: m_video(NULL)
, m_bCache(false)
{

}

SvgaResource::~SvgaResource()
{
	clear();

	if (m_video)
	{
		delete m_video;
		m_video = NULL;
	}
}

bool SvgaResource::load(const std::wstring& path, bool cache)
{
	clear();

	if (path.empty())
	{
		return false;
	}

	QFileInfo fileInfo(QString::fromStdWString(path));
	if (!fileInfo.exists())
	{
		return false;
	}

	QString filePath = fileInfo.absoluteFilePath();
	if (s_videoEntityCache.contains(filePath))
	{
		m_video = &(s_videoEntityCache[filePath]);
		if (m_video->valid())
		{
			m_bCache = true;
			return true;
		}
	}

	m_bCache = cache;
	if (cache)
	{
		m_video = &(s_videoEntityCache[fileInfo.absoluteFilePath()]);
	}
	else
	{
		m_video = new SvgaVideoEntity;
	}

	bool is1_x = false;
	QFile file(filePath);
	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray bytes = file.read(4);
		if (bytes.size() == 4)
		{
			if (bytes[0] == (char)80 && bytes[1] == (char)75 && bytes[2] == (char)3 && bytes[3] == (char)4)
			{
				is1_x = true;
			}
		}

		file.close();
	}

	if (is1_x)
	{
		return _load1_x(path);
	}
	else
	{
		return _load2_x(path);
	}
}

void SvgaResource::clear()
{
	m_dynamicImages.clear();

	if (m_video)
	{
		if (m_bCache)
		{
			m_video = NULL;
		}
		else
		{
			m_video->clear();
		}
	}
}

QPixmap SvgaResource::getImage(const QString& key)
{
	return m_video->getImage(key);
}

QPixmap SvgaResource::getDynamicImage(const QString& key)
{
	return m_dynamicImages.value(key);
}

SvgaVideoEntity* SvgaResource::getVideoEntity()
{
	return m_video;
}

QSize SvgaResource::getItemSize(const QString& key)
{
	return getImage(key).size();
}

bool SvgaResource::addDynamicItem(const QString& key, QPixmap& image)
{
	if (image.size() == getItemSize(key))
	{
		m_dynamicImages[key] = image;
		return true;
	}

	return false;
}

void SvgaResource::removeDynamicItem(const QString& key)
{
	m_dynamicImages.remove(key);
}

void SvgaResource::clearAllDynamicItems()
{
	m_dynamicImages.clear();
}

void SvgaResource::clearCache(const std::wstring& path)
{
	QFileInfo fileInfo(QString::fromStdWString(path));
	QString filePath = fileInfo.absoluteFilePath();
	s_videoEntityCache.remove(filePath);
}

void SvgaResource::clearAllCache()
{
	s_videoEntityCache.clear();
}

bool SvgaResource::_load1_x(const std::wstring& path)
{
	HZIP hz = OpenZip(path.c_str(), 0);
	if (hz == NULL)
	{
		return false;
	}

	ZIPENTRY ze; 
	if (ZR_OK != GetZipItem(hz, -1, &ze))
	{
		CloseZip(hz);
		return false;
	}

	std::string buffer;
	int count = ze.index;
	for (int i = 0; i < count; i++)
	{
		if (ZR_OK != GetZipItem(hz, i, &ze))
		{
			CloseZip(hz);
			return false;
		}

		buffer.resize(ze.unc_size + 1);
		if (ZR_OK != UnzipItem(hz, i, &buffer[0], ze.unc_size))
		{
			CloseZip(hz);
			return false;
		}
		buffer[ze.unc_size] = 0;

		QString name = QString::fromWCharArray(ze.name);
		if (name == "movie.spec")
		{
			if (!_parseMovie(buffer))
			{
				CloseZip(hz);
				return false;
			}
		}
		else
		{
			if (!m_video->parseImage(buffer, name))
			{
				CloseZip(hz);
				return false;
			}
		}
	}

	CloseZip(hz);
	return true;
}

bool SvgaResource::_load2_x(const std::wstring& path)
{
	std::ifstream input;
	input.open(path.c_str(), std::ifstream::in | std::ifstream::binary);

	google::protobuf::io::IstreamInputStream iis(&input);
	google::protobuf::io::GzipInputStream gis(&iis);
	com::opensource::svga::MovieEntity entity;
	bool res = entity.ParseFromZeroCopyStream(&gis);
	if (res)
	{
		m_video->parse(entity);
	}

	input.close();

	return res;
}

bool SvgaResource::_parseMovie(const std::string& buffer)
{
	Json::Reader jsonReader;
	Json::Value jsonRoot;
	if (!jsonReader.parse(buffer, jsonRoot) || jsonRoot.isNull())
	{
		return false;
	}

	return m_video->parse(jsonRoot);
}
