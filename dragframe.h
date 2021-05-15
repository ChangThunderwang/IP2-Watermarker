#ifndef DRAGFRAME_H
#define DRAGFRAME_H

#include <QFrame>
#include <QObject>

class DragFrame : public QFrame
{
Q_OBJECT
public:
    DragFrame(QWidget *parent);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;

    void dropEvent(QDropEvent *e) override;

signals:
    void videoDropped(QString);

protected slots:
    void dragMoveEvent(QDragMoveEvent *event);
};

#endif // DRAGFRAME_H
