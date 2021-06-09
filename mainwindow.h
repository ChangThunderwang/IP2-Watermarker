#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logooverlay.h"
#include "DragEventFilter.h"
#include <QVector>
#include <QStringList>
#include <QSettings>

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

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:

    void watermarkVideo();

    void setDefaultLogoParams();
private:
    QString extractLogo();
    void extractFfmpeg();
    QString currentVideo;
    QString currentLogo;
    Ui::MainWindow *ui;
    LogoOverlay *overlay;
    QGraphicsPixmapItem *videoPrev;
    QGraphicsScene *graphicsScene;
    DragEventFilter *dragEventFilter;
    QString getExecutable(QString name);

    double timeToSeconds(QStringView duration);
    void setEnableUi(bool enabled);
    QString getOutputFilename(QString filename);
    void setupLogos();
    const QVector<QStringList> logoNames = {
        {"Standard logo","standard"},
        {"Standard Logo With Shadow","shadow"},
        {"Classic Logo","undefeated"},
        {"nTrepid Logo","ntrepid"},
        {"Circular Logo","circular"},
    };
    void detectLoudNorm();

    QString uploadToVidey(QString filename);
    QSettings settings;
};

#endif // MAINWINDOW_H
