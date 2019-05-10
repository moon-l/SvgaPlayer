#include "svgavideospriteframeentity.h"
#include "json/json.h"

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

	int x, y, w, h;
	Json::Value& jsonLayout = jsonObj["layout"];
	if (!jsonLayout.isNull())
	{
		x = jsonLayout["x"].asInt();
		y = jsonLayout["y"].asInt();
		w = jsonLayout["width"].asInt();
		h = jsonLayout["height"].asInt();
		m_layout = QRect(x, y, w, h);
	}

	float a, b, c, d, tx, ty;
	Json::Value& jsonTransform = jsonObj["transform"];
	if (jsonTransform.isNull())
	{
		a = d = 1.0f;
		b = c = tx = ty = 0;
	}
	else
	{
		a = jsonTransform["a"].asFloat();
		b = jsonTransform["b"].asFloat();
		c = jsonTransform["c"].asFloat();
		d = jsonTransform["d"].asFloat();
		tx = jsonTransform["tx"].asFloat();
		ty = jsonTransform["ty"].asFloat();
	}
	m_transform = QTransform(a, b, c, d, tx, ty);

	Json::Value& jsonClipPath = jsonObj["clipPath"];
	if (!jsonClipPath.isNull())
	{
		m_clipPath.setPath(QString::fromStdString(jsonClipPath.asString()));
	}

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

SvgaPath& SvgaVideoSpriteFrameEntity::clipPath()
{
	return m_clipPath;
}
