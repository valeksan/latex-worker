#ifndef QQRCODEREADER_H
#define QQRCODEREADER_H

#include <QObject>

#include <opencv2/opencv.hpp>
#include <cv.h>
#include <zbar.h>

#include <QDebug>

//#define DEF_DEBUG_VIEW_MODIFIED_IMAGE

class QQrCodeReader : public QObject
{
	Q_OBJECT
public:
	explicit QQrCodeReader(QObject *parent = nullptr);

	static bool scan(const QString fileName, QList<QString> &outlist, double kx = 1.0, bool adaptiveThreshold = true);

private:

signals:

public slots:
};

#endif // QQRCODEREADER_H
