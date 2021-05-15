#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logooverlay.h"
#include "DragEventFilter.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

public slots:

    void logoChange();

    void videoChange();

    void setLogo(QString fileName);

    void setVideo(QString filename);

private slots:

    void watermarkVideo();

private:
    QString currentVideo;
    QString currentLogo;
    Ui::MainWindow *ui;
    LogoOverlay *overlay;
    QGraphicsPixmapItem *videoPrev;
    QGraphicsScene *graphicsScene;
    DragEventFilter *dragEventFilter;
};

#endif // MAINWINDOW_H
