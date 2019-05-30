#pragma once

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

	bool parse(const com::opensource::svga::SpriteEntity& obj);
	void clear();

	QString imageKey();
	SvgaVideoSpriteFrameEntity* frame(int index);

private:
	QString									m_imageKey;
	QVector<SvgaVideoSpriteFrameEntity*>	m_frames;
};
