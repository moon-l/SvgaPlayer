#pragma once

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
	QString clipPath();

private:
	float		m_alpha;
	QRect		m_layout;
	QTransform	m_transform;
	QString		m_clipPath;
};
