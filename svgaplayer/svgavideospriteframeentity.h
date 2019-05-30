#pragma once

namespace com
{
	namespace opensource
	{
		namespace svga
		{
			class FrameEntity;
		}
	}
}

class SvgaVideoSpriteFrameEntity
{
public:
	SvgaVideoSpriteFrameEntity();
	~SvgaVideoSpriteFrameEntity();

	bool parse(const com::opensource::svga::FrameEntity& obj);

	float alpha();
	QRect layout();
	QTransform& transform();
	QString clipPath();

private:
	float		m_alpha;
	QRect		m_layout;
	QTransform	m_transform;
	QString		m_clipPath;
};
