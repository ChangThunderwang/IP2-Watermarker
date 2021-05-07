#include "dragframe.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

void checkEvent(QDropEvent *e){
    if (e->mimeData()->hasUrls() && e->mimeData()->urls().size() == 1) {
        auto url = e->mimeData()->urls()[0];
        if(url.isLocalFile()){
            auto str = url.toLocalFile().toLower();
            qDebug() << str;
            const QString exts[] = {".mov",".mp4",".avi",".m4v",".flv",".webm",".mkv"};
            for(const auto &c : exts){
                if(str.endsWith(c)){
                    e->acceptProposedAction();
                }
            }
        }
    }
}
DragFrame::DragFrame(QWidget *parent) : QFrame(parent)
{
    setAcceptDrops(true);
}

void DragFrame::dragEnterEvent(QDragEnterEvent *e)
{
    checkEvent(e);
}

void DragFrame::dropEvent(QDropEvent *e)
{
    auto path = e->mimeData()->urls()[0].toLocalFile();
    emit videoDropped(path);
}

void DragFrame::dragMoveEvent(QDragMoveEvent *e){
    checkEvent(e);
}
