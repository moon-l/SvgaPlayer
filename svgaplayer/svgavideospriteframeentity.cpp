#include "svgavideospriteframeentity.h"
#include "proto/svga.pb.h"

SvgaVideoSpriteFrameEntity::SvgaVideoSpriteFrameEntity()
: m_alpha(0)
{

}

SvgaVideoSpriteFrameEntity::~SvgaVideoSpriteFrameEntity()
{

}

bool SvgaVideoSpriteFrameEntity::parse(const com::opensource::svga::FrameEntity& obj)
{
	if (!obj.has_layout())
	{
		return false;
	}

	m_alpha = obj.alpha();

	float x = obj.layout().x();
	float y = obj.layout().y();
	float w = obj.layout().width();
	float h = obj.layout().height();
	m_layout = QRect(x, y, w, h);

	float a = obj.transform().a();
	float b = obj.transform().b();
	float c = obj.transform().c();
	float d = obj.transform().d();
	float tx = obj.transform().tx();
	float ty = obj.transform().ty();
	m_transform = QTransform(a, b, c, d, tx, ty);

	m_clipPath = QString::fromStdString(obj.clippath());

	return true;
}

float SvgaVideoSpriteFrameEntity::alpha()
{
	return m_alpha;
}

QRect SvgaVideoSpriteFrameEntity::layout()
{
	return m_layout;
}

QTransform& SvgaVideoSpriteFrameEntity::transform()
{
	return m_transform;
}

QString SvgaVideoSpriteFrameEntity::clipPath()
{
	return m_clipPath;
}
