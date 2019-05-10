#include "svgaresource.h"
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

void SvgaResource::clear()
{
	m_images.clear();
	m_video.clear();
}

QPixmap SvgaResource::getImage(const QString& key, SvgaPath& clipPath)
{
	QPixmap pix = m_images.value(key);
	if (clipPath.isValid())
	{
		QPixmap temp = QPixmap(pix.width(), pix.height());
		temp.fill(Qt::transparent);

		QPainter imagePainter(&temp);
		imagePainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

		imagePainter.setClipping(true);
		imagePainter.setClipPath(clipPath.getPath());
		imagePainter.drawPixmap(0, 0, pix);
		imagePainter.setClipping(false);
		imagePainter.end();

		pix = temp;
	}

	return pix;
}

SvgaVideoEntity* SvgaResource::getVideoEntity()
{
	return &m_video;
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
