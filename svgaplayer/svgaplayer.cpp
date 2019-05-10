#include "svgaplayer.h"
#include "svgaresource.h" 
#include "svgavideoentity.h"
#include "svgavideospriteentity.h"
#include "svgavideospriteframeentity.h"

class SvgaPlayerPrivate
{
public:
	SvgaPlayerPrivate(SvgaPlayer* q);
	~SvgaPlayerPrivate();

	bool loadSvgaFile();
	void setCanvas(SvgaCanvas* canvas);
	void draw();
	int fps();
	int frames();

private:
	SvgaPlayer* q_ptr;
	Q_DECLARE_PUBLIC(SvgaPlayer)
 
private:
	SvgaCanvas*		m_canvas;
	SvgaResource	m_resource;
	QString			m_path;
	quint32			m_index;

	bool			m_bloop;

	QTimer			m_timer;
};

SvgaPlayerPrivate::SvgaPlayerPrivate(SvgaPlayer* q)
: q_ptr(q)
, m_index(0)
, m_bloop(false)
, m_canvas(NULL)
{
	QObject::connect(&m_timer, SIGNAL(timeout()), q_ptr, SLOT(on_timeout()));
}

SvgaPlayerPrivate::~SvgaPlayerPrivate()
{
	m_timer.stop();
}

bool SvgaPlayerPrivate::loadSvgaFile()
{
	bool ret = m_resource.load(m_path.toStdWString());
	setCanvas(m_canvas);

	return ret;
}

void SvgaPlayerPrivate::setCanvas(SvgaCanvas* canvas)
{
	m_canvas = canvas;
	if (m_canvas)
	{
		SvgaVideoEntity* video = m_resource.getVideoEntity();
		if (video)
		{
			m_canvas->setVideoSize(video->width(), video->height());
		}
	}
}

void SvgaPlayerPrivate::draw()
{
	if (m_canvas)
	{
		SvgaVideoEntity* video = m_resource.getVideoEntity();
		if (!video)
			return;

		m_canvas->begin();

		QVector<SvgaVideoSpriteEntity*>& sprites = video->sprites();
		for (int i = 0; i < sprites.size(); i++)
		{
			SvgaVideoSpriteFrameEntity* item = sprites[i]->frame(m_index);
			if (item)
			{
				m_canvas->draw(m_resource.getImage(sprites[i]->imageKey(), item->clipPath()), item->layout(), item->transform(), item->alpha());
			}
		}

		m_canvas->end();
	}
}

int SvgaPlayerPrivate::fps()
{
	SvgaVideoEntity* video = m_resource.getVideoEntity();
	if (video)
	{
		return video->fps();
	}

	return 0;
}

int SvgaPlayerPrivate::frames()
{
	SvgaVideoEntity* video = m_resource.getVideoEntity();
	if (video)
	{
		return video->frames();
	}

	return 0;
}

SvgaPlayer::SvgaPlayer(QObject *parent /*= NULL */)
: QObject(parent)
, m_ptr(new SvgaPlayerPrivate(this))
{

}

SvgaPlayer::~SvgaPlayer()
{
	if (m_ptr)
	{
		delete m_ptr;
		m_ptr = NULL;
	}
}

void SvgaPlayer::play(const QString& path, int index)
{
	setImagePath(path);
	setFrameIndex(index);
	start();
}

bool SvgaPlayer::start()
{
	Q_D(SvgaPlayer);

	stop();
	if (!d->loadSvgaFile())
	{
		return false;
	}

	d->m_timer.stop();
	if (d->fps() == 0)
	{
		return false;
	}
	d->m_timer.start(1000 / d->fps());

	d->draw();
	return true;
}

void SvgaPlayer::stop()
{
	Q_D(SvgaPlayer);

	d->m_resource.clear(); // ��ռ����Դ������

	if (d->m_timer.isActive())
	{
		d->m_timer.stop();
	}
}

bool SvgaPlayer::isPlaying()
{
	Q_D(SvgaPlayer);
	return d->m_timer.isActive();
}

void SvgaPlayer::on_timeout()
{
	Q_D(SvgaPlayer);
	d->m_index = d->m_index + 1;
	if (d->m_index >= d->frames()) // end
	{
		if (d->m_bloop)
		{
			d->m_index = 0;
		}
		else
		{
			d->m_index = d->frames() - 1;	// last frame 
			stop();
			emit finished();
		}		
	}

	d->draw();
}

void SvgaPlayer::setFrameIndex(quint32 index)
{
	Q_D(SvgaPlayer);
	d->m_index = index;
}

quint32 SvgaPlayer::getFrameIndex()
{
	Q_D(SvgaPlayer);
	return d->m_index;
}

void SvgaPlayer::setImagePath(const QString &imageName)
{
	Q_D(SvgaPlayer);
	stop();
	d->m_path = imageName;
}

QString SvgaPlayer::getImagePath()
{
	Q_D(SvgaPlayer);
	return d->m_path;
}

void SvgaPlayer::setLoops(bool bLoop)
{
	Q_D(SvgaPlayer);
	d->m_bloop = bLoop;
}

bool SvgaPlayer::getLoops()
{
	Q_D(SvgaPlayer);
	return d->m_bloop;
}

void SvgaPlayer::setCanvas(SvgaCanvas* canvas)
{
	Q_D(SvgaPlayer);
	d->setCanvas(canvas);
}