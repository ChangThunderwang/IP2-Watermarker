#include "draglabel.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
DragLabel::DragLabel(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true);
}


void DragLabel::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls() && e->mimeData()->urls().size() == 1) {
        auto url = e->mimeData()->urls()[0];
        if(url.isLocalFile()){
            auto str = url.toLocalFile().toLower();
            qDebug() << str;
            const QString exts[] = {".png",".jpg",".tiff",".bmp",".gif"};
            for(const auto &c : exts){
                if(str.endsWith(c)){
                    e->acceptProposedAction();
                }
            }
        }
    }
}

void DragLabel::dropEvent(QDropEvent *e)
{
    auto path = e->mimeData()->urls()[0].toLocalFile();
    emit imageDropped(path);
}
