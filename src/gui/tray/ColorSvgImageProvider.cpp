#include <QPainter>
#include <QLoggingCategory>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QPalette>
#include <QSvgRenderer>
#include <QNetworkReply>
#include <qpalette.h>

#include "ColorSvgImageProvider.h"

namespace OCC {

Q_LOGGING_CATEGORY(lcColorSvgImageProvider, "nextcloud.gui.tray.colorsvgimageprovider", QtInfoMsg)

ColorSvgImageProvider::ColorSvgImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}


QImage ColorSvgImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // TODO: Clean up
    if (size) {
        *size = requestedSize;
    }
    qCDebug(lcColorSvgImageProvider) << "Load" << id;
    QSvgRenderer svgRenderer;
    if (id.startsWith("data")) {
        const QString prefix("data:image/svg+xml;utf8,");
        Q_ASSERT(id.startsWith(prefix));
        const auto svgData = QUrl::fromPercentEncoding(id.toLocal8Bit()).mid(prefix.size()).toLocal8Bit();
        Q_ASSERT(svgRenderer.load(svgData));
    } else if (id.startsWith("http")) {
        const auto url = QUrl::fromUserInput(id);
        auto networkAccessManager = new QNetworkAccessManager();
        auto reply = networkAccessManager->get(QNetworkRequest(url));

        // TODO: This loop is a little bit ugly but had no better idea yet to do synchronious network request
        // I think doing a syncronious network request is fine because I hope that requestImage() gets executed async.
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::readyRead, [&] {
            qCDebug(lcColorSvgImageProvider) << "Loaded http";
            svgRenderer.load(reply->readAll());
            loop.quit();
        });
        QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
        loop.exec();
    } else {
        const QString realId = QLatin1Char(':') + id;
        Q_ASSERT(svgRenderer.load(realId));
    }
    QImage svgImage(requestedSize.width(), requestedSize.height(), QImage::Format_ARGB32);
    QPainter svgImagePainter(&svgImage);
    svgImage.fill(Qt::GlobalColor::transparent);
    svgRenderer.render(&svgImagePainter);
    svgImagePainter.end();

    QImage image(requestedSize.width(), requestedSize.height(), QImage::Format_ARGB32);
    image.fill(_palette.windowText().color());
    QPainter imagePainter(&image);
    imagePainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    imagePainter.drawImage(0, 0, svgImage);
    imagePainter.end();

    return image;
}

class AsyncImageResponseRunnable : public QObject, public QRunnable
{
    Q_OBJECT

signals:
    void done(QImage image);

public:
    AsyncImageResponseRunnable(const QString &id, const QSize &requestedSize)
        : _id(id)
        , _requestedSize(requestedSize)
    {
    }

    void run() override
    {
        qCDebug(lcColorSvgImageProvider) << "Load" << _id;
        // QSvgRenderer svgRenderer;
        // if (_id.startsWith("data")) {
        //     const QString prefix("data:image/svg+xml;utf8,");
        //     Q_ASSERT(_id.startsWith(prefix));
        //     const auto svgData = QUrl::fromPercentEncoding(_id.toLocal8Bit()).mid(prefix.size()).toLocal8Bit();
        //     Q_ASSERT(svgRenderer.load(svgData));
        // } else if (_id.startsWith("http")) {
        //     // const auto url = QUrl::fromUserInput(_id);
        //     // auto networkAccessManager = new QNetworkAccessManager();
        //     // auto reply = networkAccessManager->get(QNetworkRequest(url));

        //     // // TODO: This loop is a little bit ugly but had no better idea yet to do synchronious network request
        //     // // I think doing a syncronious network request is fine because I hope that requestImage() gets executed async.
        //     // QEventLoop loop;
        //     // QObject::connect(reply, &QNetworkReply::readyRead, [&] {
        //     //     qCDebug(lcColorSvgImageProvider) << "Loaded http";
        //     //     svgRenderer.load(reply->readAll());
        //     //     loop.quit();
        //     // });
        //     // QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
        //     // loop.exec();

        //     auto image = QImage(_requestedSize.width(), _requestedSize.height(), QImage::Format_ARGB32);
        //     image.fill(Qt::red);
        //     emit done(image);
        //     return;
        // } else {
        //     const QString realId = QLatin1Char(':') + _id;
        //     Q_ASSERT(svgRenderer.load(realId));
        // }
        // QImage svgImage(_requestedSize.width(), _requestedSize.height(), QImage::Format_ARGB32);
        // QPainter svgImagePainter(&svgImage);
        // svgImage.fill(Qt::GlobalColor::transparent);
        // svgRenderer.render(&svgImagePainter);
        // svgImagePainter.end();

        // Q_ASSERT(_requestedSize.isValid());
        // QImage image(_requestedSize.width(), _requestedSize.height(), QImage::Format_ARGB32);
        // image.fill(_palette.windowText().color());
        // QPainter imagePainter(&image);
        // imagePainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        // imagePainter.drawImage(0, 0, svgImage);
        // imagePainter.end();
        QThread::sleep(1);

        auto image = QImage(_requestedSize.width(), _requestedSize.height(), QImage::Format_ARGB32);
        if (_id == QLatin1String("slow")) {
            qDebug() << "Slow, red, sleeping for 5 seconds";
            image.fill(Qt::red);
        } else {
            qDebug() << "Fast, blue, sleeping for 1 second";
            image.fill(Qt::blue);
        }

        qCDebug(lcColorSvgImageProvider) << "Done";
        emit done(image);
    }

private:
    QString _id;
    QPalette _palette;
    QSize _requestedSize;
};


class AsyncImageResponse : public QQuickImageResponse
{
public:
    AsyncImageResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool)
    {
        auto runnable = new AsyncImageResponseRunnable(id, requestedSize);
        connect(runnable, &AsyncImageResponseRunnable::done, this, &AsyncImageResponse::handleDone);
        pool->start(runnable);
    }

    void handleDone(QImage image)
    {
        m_image = image;
        emit finished();
    }

    QQuickTextureFactory *textureFactory() const override
    {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    QImage m_image;
};

QQuickImageResponse *AsyncColorSvgImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    return new AsyncImageResponse(id, requestedSize, &_pool);
}
}

#include "ColorSvgImageProvider.moc"
