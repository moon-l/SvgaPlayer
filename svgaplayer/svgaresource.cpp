#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include <fstream>
#include "svgaresource.h"
#include "proto/svga.pb.h"
#include "json/json.h"
#include "zip/unzipex.h"

SvgaResource::SvgaResource()
{

}

SvgaResource::~SvgaResource()
{
	clear();
}

bool SvgaResource::load(const std::wstring& path)
{
	clear();

	if (path.empty())
	{
		return false;
	}

	bool is1_x = false;
	QFile file(QString::fromStdWString(path));
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
	m_images.clear();
	m_video.clear();
}

QPixmap SvgaResource::getImage(const QString& key)
{
	return m_images.value(key);
}

QPixmap SvgaResource::getDynamicImage(const QString& key)
{
	return m_dynamicImages.value(key);
}

SvgaVideoEntity* SvgaResource::getVideoEntity()
{
	return &m_video;
}

QSize SvgaResource::getItemSize(const QString& key)
{
	const QPixmap& pix = m_images.value(key);
	return pix.size();
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
			if (!_parseImage(buffer, name))
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
		_parseImage(entity);
		m_video.parse(entity);
	}

	input.close();

	return res;
}

void SvgaResource::_parseImage(const com::opensource::svga::MovieEntity& obj)
{
	for (google::protobuf::Map<std::string, std::string>::const_iterator iter = obj.images().begin(); iter != obj.images().end(); iter++)
	{
		QString key = QString::fromStdString(iter->first);
		QImage image = QImage::fromData((const uchar*)iter->second.c_str(), iter->second.size());
		m_images[key] = QPixmap::fromImage(image);
	}
}

bool SvgaResource::_parseImage(const std::string& buffer, const QString& name)
{
	QImage image = QImage::fromData((const uchar*)buffer.c_str(), buffer.size());
	if (image.isNull())
	{
		return false;
	}

	m_images[name] = QPixmap::fromImage(image);
	return true;
}

bool SvgaResource::_parseMovie(const std::string& buffer)
{
	Json::Reader jsonReader;
	Json::Value jsonRoot;
	if (!jsonReader.parse(buffer, jsonRoot) || jsonRoot.isNull())
	{
		return false;
	}

	return m_video.parse(jsonRoot);
}
