#include "mainwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QImageReader>
#include <QPixmap>
#include <QGraphicsScene>
#include <QBrush>
#include <QFileDialog>
#include <QStandardPaths>
#include <QProcess>
#include <QImage>
#include <QDragEnterEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    graphicsScene = new QGraphicsScene(ui->gv);
    ui->gv->setScene(graphicsScene);
    auto videoPrev = new QGraphicsPixmapItem;
    this->videoPrev = videoPrev;
    videoPrev->setPixmap(QPixmap(640, 360));
    graphicsScene->addItem(videoPrev);
    ui->gv->setVisible(false);

    dragEventFilter = new DragEventFilter(this);


    overlay = new LogoOverlay(videoPrev, QPixmap(), 0.18, .0125, 75);
    QString picture = "/Users/shein/Downloads/ip2_logo2.png";
    setLogo(picture);

    // Connects sliders to spinboxes
    connect(
            ui->sizeSlider, &QSlider::valueChanged, [=](int val) {
                ui->sizeSpinBox->setValue(val);
                ui->gv->repaint();
                ui->gv->setVisible(false);
                ui->gv->setVisible(true);
            }
    );
    connect(
            ui->sizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d) {
                ui->sizeSlider->setValue(round(d));
            }
    );
    connect(
            ui->paddingSlider, &QSlider::valueChanged, [=](int val) {
                ui->paddingSpinBox->setValue(val / 8.0);
            }
    );
    connect(
            ui->paddingSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d) {
                ui->paddingSlider->setValue(round(8 * d));
            }
    );
    connect(
            ui->opacitySlider, &QSlider::valueChanged, [=](int val) {
                ui->opacitySpinBox->setValue(val);
            }
    );
    connect(
            ui->opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d) {
                ui->opacitySlider->setValue(round(d));
            }
    );

    //Update scale of logo
    connect(ui->sizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), overlay, &LogoOverlay::scaleChanged);
    //Update scale of logo
    connect(
            ui->paddingSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), overlay,
            &LogoOverlay::paddingChanged
    );
    //Update opacity
    connect(
            ui->opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), overlay,
            &LogoOverlay::opacityChanged
    );


    connect(ui->selectLogoButton, &QPushButton::clicked, this, &MainWindow::logoChange);
    connect(ui->selectVideoButton, &QPushButton::clicked, this, &MainWindow::videoChange);
    connect(ui->logoImage, &DragLabel::imageDropped, this, &MainWindow::setLogo);
    connect(ui->watermarkButton, &QPushButton::clicked, this, &MainWindow::watermarkVideo);

    ui->gv->installEventFilter(dragEventFilter);
    ui->frame->installEventFilter(dragEventFilter);
    ui->selectVideoButton->installEventFilter(dragEventFilter);
    ui->gv->setAcceptDrops(true);
    ui->frame->setAcceptDrops(true);
    ui->selectVideoButton->setAcceptDrops(true);
    videoPrev->setAcceptDrops(true);

    connect(dragEventFilter,&DragEventFilter::videoDropped,this,&MainWindow::setVideo);
}


void MainWindow::watermarkVideo()
{
    auto ffmpeg = new QProcess;
    auto output = "/Users/shein/Downloads/clip.mp4";
    const auto logoPos = overlay->getLogoPos();
    const QString x1 = "%1*min(main_h\\,main_w)";
    const QString y1 = "%1*min(main_h\\,main_w)";
    const QString x2 = "main_w-overlay_w-%1*min(main_h\\,main_w)";
    const QString y2 = "main_h-overlay_h-%1*min(main_h\\,main_w)";

    auto xPos = logoPos / 2 ? x2 : x1;
    auto yPos = logoPos % 2 ? y2 : y1;
    xPos = xPos.arg(ui->paddingSpinBox->value() / 100);
    yPos = yPos.arg(ui->paddingSpinBox->value() / 100);
    QString filter = "[1][0]scale2ref=h=min(ih\\,iw)*%1:w=oh*mdar:sws_flags=lanczos[wmark_scaled][base_video];[wmark_scaled]format=argb,colorchannelmixer=aa=%2[wmark_transparent];[base_video][wmark_transparent]overlay=%3:%4";

    ffmpeg->start(
            "/usr/local/bin/ffmpeg", {
                    "-i",
                    currentVideo,
                    "-i",
                    currentLogo,
                    "-preset",
                    "veryfast",
                    "-crf",
                    "21",
                    "-max_muxing_queue_size",
                    "9999",
                    "-filter_complex",
                    filter.arg(ui->sizeSpinBox->value() / 100).arg(ui->opacitySpinBox->value() / 100).arg(xPos, yPos),
                    "-c:a",
                    "libfdk_aac",
                    "-vbr",
                    "5",
                    "-y",
                    output
            }
    );
    connect(
            ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
                qDebug() << "DONE";
                qDebug() << ffmpeg->readAllStandardError();
                delete ffmpeg;
            }
    );
}

void MainWindow::logoChange()
{
    auto locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QString start_dir;
    if (!locations.isEmpty())
    {
        start_dir = locations[0];
    }
    auto picture = QFileDialog::getOpenFileName(
            this,
            tr("Open Image"), start_dir,
            "Image Files (*.png *.jpg *tiff *.bmp *gif)"
    );
    if (picture == nullptr)
    {
        return;
    }
    setLogo(picture);
}

void MainWindow::videoChange()
{
    auto locations = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    QString start_dir;
    if (!locations.isEmpty())
    {
        start_dir = locations[0];
    }
    auto picture = QFileDialog::getOpenFileName(
            this,
            tr("Open Video"),
            start_dir,
            "Video Files (*.mp4 *.mov *.webm *.mkv *.avi *.wmv *.m4a *.ts *.mpeg *.flv)"
    );
    if (picture == nullptr)
    {
        return;
    }
    setVideo(picture);
}

void MainWindow::setVideo(QString filename)
{
    currentVideo = filename;
    auto ffmpeg = new QProcess();
    ffmpeg->start(
            "/usr/local/bin/ffmpeg",
            {"-i", filename, "-vf", "thumbnail,scale=w=640:h=360:force_original_aspect_ratio=decrease",
             "-sws_flags", "lanczos+accurate_rnd+full_chroma_int+full_chroma_inp", "-frames:v", "1", "-y", "-f",
             "image2pipe", "-c:v", "ppm", "-an", "-"}
    );
    connect(
            ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
                QImage image;
                bool res = image.load(ffmpeg, "PPM");
                this->ui->gv->setFixedSize(image.size());
                this->videoPrev->setPixmap(QPixmap::fromImage(image));
                overlay->videoChanged();
                ui->gv->setVisible(true);
                currentVideo = filename;
                delete ffmpeg;
            }
    );
}

void MainWindow::setLogo(QString fileName)
{

    QImageReader reader(fileName);
    QImage img = reader.read();
    QPixmap pixmap = QPixmap::fromImage(img);
    ui->logoImage->setPixmap(pixmap.scaled(130, 130, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QUrl url = QUrl::fromLocalFile(fileName);
    ui->logoFileNameLabel->setText(url.fileName());
    overlay->logoChanged(pixmap);
    currentLogo = fileName;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete overlay;
}

