#include "svgavideospriteentity.h"
#include "svgavideospriteframeentity.h"
#include "proto/svga.pb.h"

SvgaVideoSpriteEntity::SvgaVideoSpriteEntity()
{

}

SvgaVideoSpriteEntity::~SvgaVideoSpriteEntity()
{
	clear();
}

bool SvgaVideoSpriteEntity::parse(const com::opensource::svga::SpriteEntity& obj)
{
	m_imageKey = QString::fromStdString(obj.imagekey());

	for (int i = 0; i < obj.frames_size(); i++)
	{
		SvgaVideoSpriteFrameEntity* frame = new SvgaVideoSpriteFrameEntity;
		if (frame->parse(obj.frames(i)))
		{
			m_frames.push_back(frame);
		}
		else
		{
			delete frame;
			m_frames.push_back(NULL);
		}
	}

	return true;
}

void SvgaVideoSpriteEntity::clear()
{
	m_imageKey.clear();

	for (int i = 0; i < m_frames.size(); i++)
	{
		if (m_frames[i])
		{
			delete m_frames[i];
		}
	}
	m_frames.clear();
}

QString SvgaVideoSpriteEntity::imageKey()
{
	return m_imageKey;
}

SvgaVideoSpriteFrameEntity* SvgaVideoSpriteEntity::frame(int index)
{
	return m_frames.value(index);
}
