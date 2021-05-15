//
// Created by Shein Htike on 5/14/21.
//

#ifndef IP2WATERMARKER_DRAGEVENTFILTER_H
#define IP2WATERMARKER_DRAGEVENTFILTER_H


#include <QDropEvent>
#include <QObject>

class DragEventFilter : public QObject
{
    Q_OBJECT

    bool eventFilter(QObject *watched, QEvent *event) override;

    static void checkEvent(QDropEvent *e);
public:
    DragEventFilter(QObject *parent);
signals:
    void videoDropped(QString);
};


#endif //IP2WATERMARKER_DRAGEVENTFILTER_H
