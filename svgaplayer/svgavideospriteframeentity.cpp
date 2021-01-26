#include "svgavideospriteframeentity.h"
#include "json/json.h"
#include "proto/svga.pb.h"

SvgaVideoSpriteFrameEntity::SvgaVideoSpriteFrameEntity()
: m_alpha(0)
{

}

SvgaVideoSpriteFrameEntity::~SvgaVideoSpriteFrameEntity()
{

}

bool SvgaVideoSpriteFrameEntity::parse(Json::Value& jsonObj)
{
	if (jsonObj.isNull())
	{
		return false;
	}

	m_alpha = jsonObj["alpha"].asFloat();

	int x = 0, y = 0, w = 0, h = 0;
	Json::Value& jsonLayout = jsonObj["layout"];
	if (!jsonLayout.isNull())
	{
		x = jsonLayout["x"].asInt();
		y = jsonLayout["y"].asInt();
		w = jsonLayout["width"].asInt();
		h = jsonLayout["height"].asInt();
	}
	m_layout = QRect(x, y, w, h);

	float a, b, c, d, tx, ty;
	Json::Value& jsonTransform = jsonObj["transform"];
	if (jsonTransform.isNull())
	{
		a = d = 1.0f;
		b = c = tx = ty = 0.0f;
	}
	else
	{
		a = jsonTransform["a"].asFloat();
		b = jsonTransform["b"].asFloat();
		c = jsonTransform["c"].asFloat();
		d = jsonTransform["d"].asFloat();
		tx = jsonTransform["tx"].asFloat();
		ty = jsonTransform["ty"].asFloat();

		if (fabs(a) < 1e-6)
		{
			a = 1.0f;
		}

		if (fabs(d) == 0.0f)
		{
			d = 1.0f;
		}
	}
	m_transform = QTransform(a, b, c, d, tx, ty);

	Json::Value& jsonClipPath = jsonObj["clipPath"];
	if (!jsonClipPath.isNull())
	{
		m_clipPath = QString::fromStdString(jsonClipPath.asString());
	}

	return true;
}

bool SvgaVideoSpriteFrameEntity::parse(const com::opensource::svga::FrameEntity& obj)
{
	m_alpha = obj.alpha();

	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	if (obj.has_layout())
	{
		x = obj.layout().x();
		y = obj.layout().y();
		w = obj.layout().width();
		h = obj.layout().height();
	}
	m_layout = QRect(x, y, w, h);

	float a = 1.0f;
	float b = 0.0f;
	float c = 0.0f;
	float d = 1.0f;
	float tx = 0.0f;
	float ty = 0.0f;
	if (obj.has_transform())
	{
		a = obj.transform().a();
		b = obj.transform().b();
		c = obj.transform().c();
		d = obj.transform().d();
		tx = obj.transform().tx();
		ty = obj.transform().ty();

		if (fabs(a) < 1e-6)
		{
			a = 1.0f;
		}

		if (fabs(d) == 0.0f)
		{
			d = 1.0f;
		}
	}
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
