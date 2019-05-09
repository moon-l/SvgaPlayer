#include "svgawidget.h"
#include "svgaresource.h" 
#include "gdisvgaplayer.h"

class SvgaWidgetPrivate
{
public:
	SvgaWidgetPrivate();
	~SvgaWidgetPrivate();

	void init();
	void release();
	bool loadSvgaFile();
	void updatePixmap();

private:
	SvgaWidget* q_ptr;
	Q_DECLARE_PUBLIC(SvgaWidget)
 
private:
	SvgaPlayer*		m_player;
	SvgaResource	m_svgaResource;
	QString			m_path;
	quint32			m_index;
	QPixmap			m_pixmap;	// 当前帧图像

	// property
	SvgaWidget::SvgaImageScalePolicy	m_scalePolicy;
	bool			m_bloop;

	QTimer			m_timer;
	QTimer			m_retryTimer;
	quint32			m_retryCounter;
};

SvgaWidgetPrivate::SvgaWidgetPrivate()
: m_index(0)
, m_scalePolicy(SvgaWidget::ScaleWidgetCenter)
, m_bloop(false)
, m_retryCounter(0)
{

}

SvgaWidgetPrivate::~SvgaWidgetPrivate()
{ 
	
}

void SvgaWidgetPrivate::init()
{
	m_player = new GDISvgaPlayer;
	m_player->init();
}

void SvgaWidgetPrivate::release()
{
	if (m_player)
	{
		m_player->release();
		delete m_player;
		m_player = NULL;
	}

	m_retryTimer.stop();
	m_timer.stop();
}

bool SvgaWidgetPrivate::loadSvgaFile()
{
	return m_svgaResource.load(m_path.toStdWString());
}

void SvgaWidgetPrivate::updatePixmap()
{
	if (m_player)
		m_player->draw(m_pixmap, &m_svgaResource, m_index);
}

SvgaWidget::SvgaWidget(QWidget *parent /*= NULL */)
: QWidget(parent)
{
	m_ptr = new SvgaWidgetPrivate();
	m_ptr->q_ptr = this;
	m_ptr->init();

	QObject::connect(&d_func()->m_timer, SIGNAL(timeout()), this, SLOT(on_timeout()),Qt::UniqueConnection);
	QObject::connect(&d_func()->m_retryTimer,SIGNAL(timeout()),this,SLOT(on_restart()),Qt::UniqueConnection);
}

SvgaWidget::~SvgaWidget()
{
	QObject::disconnect(this);

	if (m_ptr)
	{
		m_ptr->release();
		delete m_ptr;
	}
}

void SvgaWidget::play(const QString& path, int index)
{
	setImagePath(path);
	setFrameIndex(index);
	start();
}

bool SvgaWidget::start()
{
	Q_D(SvgaWidget);

	stop();
	if (!d->loadSvgaFile())
	{
		d->m_retryCounter = 0;
		d->m_retryTimer.start(300);
		return false;
	}

	d->m_timer.stop();
	if (d->m_svgaResource.getFps() == 0)
	{
		return false;
	}
	d->m_timer.start(1000 / d->m_svgaResource.getFps());

	d->updatePixmap();
	update();
	return true;
}

void SvgaWidget::on_restart()
{ 
	Q_D(SvgaWidget);

	if (d->m_retryCounter++ < 10)
	{
		if (!d->loadSvgaFile())
		{
			return;
		}

		if (d->m_svgaResource.getFps() == 0)
		{
			return;
		}
		d->m_timer.start(1000 / d->m_svgaResource.getFps());

		d->updatePixmap();
		update();
	}
	else
	{
		// 重试超时
		emit finished();
		emit startTimeout();
	}
	d->m_retryTimer.stop();
}

void SvgaWidget::stop()
{
	Q_D(SvgaWidget);
	
	SvgaResource temp;	// 不占用资源的引用
	d->m_svgaResource = temp;

	if (d->m_retryTimer.isActive())
	{
		d->m_retryTimer.stop();
	}
	if (d->m_timer.isActive())
	{
		d->m_timer.stop();
	}
}

bool SvgaWidget::isPlaying()
{
	Q_D(SvgaWidget);
	return d->m_timer.isActive() || d->m_retryTimer.isActive();
}

void SvgaWidget::on_timeout()
{
	Q_D(SvgaWidget);
	d->m_index = d->m_index + 1;
	if (d->m_index >= d->m_svgaResource.getFrameCount()) // end
	{
		if (d->m_bloop)
		{
			d->m_index = 0;
		}
		else
		{
			d->m_index = d->m_svgaResource.getFrameCount() - 1;	// last frame 
			stop();
			emit finished();
		}		
	}
	d->updatePixmap();
	update();
}

void SvgaWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	Q_D(SvgaWidget);

	QRectF rcImage = d->m_pixmap.rect();
	QRectF rect = this->rect();
	if (rect.width() <= 0 || rect.height() <= 0 || rcImage.width() <= 0 || rcImage.height() <= 0)
	{
		return;
	}

	switch (d->m_scalePolicy)
	{
	case ScaleWidgetCenter:
		{
			if (rect.width() != rcImage.width() || rect.height() != rcImage.height())
			{
				if (rcImage.width() * rect.height() > rcImage.height() * rect.width())
				{
					// scal with width
					int h = rcImage.height() * rect.width() / rcImage.width();
					rect.setTop((this->height() - h) / 2);
					rect.setHeight(h);
				}
				else
				{
					// scal with height
					int w = rcImage.width() * rect.height() / rcImage.height();
					rect.setLeft((this->width() - w) / 2);
					rect.setWidth(w);
				}
			}
		}
		break;
	case ScaleToLeftTop:
		{
			rect.setLeft(0);
			rect.setTop(0);
			if (rect.width() >= rcImage.width() && rect.height() >= rcImage.height())
			{
				rect.setWidth(rcImage.width());
				rect.setWidth(rcImage.height());
			}
			else
			{
				if (rcImage.width() * rect.height() > rcImage.height() * rect.width())
				{
					// scal with width
					rect.setHeight(rcImage.height() * rect.width() / rcImage.width());
				}
				else
				{
					// scal with height
					rect.setWidth(rcImage.width() * rect.height() / rcImage.height());
				}
			}
		}
		break;
	case WidgetCenter:
		{
			rect.setLeft((this->width() - rcImage.width()) / 2);
			rect.setTop((this->height() - rcImage.height()) / 2);
			rect.setWidth(rcImage.width());
			rect.setHeight(rcImage.height());
		}
		break;
	default:
		break;
	}

	painter.setRenderHint(QPainter::SmoothPixmapTransform, true); 
	painter.drawPixmap(rect, d->m_pixmap, rcImage);
}

void SvgaWidget::setFrameIndex(quint32 index)
{
	d_func()->m_index = index;
}

quint32 SvgaWidget::getFrameIndex()
{
	return d_func()->m_index;
}

void SvgaWidget::setImagePath(const QString &imageName)
{
	stop();
	d_func()->m_path = imageName;
}

QString SvgaWidget::getImagePath()
{
	return d_func()->m_path;
}

void SvgaWidget::setScalePolicy(SvgaImageScalePolicy scalePolicy)
{
	d_func()->m_scalePolicy = scalePolicy;
}

SvgaWidget::SvgaImageScalePolicy SvgaWidget::getScalePolicy()
{
	return d_func()->m_scalePolicy;
}

void SvgaWidget::setLoops(bool bLoop)
{
	d_func()->m_bloop = bLoop;
}

bool SvgaWidget::getLoops()
{
	return d_func()->m_bloop;
}
