#pragma once

namespace Json
{
	class Value;
}

class SvgaVideoSpriteFrameEntity;

class SvgaVideoSpriteEntity
{
public:
	SvgaVideoSpriteEntity();
	~SvgaVideoSpriteEntity();

	bool parse(Json::Value& jsonObj, Json::Value& jsonImages);
	void clear();

	QString imageKey();
	SvgaVideoSpriteFrameEntity* frame(int index);

private:
	QString									m_imageKey;
	QVector<SvgaVideoSpriteFrameEntity*>	m_frames;
};
