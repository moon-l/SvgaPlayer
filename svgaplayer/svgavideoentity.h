#pragma once

namespace Json
{
	class Value;
}
class SvgaVideoSpriteEntity;

class SvgaVideoEntity
{
public:
	SvgaVideoEntity();
	~SvgaVideoEntity();

	bool parse(Json::Value& jsonObj);
	void clear();

	QString version();
	int width();
	int height();
	int fps();
	int frames();

	QVector<SvgaVideoSpriteEntity*>& sprites();

private:
	QString	m_version;
	int		m_width;
	int		m_height;
	int		m_fps;
	int		m_frames;

	QVector<SvgaVideoSpriteEntity*>	m_sprites;
};
