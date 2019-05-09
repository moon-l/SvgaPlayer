#include "svgaresource.h"
#include "json/json.h"
#include "zip/unzipex.h"

SvgaResource::SvgaResource()
: m_width(0)
, m_height(0)
, m_fps(0)
, m_frameCounts(0)
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
			if (!_parseImage(buffer, name.toStdString()))
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
	m_version.clear();

	for(std::vector<SvgaSprite*>::iterator iter = m_sprites.begin(); iter != m_sprites.end(); iter++)
	{
		for(std::vector<SvgaFrame*>::iterator it = (*iter)->frames.begin(); it != (*iter)->frames.end(); it++)
		{
			if (*it != NULL)
			{
				delete (*it);
			}
		}

		delete (*iter);
	}
	m_sprites.clear();
	m_images.clear();

	m_width = 0;
	m_height = 0;
	m_fps = 0;
	m_frameCounts = 0;
}

std::string SvgaResource::getVersion()
{
	return m_version;
}

int SvgaResource::getWidth()
{
	return m_width;
}

int SvgaResource::getHeight()
{
	return m_height;
}

int SvgaResource::getFps()
{
	return m_fps;
}

int SvgaResource::getFrameCount()
{
	return m_frameCounts;
}

bool SvgaResource::getFrame(int index, std::vector<SvgaSingleFrame>& frames)
{
	if (index >= m_frameCounts)
		return false;

	for (int i = 0; i < m_sprites.size(); i++)
	{
		if (index < m_sprites[i]->frames.size())
		{
			SvgaSingleFrame frame;
			frame.name = m_sprites[i]->name;
			frame.frame = m_sprites[i]->frames[index];
			if (frame.frame)
				frames.push_back(frame);
		}
	}

	return true;
}

QPixmap SvgaResource::getImage(const std::string& key)
{
	std::map<std::string, QPixmap>::iterator iter = m_images.find(key + ".png");
	if (iter != m_images.end())
		return iter->second;
	else
		return QPixmap();
}

bool SvgaResource::_parseImage(const std::string& buffer, const std::string& name)
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

	m_version = jsonRoot["ver"].asString();
	
	Json::Value jsonMovie = jsonRoot["movie"];
	if (jsonMovie.isNull())
	{
		return false;
	}

	m_fps = jsonMovie["fps"].asInt();
	m_frameCounts = jsonMovie["frames"].asInt();

	Json::Value jsonViewBox = jsonMovie["viewBox"];
	if (jsonViewBox.isNull())
	{
		return false;
	}

	m_width = jsonViewBox["width"].asInt();
	m_height = jsonViewBox["height"].asInt();

	Json::Value jsonImages = jsonRoot["images"];
	Json::Value jsonSprites = jsonRoot["sprites"];
	if (jsonImages.isNull() || jsonSprites.isNull())
	{
		return false;
	}

	for (int i = 0; i < jsonSprites.size(); i++)
	{
		SvgaSprite* sprite = new SvgaSprite;
		
		std::string imageKey = jsonSprites[i]["imageKey"].asString();
		sprite->name = jsonImages[imageKey].asString();

		Json::Value jsonFrames = jsonSprites[i]["frames"];
		for (int j = 0; j < jsonFrames.size(); j++)
		{
			if (jsonFrames[j].isNull())
			{
				sprite->frames.push_back(NULL);
			}
			else
			{
				SvgaFrame* frame = new SvgaFrame;
				frame->alpha = jsonFrames[j]["alpha"].asFloat();

				int x, y, w, h;
				Json::Value& jsonLayout = jsonFrames[j]["layout"];
				if (!jsonLayout.isNull())
				{
					x = jsonLayout["x"].asInt();
					y = jsonLayout["y"].asInt();
					w = jsonLayout["width"].asInt();
					h = jsonLayout["height"].asInt();
					frame->layout = QRect(x, y, w, h);
				}

				float a, b, c, d, tx, ty;
				Json::Value& jsonTransform = jsonFrames[j]["transform"];
				if (jsonTransform.isNull())
				{
					a = d = 1.0f;
					b = c = tx = ty = 0;
				}
				else
				{
					a = jsonTransform["a"].asFloat();
					b = jsonTransform["b"].asFloat();
					c = jsonTransform["c"].asFloat();
					d = jsonTransform["d"].asFloat();
					tx = jsonTransform["tx"].asFloat();
					ty = jsonTransform["ty"].asFloat();
				}
				frame->transform.setMatrix(a, b, c, d, tx, ty);

				Json::Value& jsonClipPath = jsonFrames[j]["clipPath"];
				if (!jsonClipPath.isNull())
				{
					frame->clipPath = QString::fromStdString(jsonClipPath.asString());
				}

				sprite->frames.push_back(frame);
			}
		}

		m_sprites.push_back(sprite);
	}

	return true;
}
