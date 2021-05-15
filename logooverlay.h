#ifndef LOGOOVERLAY_H
#define LOGOOVERLAY_H

#include "placeholdersquare.h"

#include <QGraphicsPixmapItem>
#include <array>


class LogoOverlay : public QObject
{
Q_OBJECT
public:
    LogoOverlay(QGraphicsPixmapItem *video, QPixmap logo, double scale, double padding, double opacity);

    int getLogoPos();

public slots:

    void paddingChanged(double newVal);

    void scaleChanged(double newVal);

    void logoChanged(QPixmap newVal);

    void opacityChanged(double newVal);

    void videoChanged();

private:
    QPoint getTopLeftCorner(bool top, bool left);

    QPixmap logo;
    QSize videoSize;
    int logoPos;
    QGraphicsPixmapItem *logoImage;
    std::array<PlaceholderSquare *, 4> placeholders;
    double padding, scale;

    int getScaledSize();

    int getPadding();

    QGraphicsPixmapItem *video;

    void moveLogo(int pos);

    QPoint getTopLeftCornerLogo(bool left, bool top);

    void resetLogoPos();

    int smallerDimension();

    int largerDimension();
};

#endif // LOGOOVERLAY_H
