#pragma once

#include <QObject>
#include "svgadef.h"

class SvgaPlayerPrivate;

class SVGA_EXPORT SvgaPlayer : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(SvgaPlayer)
	
public:
	explicit SvgaPlayer(QObject *parent = NULL);
	~SvgaPlayer();

	void play(const QString& path, int index = 0);
	bool start();
	void stop();

	bool isPlaying();

	void setFrameIndex(quint32 index);
	quint32 getFrameIndex();

	void setImagePath(const QString &imageName);
	QString getImagePath();

	void setLoops(bool bLoop);
	bool getLoops();

	void setCanvas(SvgaCanvas* canvas);

	QSize getItemSize(const QString& key);
	bool addDynamicItem(const QString& key, QPixmap& image);
	void removeDynamicItem(const QString& key);
	void clearAllDynamicItems();

signals:
	void finished();

protected slots:
	void on_timeout();

private:
	SvgaPlayerPrivate*	m_ptr;
	Q_DECLARE_PRIVATE_D(m_ptr, SvgaPlayer)
};
