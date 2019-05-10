#include "svgavideoentity.h"
#include "svgavideospriteentity.h"
#include "json/json.h"

SvgaVideoEntity::SvgaVideoEntity()
: m_fps(0)
, m_frames(0)
, m_width(0)
, m_height(0)
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

QVector<SvgaVideoSpriteEntity*>& SvgaVideoEntity::sprites()
{
	return m_sprites;
}
