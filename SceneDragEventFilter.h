//
// Created by Shein Htike on 5/14/21.
//

#ifndef IP2WATERMARKER_SCENEDRAGEVENTFILTER_H
#define IP2WATERMARKER_SCENEDRAGEVENTFILTER_H


#include <QGraphicsItem>

class SceneDragEventFilter : public QGraphicsItem
{
protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

public:
    SceneDragEventFilter(QGraphicsItem *parent);
};


#endif //IP2WATERMARKER_SCENEDRAGEVENTFILTER_H
