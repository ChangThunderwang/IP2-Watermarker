#ifndef PLACEHOLDERSQUARE_H
#define PLACEHOLDERSQUARE_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QObject>

class PlaceholderSquare : public QObject, public QGraphicsRectItem
{

Q_OBJECT

public:
    PlaceholderSquare(int x, int y, int w, int h, QGraphicsItem *parent);

signals:

    void clicked();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // PLACEHOLDERSQUARE_H
