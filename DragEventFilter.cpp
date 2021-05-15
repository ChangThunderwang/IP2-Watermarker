#include "DragEventFilter.h"
#include <QEvent>
#include <QMimeData>


void DragEventFilter::checkEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls() && e->mimeData()->urls().size() == 1)
    {
        auto url = e->mimeData()->urls()[0];
        if (url.isLocalFile())
        {
            auto str = url.toLocalFile().toLower();
            const QString exts[] = {".mov", ".mp4", ".avi", ".m4v", ".flv", ".webm", ".mkv"};
            for (const auto &c : exts)
            {
                if (str.endsWith(c))
                {
                    e->acceptProposedAction();
                }
            }
        }
    }
}

bool DragEventFilter::eventFilter(QObject *object, QEvent *event)
{
    auto eventType = event->type();
    switch (eventType)
    {
        case QEvent::DragEnter:
        case QEvent::DragMove:
        {
            checkEvent((QDropEvent *) event);
            break;
        }
        case QEvent::Drop:
        {
            auto path = ((QDropEvent *) event)->mimeData()->urls()[0].toLocalFile();
            emit videoDropped(path);
            break;
        }
        default:
        {
            return false;
        }
    }
    return true;
}

DragEventFilter::DragEventFilter(QObject *parent) : QObject(parent)
{}
