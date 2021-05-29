//
// Created by Shein Htike on 5/14/21.
//

#ifndef IP2WATERMARKER_DRAGEVENTFILTER_H
#define IP2WATERMARKER_DRAGEVENTFILTER_H


#include <QDropEvent>
#include <QGraphicsObject>

class DragEventFilter : public QGraphicsObject
{
    Q_OBJECT

    bool eventFilter(QObject *watched, QEvent *event) override;

    static void checkEvent(QDropEvent *e);
public:
protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

public:
    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    DragEventFilter(QGraphicsItem *parent = nullptr);
    bool handleDrag(QEvent *event);

signals:
    void videoDropped(QString);

};


#endif //IP2WATERMARKER_DRAGEVENTFILTER_H
