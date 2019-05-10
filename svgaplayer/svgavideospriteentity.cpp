#include "svgavideospriteentity.h"
#include "svgavideospriteframeentity.h"
#include "json/json.h"

SvgaVideoSpriteEntity::SvgaVideoSpriteEntity()
{

}

SvgaVideoSpriteEntity::~SvgaVideoSpriteEntity()
{
	clear();
}

bool SvgaVideoSpriteEntity::parse(Json::Value& jsonObj, Json::Value& jsonImages)
{
	std::string imageKey = jsonObj["imageKey"].asString();
	m_imageKey = QString::fromStdString(jsonImages[imageKey].asString()) + ".png";

	Json::Value jsonFrames = jsonObj["frames"];
	for (int i = 0; i < jsonFrames.size(); i++)
	{
		SvgaVideoSpriteFrameEntity* frame = new SvgaVideoSpriteFrameEntity;
		if (frame->parse(jsonFrames[i]))
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
