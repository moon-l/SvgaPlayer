#include "svgavideoentity.h"
#include "svgavideospriteentity.h"
#include "proto/svga.pb.h"

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

bool SvgaVideoEntity::parse(const com::opensource::svga::MovieEntity& obj)
{
	m_version = QString::fromStdString(obj.version());

	m_width = obj.params().viewboxwidth();
	m_height = obj.params().viewboxheight();
	m_fps = obj.params().fps();
	m_frames = obj.params().frames();

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
