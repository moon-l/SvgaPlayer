#pragma once

namespace Json
{
	class Value;
}

namespace com
{
	namespace opensource
	{
		namespace svga
		{
			class SpriteEntity;
		}
	}
}

class SvgaVideoSpriteFrameEntity;

class SvgaVideoSpriteEntity
{
public:
	SvgaVideoSpriteEntity();
	~SvgaVideoSpriteEntity();

	bool parse(Json::Value& jsonObj, Json::Value& jsonImages);
	bool parse(const com::opensource::svga::SpriteEntity& obj);
	void clear();

	QString imageKey();
	SvgaVideoSpriteFrameEntity* frame(int index);

private:
	QString									m_imageKey;
	QVector<SvgaVideoSpriteFrameEntity*>	m_frames;
};
