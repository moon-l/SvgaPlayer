#pragma once

#include "svgapath.h"

namespace Json
{
	class Value;
}

class SvgaVideoSpriteFrameEntity
{
public:
	SvgaVideoSpriteFrameEntity();
	~SvgaVideoSpriteFrameEntity();

	bool parse(Json::Value& jsonObj);

	float alpha();
	QRect layout();
	QTransform& transform();
	SvgaPath& clipPath();

private:
	float		m_alpha;
	QRect		m_layout;
	QTransform	m_transform;
	SvgaPath	m_clipPath;
};
