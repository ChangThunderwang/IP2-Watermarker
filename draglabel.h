#ifndef DRAGLABEL_H
#define DRAGLABEL_H

#include <QLabel>


class DragLabel : public QLabel
{
Q_OBJECT
public:
    DragLabel(QWidget *parent);

protected:
    void dragEnterEvent(QDragEnterEvent *e);

    void dropEvent(QDropEvent *e);

signals:

    void imageDropped(QString);
};

#endif // DRAGLABEL_H
