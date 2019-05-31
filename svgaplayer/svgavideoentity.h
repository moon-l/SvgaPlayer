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
			class MovieEntity;
		}
	}
}

class SvgaVideoSpriteEntity;

class SvgaVideoEntity
{
public:
	SvgaVideoEntity();
	~SvgaVideoEntity();

	bool parse(Json::Value& jsonObj);
	bool parse(const com::opensource::svga::MovieEntity& obj);
	bool parseImage(const std::string& buffer, const QString& name);
	void clear();

	bool valid();

	QString version();
	int width();
	int height();
	int fps();
	int frames();
	QPixmap getImage(const QString& key);

	QVector<SvgaVideoSpriteEntity*>& sprites();

private:
	bool	m_bValid;

	QString	m_version;
	int		m_width;
	int		m_height;
	int		m_fps;
	int		m_frames;

	QMap<QString, QPixmap>	m_images;

	QVector<SvgaVideoSpriteEntity*>	m_sprites;
};
