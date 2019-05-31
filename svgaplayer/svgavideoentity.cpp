#include "svgavideoentity.h"
#include "svgavideospriteentity.h"
#include "json/json.h"
#include "proto/svga.pb.h"

SvgaVideoEntity::SvgaVideoEntity()
: m_fps(0)
, m_frames(0)
, m_width(0)
, m_height(0)
, m_bValid(false)
{

}

SvgaVideoEntity::~SvgaVideoEntity()
{
	clear();
}

bool SvgaVideoEntity::parse(Json::Value& jsonObj)
{
	m_version = QString::fromStdString(jsonObj["ver"].asString());

	Json::Value jsonMovie = jsonObj["movie"];
	if (jsonMovie.isNull())
	{
		return false;
	}

	m_fps = jsonMovie["fps"].asInt();
	m_frames = jsonMovie["frames"].asInt();

	Json::Value jsonViewBox = jsonMovie["viewBox"];
	if (jsonViewBox.isNull())
	{
		return false;
	}

	m_width = jsonViewBox["width"].asInt();
	m_height = jsonViewBox["height"].asInt();

	Json::Value jsonImages = jsonObj["images"];
	Json::Value jsonSprites = jsonObj["sprites"];
	if (jsonImages.isNull() || jsonSprites.isNull())
	{
		return false;
	}

	for (int i = 0; i < jsonSprites.size(); i++)
	{
		SvgaVideoSpriteEntity* sprite = new SvgaVideoSpriteEntity;
		if (sprite->parse(jsonSprites[i], jsonImages))
		{
			m_sprites.push_back(sprite);
		}
		else
		{
			delete sprite;
		}
	}

	m_bValid = true;

	return true;
}

bool SvgaVideoEntity::parse(const com::opensource::svga::MovieEntity& obj)
{
	m_version = QString::fromStdString(obj.version());

	m_width = obj.params().viewboxwidth();
	m_height = obj.params().viewboxheight();
	m_fps = obj.params().fps();
	m_frames = obj.params().frames();

	for (google::protobuf::Map<std::string, std::string>::const_iterator iter = obj.images().begin(); iter != obj.images().end(); iter++)
	{
		QString key = QString::fromStdString(iter->first);
		parseImage(iter->second, key);
	}

	for (int i = 0; i < obj.sprites_size(); i++)
	{
		SvgaVideoSpriteEntity* sprite = new SvgaVideoSpriteEntity;
		if (sprite->parse(obj.sprites(i)))
		{
			m_sprites.push_back(sprite);
		}
		else
		{
			delete sprite;
		}
	}

	m_bValid = true;

	return true;
}

bool SvgaVideoEntity::parseImage(const std::string& buffer, const QString& name)
{
	QImage image = QImage::fromData((const uchar*)buffer.c_str(), buffer.size());
	if (image.isNull())
	{
		return false;
	}

	m_images[name] = QPixmap::fromImage(image);
	return true;
}

void SvgaVideoEntity::clear()
{
	m_fps = 0;
	m_frames = 0;
	m_width = 0;
	m_height = 0;

	for (int i = 0; i < m_sprites.size(); i++)
	{
		delete m_sprites[i];
	}
	m_sprites.clear();

	m_images.clear();

	m_bValid = false;
}

bool SvgaVideoEntity::valid()
{
	return m_bValid;
}

QString SvgaVideoEntity::version()
{
	return m_version;
}

int SvgaVideoEntity::width()
{
	return m_width;
}

int SvgaVideoEntity::height()
{
	return m_height;
}

int SvgaVideoEntity::fps()
{
	return m_fps;
}

int SvgaVideoEntity::frames()
{
	return m_frames;
}

QPixmap SvgaVideoEntity::getImage(const QString& key)
{
	return m_images.value(key);
}

QVector<SvgaVideoSpriteEntity*>& SvgaVideoEntity::sprites()
{
	return m_sprites;
}
