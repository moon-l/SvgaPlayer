#include "svgagdicanvas.h"
#include "svgapath.h"

class SvgaGDICanvasPrivate
{
public:
	SvgaGDICanvasPrivate(SvgaGDICanvas* q);
	~SvgaGDICanvasPrivate();

	void begin();
	void end();

private:
	SvgaGDICanvas* q_ptr;
	Q_DECLARE_PUBLIC(SvgaGDICanvas)

private:
	SvgaGDICanvas::SvgaImageScalePolicy	m_scalePolicy;

	int			m_videoWidth;
	int			m_videoHeight;

	QPixmap		m_lastFrame;
	QPainter*	m_painter;
};

SvgaGDICanvasPrivate::SvgaGDICanvasPrivate(SvgaGDICanvas* q)
: q_ptr(q)
, m_scalePolicy(SvgaGDICanvas::ScaleWidgetCenter)
, m_videoWidth(0)
, m_videoHeight(0)
, m_painter(NULL)
{

}

SvgaGDICanvasPrivate::~SvgaGDICanvasPrivate()
{
	if (m_painter)
	{
		delete m_painter;
		m_painter = NULL;
	}
}

void SvgaGDICanvasPrivate::begin()
{
	if (m_videoWidth <= 0 || m_videoHeight <= 0)
	{
		return;
	}

	if (m_lastFrame.isNull() || m_lastFrame.width() != m_videoWidth || m_lastFrame.height() != m_videoHeight)
	{
		m_lastFrame = QPixmap(m_videoWidth, m_videoHeight);
	}

	m_lastFrame.fill(Qt::transparent); // clear image

	m_painter = new QPainter(&m_lastFrame);
	if (m_painter)
	{
		m_painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
	}
}

void SvgaGDICanvasPrivate::end()
{
	if (m_painter)
	{
		m_painter->end();
		delete m_painter;
		m_painter = NULL;
	}
}

SvgaGDICanvas::SvgaGDICanvas(QWidget* parent /*= NULL*/)
: QWidget(parent)
, m_ptr(new SvgaGDICanvasPrivate(this))
{

}

SvgaGDICanvas::~SvgaGDICanvas()
{
	if (m_ptr)
	{
		delete m_ptr;
		m_ptr = NULL;
	}
}

void SvgaGDICanvas::setScalePolicy(SvgaImageScalePolicy scalePolicy)
{
	Q_D(SvgaGDICanvas);
	d->m_scalePolicy = scalePolicy;
}

SvgaGDICanvas::SvgaImageScalePolicy SvgaGDICanvas::getScalePolicy()
{
	Q_D(SvgaGDICanvas);
	return d->m_scalePolicy;
}

void SvgaGDICanvas::setVideoSize(int width, int height)
{
	Q_D(SvgaGDICanvas);
	d->m_videoWidth = width;
	d->m_videoHeight = height;
}

void SvgaGDICanvas::begin()
{
	Q_D(SvgaGDICanvas);
	d->begin();
}

void SvgaGDICanvas::end()
{
	Q_D(SvgaGDICanvas);
	d->end();
	
	update();
}

void SvgaGDICanvas::draw(DrawItem* item)
{
	SvgaPath clipPath;
	clipPath.setPath(item->clipPath);
	QPixmap p = clipPath.clip(item->pix);

	Q_D(SvgaGDICanvas);
	if (d->m_painter)
	{
		d->m_painter->save();

		d->m_painter->setOpacity(item->alpha);
		d->m_painter->setTransform(item->transform);
		d->m_painter->drawPixmap(item->layout, p);

		d->m_painter->restore();
	}
}

void SvgaGDICanvas::paintEvent(QPaintEvent*)
{
	Q_D(SvgaGDICanvas);

	QPainter painter(this);

	QRectF rcImage = d->m_lastFrame.rect();
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
	painter.drawPixmap(rect, d->m_lastFrame, rcImage);
}
