#include "logooverlay.h"
#include <QPen>
#include <QDebug>

int LogoOverlay::smallerDimension()
{
    return qMin(videoSize.width(), videoSize.height());
}

int LogoOverlay::largerDimension()
{
    return qMax(videoSize.width(), videoSize.height());
}

LogoOverlay::LogoOverlay(QGraphicsPixmapItem *video, QPixmap logo, double scale, double padding, double opacity) :
        video(video), videoSize(video->pixmap().size()), padding(padding), scale(scale), logoPos(0)
{
    this->logo = logo;
    logoImage = new QGraphicsPixmapItem(this->logo.scaledToHeight(getScaledSize(), Qt::SmoothTransformation), video);
    logoImage->setOffset({qRound(padding * smallerDimension()), qRound(padding * smallerDimension())});
    logoImage->setOpacity(opacity / 100);

    auto p0 = getTopLeftCorner(true, true);

    auto rect0 = new PlaceholderSquare(
            p0.x(),
            p0.y(),
            getScaledSize(),
            getScaledSize(),
            video
    );
    placeholders[0] = rect0;

    rect0->setVisible(false);

    auto p1 = getTopLeftCorner(false, true);
    auto rect1 = new PlaceholderSquare(
            p1.x(),
            p1.y(),
            getScaledSize(),
            getScaledSize(),
            video
    );
    placeholders[1] = rect1;


    auto p2 = getTopLeftCorner(true, false);
    auto rect2 = new PlaceholderSquare(
            p2.x(),
            p2.y(),
            getScaledSize(),
            getScaledSize(),
            video
    );
    placeholders[2] = rect2;


    auto p3 = getTopLeftCorner(false, false);
    auto rect3 = new PlaceholderSquare(
            p3.x(),
            p3.y(),
            getScaledSize(),
            getScaledSize(),
            video
    );
    placeholders[3] = rect3;


    for (int i = 0; i < 4; i++)
    {
        auto rect = placeholders[i];
        connect(
                rect, &PlaceholderSquare::clicked, [=]() {
                    moveLogo(i);
                }
        );
    }
}

void LogoOverlay::setShowPlaceholders(bool show){
    if(show == false){
        for(auto x : placeholders)
            x->hide();
    }
    else{
        for(int i = 0; i < 4; i++){
            if(i != logoPos){
                placeholders[i]->show();
            }
        }
    }
}

void LogoOverlay::moveLogo(int pos)
{
    if (pos == logoPos)
    {
        return;
    }
    placeholders[logoPos]->setVisible(true);
    placeholders[pos]->setVisible(false);
    logoPos = pos;
    resetLogoPos();
}


QPoint LogoOverlay::getTopLeftCorner(bool left, bool top)
{
    QPoint point;
    if (left)
        point.setX(getPadding());
    else
        point.setX(videoSize.width() - 3 - getPadding() - getScaledSize());
    if (top)
        point.setY(getPadding());
    else
        point.setY(videoSize.height() - 3 - getPadding() - getScaledSize());
    return point;
}

QPoint LogoOverlay::getTopLeftCornerLogo(bool left, bool top)
{
    QPoint point;
    if (left)
        point.setX(getPadding());
    else
        point.setX(videoSize.width() - getPadding() - logoImage->pixmap().width() - 3);
    if (top)
        point.setY(getPadding());
    else
        point.setY(videoSize.height() - getPadding() - logoImage->pixmap().height() - 3);
    return point;
}

int LogoOverlay::getScaledSize()
{
    return (int) qRound(scale * smallerDimension());
}

int LogoOverlay::getPadding()
{
    return (int) qRound(padding * smallerDimension());
}

int LogoOverlay::getLogoPos()
{
    return logoPos;
}

void LogoOverlay::paddingChanged(double newVal)
{
    newVal /= 100;
    this->padding = newVal;
    QPoint points[] = {getTopLeftCorner(true, true),
                       getTopLeftCorner(false, true),
                       getTopLeftCorner(true, false),
                       getTopLeftCorner(false, false)};
    for (auto &p : points)
    {
        qDebug() << p;
    }
    for (int i = 0; i < 4; i++)
    {
        auto rectView = placeholders[i];
        QRectF rect(points[i].x(), points[i].y(), getScaledSize(), getScaledSize());
        rectView->setRect(rect);
    }
    resetLogoPos();
}

void LogoOverlay::resetLogoPos()
{
    bool top = (logoPos / 2) == 0;
    bool left = (logoPos % 2) == 0;
    QPoint point = getTopLeftCornerLogo(left, top);
    qDebug() << point;
    logoImage->setOffset(point);
}

void LogoOverlay::scaleChanged(double newVal)
{
    newVal /= 100;
    this->scale = newVal;
    logoImage->setPixmap(logo.scaled(getScaledSize(), getScaledSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QPoint points[] = {getTopLeftCorner(true, true),
                       getTopLeftCorner(false, true),
                       getTopLeftCorner(true, false),
                       getTopLeftCorner(false, false)};
    for (auto &p : points)
    {
        qDebug() << p;
    }
    for (int i = 0; i < 4; i++)
    {
        auto rectView = placeholders[i];
        QRectF rect(points[i].x(), points[i].y(), getScaledSize(), getScaledSize());
        qDebug() << rect;
        rectView->setRect(rect);
    }
    resetLogoPos();
}

void LogoOverlay::logoChanged(QPixmap newVal)
{
    logo = newVal;
    logoImage->setPixmap(logo.scaled(getScaledSize(), getScaledSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    resetLogoPos();
}

void LogoOverlay::opacityChanged(double newVal)
{
    logoImage->setOpacity(newVal / 100);
}

void LogoOverlay::videoChanged()
{
    videoSize = video->pixmap().size();
    logoImage->setPixmap(logo.scaled(getScaledSize(), getScaledSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QPoint points[] = {getTopLeftCorner(true, true),
                       getTopLeftCorner(false, true),
                       getTopLeftCorner(true, false),
                       getTopLeftCorner(false, false)};
    for (auto &p : points)
    {
        qDebug() << p;
    }
    for (int i = 0; i < 4; i++)
    {
        auto rectView = placeholders[i];
        QRectF rect(points[i].x(), points[i].y(), getScaledSize(), getScaledSize());
        qDebug() << rect;
        rectView->setRect(rect);
    }
    resetLogoPos();

}
