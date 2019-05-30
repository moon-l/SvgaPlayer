#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
#include <fstream>
#include "svgaresource.h"
#include "proto/svga.pb.h"

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

void SvgaResource::_parseImage(const com::opensource::svga::MovieEntity& obj)
{
	for (google::protobuf::Map<std::string, std::string>::const_iterator iter = obj.images().begin(); iter != obj.images().end(); iter++)
	{
		QString key = QString::fromStdString(iter->first);
		QImage image = QImage::fromData((const uchar*)iter->second.c_str(), iter->second.size());
		m_images[key] = QPixmap::fromImage(image);
	}
}
