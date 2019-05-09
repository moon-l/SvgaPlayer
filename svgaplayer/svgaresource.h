#pragma once

struct SvgaFrame
{
	float	alpha;
	QRect	layout;
	QMatrix	transform;
	QString	clipPath;
};

struct SvgaSprite
{
	std::string				name;
	std::vector<SvgaFrame*>	frames;
};

struct SvgaSingleFrame
{
	std::string	name;
	SvgaFrame*	frame;
};

class SvgaResource
{
public:
	SvgaResource();
	~SvgaResource();

	bool load(const std::wstring& path);
	void clear();

	std::string getVersion();
	int getWidth();
	int getHeight();
	int getFps();
	int getFrameCount();
	bool getFrame(int index, std::vector<SvgaSingleFrame>& frames);
	QPixmap getImage(const std::string& key);

private:
	bool _parseImage(const std::string& buffer, const std::string& name);
	bool _parseMovie(const std::string& buffer);

private:
	std::map<std::string, QPixmap>	m_images;
	std::vector<SvgaSprite*>		m_sprites;

	std::string						m_version;
	int								m_fps;
	int								m_width;
	int								m_height;
	int								m_frameCounts;
};
