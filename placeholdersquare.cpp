#include "placeholdersquare.h"

#include <QCursor>
#include <QPen>
#include <QObject>
#include <QDebug>

PlaceholderSquare::PlaceholderSquare(int x, int y,int w,int h, QGraphicsItem *parent) : QGraphicsRectItem(x,y,w,h,parent){
    QPen pen;
    pen.setColor(Qt::red);
    pen.setStyle(Qt::DashDotLine);
    setPen(pen);
    QCursor cursor;
    cursor.setShape(Qt::PointingHandCursor);
    setCursor(cursor);
}





void PlaceholderSquare::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit clicked();
    qDebug() << "clicc";
}
