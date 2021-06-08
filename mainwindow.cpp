#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHttpMultiPart>

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
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QClipboard>
#include <QSettings>

bool fileExists(QString path)
{
    QFileInfo check_file(path);
    // check if path exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), settings("IP2Watermarker")
{
    ui->setupUi(this);
    graphicsScene = new QGraphicsScene(ui->gv);
    ui->gv->setScene(graphicsScene);
    auto videoPrev = new QGraphicsPixmapItem;
    this->videoPrev = videoPrev;
    videoPrev->setPixmap(QPixmap(640, 360));
    graphicsScene->addItem(videoPrev);
    ui->gv->setVisible(false);


    extractFfmpeg();

    dragEventFilter = new DragEventFilter();


    overlay = new LogoOverlay(videoPrev, QPixmap(), 0.18, .0125, 75);

    // Connects sliders to spinboxes
    connect(
            ui->sizeSlider, &QSlider::valueChanged, [=](int val) {
                ui->sizeSpinBox->setValue(val / 100.0);
                ui->gv->repaint();
                ui->gv->setVisible(false);
                ui->gv->setVisible(true);
            }
    );
    connect(
            ui->sizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d) {
                ui->sizeSlider->setValue(qRound(d * 100));
            }
    );
    connect(
            ui->paddingSlider, &QSlider::valueChanged, [=](int val) {
                ui->paddingSpinBox->setValue(val / 100.0);
            }
    );
    connect(
            ui->paddingSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d) {
                ui->paddingSlider->setValue(qRound(100 * d));
            }
    );
    connect(
            ui->opacitySlider, &QSlider::valueChanged, [=](int val) {
                ui->opacitySpinBox->setValue(val);
            }
    );
    connect(
            ui->opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d) {
                ui->opacitySlider->setValue(qRound(d));
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


    connect(ui->selectVideoButton, &QPushButton::clicked, this, &MainWindow::videoChange);
    connect(ui->watermarkButton, &QPushButton::clicked, this, &MainWindow::watermarkVideo);

    ui->selectVideoButton->installEventFilter(dragEventFilter);
    ui->selectVideoButton->setAcceptDrops(true);

    connect(dragEventFilter, &DragEventFilter::videoDropped, this, &MainWindow::setVideo);
    setupLogos();
    connect(
            ui->logoNameComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() {
                setLogo(ui->logoNameComboBox->currentData().toString());
            }
    );
    ui->urlLineEdit->hide();
    ui->copyButton->hide();
    connect(
            ui->copyButton, &QPushButton::clicked, [this]() {
                QGuiApplication::clipboard()->setText(ui->urlLineEdit->text());
                statusBar()->showMessage("URL Copied!", 4000);
            }
    );

    ui->setDefaultsButton->setIcon(QIcon(":/icons/res/refresh.png"));
    ui->setDefaultsButton->setIconSize({16, 16});
    connect(ui->setDefaultsButton, &QPushButton::clicked, this, &MainWindow::setDefaultLogoParams);

    if (settings.contains("padding"))
    {
        ui->paddingSpinBox->setValue(settings.value("padding").toDouble());
    }
    if (settings.contains("size"))
    {
        ui->sizeSpinBox->setValue(settings.value("size").toDouble());
    }
    if (settings.contains("opacity"))
    {
        ui->opacitySpinBox->setValue(settings.value("opacity").toDouble());
    }
    if (settings.contains("logo"))
    {
        ui->logoNameComboBox->setCurrentIndex(settings.value("logo").toInt());
    }
    if (settings.contains("upload"))
    {
        ui->uploadCheck->setChecked(settings.value("upload").toBool());
    }
    if (settings.contains("logoPos"))
    {
        overlay->moveLogo(settings.value("logoPos").toUInt());
    }
    ui->gv->setVisible(false);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("opacity", ui->opacitySpinBox->value());
    settings.setValue("padding", ui->paddingSpinBox->value());
    settings.setValue("size", ui->sizeSpinBox->value());
    settings.setValue("logo", ui->logoNameComboBox->currentIndex());
    settings.setValue("upload", ui->uploadCheck->isChecked());
    settings.setValue("logoPos", overlay->getLogoPos());
    event->accept();
}

void MainWindow::setDefaultLogoParams()
{
    ui->sizeSpinBox->setValue(18.33);
    ui->paddingSpinBox->setValue(1.39);
    ui->opacitySpinBox->setValue(60);
}

void MainWindow::detectLoudNorm()
{
}

void MainWindow::setupLogos()
{
    for (const auto &n : logoNames)
    {
        ui->logoNameComboBox->addItem(n[0], QString(":/logos/res/logos/%1.png").arg(n[1]));
    }
    qDebug() << "CURRENT LOGO: " << ui->logoNameComboBox->currentData().toString();
    setLogo(ui->logoNameComboBox->currentData().toString());
}

QString MainWindow::extractLogo()
{
    auto filename = QFileInfo(ui->logoNameComboBox->currentData().toString()).fileName();
    auto path = QStandardPaths::locate(QStandardPaths::AppDataLocation, filename);
    qDebug() << path;
    if (path.length() != 0)
        return path;
    auto location = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0];
    if (!QDir(location).exists())
        QDir().mkdir(location);
    location += "/" + filename;
    QFile::copy(ui->logoNameComboBox->currentData().toString(), location);
    return location;
}

QString MainWindow::uploadToVidey(QString filename)
{
    QHttpPart imagePart;
    //imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));


    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("video/mp4"));
    imagePart.setHeader(
            QNetworkRequest::ContentDispositionHeader,
            QVariant("form-data; name=\"file\"; filename=\"clip.mp4\""));
    QFile *file = new QFile(filename);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
    multiPart->append(imagePart);

    QUrl url("https://videy.co/api/upload");
    QNetworkRequest request(url);

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
// here connect signals etc.
    connect(
            reply, &QNetworkReply::finished, [reply, this]() {
                if(reply->error() != QNetworkReply::NoError){
                    statusBar()->showMessage("Upload failed " + reply->errorString(), 4000);
                }
                qDebug() << reply;
                statusBar()->showMessage("Upload complete!", 4000);
                setEnableUi(true);
                QString outp(reply->readAll());
                qDebug() << "OUTPUT: " << outp;
                QString url = "https://videy.co/v?id=";
                int index = outp.indexOf('"');
                index = outp.indexOf('"', index + 1);
                index = outp.indexOf('"', index + 1) + 1;
                while (index <= outp.size() && outp[index] != '"')
                    url += outp[index++];
                auto lineEdit = ui->urlLineEdit;
                ui->urlLineEdit->setText(url);
                auto font = lineEdit->fontMetrics();
                auto size = font.size(0,url).width();
                size += lineEdit->textMargins().left() + lineEdit->textMargins().right() + 10;
                lineEdit->setMinimumWidth(size);

                ui->urlLineEdit->show();
                ui->copyButton->show();
            }
    );
    connect(
            reply, &QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal) {
                if (bytesSent == bytesTotal)
                    statusBar()->showMessage("Waiting for videy.co...");
                else
                {
                    statusBar()->showMessage(
                            "Uploading to videy.co (" + QString::number(double(bytesSent) / bytesTotal * 100, 'f', 0) +
                            "%)"
                    );
                    ui->progressBar->setValue(ui->progressBar->maximum() * ((float) bytesSent) / bytesTotal);
                }
            }
    );
    return "";
}

void MainWindow::extractFfmpeg()
{
#ifdef Q_OS_WIN
    auto path = QStandardPaths::locate(QStandardPaths::AppDataLocation, "ffmpeg.exe");
    qDebug() << path;
    if (path.length() != 0)
        return;

    auto location = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0];
    if (!QDir(location).exists())
        QDir().mkdir(location);
    location += "/ffmpeg.exe";
    QFile::copy(":/bin/res/ffmpeg.exe", location);
    qDebug() << location;

    auto path = QStandardPaths::locate(QStandardPaths::AppDataLocation, "ffprobe.exe");
    qDebug() << path;
    if (path.length() != 0)
        return;

    auto location = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0];
    if (!QDir(location).exists())
        QDir().mkdir(location);
    location += "/ffprobe.exe";
    QFile::copy(":/bin/res/ffprobe.exe", location);
    qDebug() << location;
#endif
#ifdef Q_OS_MACOS
    if (!fileExists("/usr/local/bin/ffmpeg") | true)
    {
        auto path = QStandardPaths::locate(QStandardPaths::AppDataLocation, "ffmpeg");
        qDebug() << path;
        if (path.length() == 0)
        {
            auto location = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0];
            if (!QDir(location).exists())
                QDir().mkdir(location);
            location += "/ffmpeg";
            QFile::copy(":/bin/res/ffmpeg_mac", location);
            qDebug() << location;
            QFile::setPermissions(location,
                                  QFile(location).permissions() | QFileDevice::ExeOther | QFileDevice::ExeGroup |
                                  QFileDevice::ExeUser | QFileDevice::WriteOwner | QFileDevice::WriteOwner
            );
        }
    }


    if (!fileExists("/usr/local/bin/ffprobe") | true)
    {
        auto path = QStandardPaths::locate(QStandardPaths::AppDataLocation, "ffprobe");
        qDebug() << path;
        if (path.length() == 0)
        {
            auto location = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0];
            if (!QDir(location).exists())
                QDir().mkdir(location);
            location += "/ffprobe";
            QFile::copy(":/bin/res/ffprobe_mac", location);
            qDebug() << location;
            QFile::setPermissions(location,
                                  QFile(location).permissions() | QFileDevice::ExeOther | QFileDevice::ExeGroup |
                                  QFileDevice::ExeUser | QFileDevice::WriteOwner | QFileDevice::WriteOwner
            );
        }
    }
#endif
}


QString MainWindow::getExecutable(QString name)
{
    QString path;
    if (path.length() != 0)
        return path;
#ifdef Q_OS_WIN
    path = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + QString("/%1.exe").arg(name);
    return path;
#endif
#ifdef Q_OS_MACOS
    path = "/usr/local/bin/" + name;
    if (fileExists(path))
        return path;
    else
    path = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)[0] + "/" + name;
    return path;
#endif
#ifdef Q_OS_LINUX
    path = "/usr/bin/" + name;
    if(fileExists(path))
        return path;
    path = "/usr/local/bin/" + name;
    if(fileExists(path))
        return path;
    path = "/bin/" + name;
    if(fileExists(path))
        return path;
#endif
}

double MainWindow::timeToSeconds(QString duration)
{
    auto split = duration.split(':');
    return (split[0].toDouble() * 3600 + split[1].toDouble() * 60 + split[2].toDouble());
}

void MainWindow::watermarkVideo()
{
    static double currentVideoDur;

    auto probe = new QProcess;
    probe->start(
            getExecutable("ffprobe"), {
                    "-v",
                    "error",
                    "-skip_frame",
                    "nokey",
                    "-show_entries",
                    "frame=pkt_pts_time",
                    "-select_streams",
                    "v",
                    "-of",
                    "csv=p=0",
                    "-read_intervals",
                    "%10",
                    currentVideo
            }
    );
    auto loudnorm = new QProcess;
    setEnableUi(false);
    statusBar()->showMessage("Processing audio");
    connect(
            loudnorm, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
                QString str;
                QJsonDocument j;
                {
                    auto err = QString(loudnorm->readAllStandardError());
                    qDebug() << "LOUDNORM:" << err;
                    auto out = err.indexOf("[Parsed_loudnorm");
                    auto start = err.indexOf('{', out);
                    j = QJsonDocument::fromJson(err.mid(start).toUtf8());
                    delete loudnorm;
                }
                str += ":measured_I=";
                str += j["input_i"].toString();
                str += ":measured_LRA=";
                str += j["input_lra"].toString();
                str += ":measured_TP=";
                str += j["input_tp"].toString();
                str += ":measured_thresh=";
                str += j["input_thresh"].toString();
                str += ":offset=";
                str += j["target_offset"].toString();

                auto logoLoc = extractLogo();

                currentVideoDur = -1;
                auto ffmpeg = new QProcess;
                auto output = getOutputFilename(currentVideo);
                const auto logoPos = overlay->getLogoPos();
                const QString x1 = "%1*min(main_h\\,main_w)";
                const QString y1 = "%1*min(main_h\\,main_w)";
                const QString x2 = "main_w-overlay_w-%1*min(main_h\\,main_w)";
                const QString y2 = "main_h-overlay_h-%1*min(main_h\\,main_w)";

                auto xPos = logoPos % 2 ? x2 : x1;
                auto yPos = logoPos / 2 ? y2 : y1;
                xPos = xPos.arg(ui->paddingSpinBox->value() / 100);
                yPos = yPos.arg(ui->paddingSpinBox->value() / 100);
                QString filter = "\"[1][0]scale2ref=h=min(ih\\,iw)*%1:w=oh*mdar:sws_flags=lanczos[wmark_scaled][base_video];[wmark_scaled]format=argb,colorchannelmixer=aa=%2[wmark_transparent];[base_video][wmark_transparent]overlay=%3:%4\"";

                connect(
                        ffmpeg, &QProcess::readyReadStandardError, [=]() {
                            const static QRegExp duration("Duration: ([\\d:\\.]+),");
                            const static QRegExp time("time=([\\d:\\.]+)\\s");
                            auto str = QString(ffmpeg->readAllStandardError());
                            if (currentVideoDur == -1 && duration.indexIn(str) != -1)
                            {
                                auto time = duration.cap(1);
//                                qDebug() << "Dur: " << time;
                                currentVideoDur = timeToSeconds(time);
                            } else if (time.indexIn(str) != -1)
                            {
                                auto seconds = timeToSeconds(time.cap(1));
//                                qDebug() << "Time: " << seconds;
                                auto percentage = seconds / currentVideoDur;
                                statusBar()->showMessage(
                                        "Watermarking (" + QString::number(percentage * 100, 'f', 0) + "%)"
                                );
                                if (percentage < 1)
                                    ui->progressBar->setValue(qRound(percentage * ui->progressBar->maximum()));
                            }
                        }
                );
                connect(
                        ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
                            delete ffmpeg;
                            if (ui->uploadCheck->checkState() == Qt::Checked)
                            {
                                uploadToVidey(output);
                            } else
                            {
                                setEnableUi(true);
                                ui->progressBar->setValue(ui->progressBar->maximum());
                                statusBar()->showMessage("Video saved!", 5000);
                                ui->urlLineEdit->hide();
                                ui->copyButton->hide();
                            }
                        }
                );
                probe->waitForFinished();
                probe->setParent(ffmpeg);
                auto time = QString(probe->readLine());
                time.chop(1);
                qDebug() << "FFPROBE TIME: " << time.toDouble();
                if (time.toDouble() == 0 || time.toDouble() >= 2)
                {
                    ffmpeg->start(
                            getExecutable("ffmpeg"), {
                                    "-i",
                                    currentVideo,
                                    "-i",
                                    logoLoc,
                                    "-preset",
                                    "faster",
                                    "-crf",
                                    "21",
                                    "-maxrate",
                                    "4.2M",
                                    "-bufsize",
                                    "4.2M",
                                    "-movflags",
                                    "+faststart",
                                    "-max_muxing_queue_size",
                                    "9999",
                                    "-filter_complex",
                                    filter.arg(ui->sizeSpinBox->value() / 100).arg(
                                            ui->opacitySpinBox->value() / 100
                                    ).arg(
                                            xPos,
                                            yPos
                                    ),
                                    "-af",
                                    "\"loudnorm=I=-16:TP=-1.5:LRA=11" + str + '"',
                                    "-vbr",
                                    "5",
                                    "-ar",
                                    "44100",
                                    "-y",
                                    output
                            }
                    );
                } else
                {
                    ffmpeg->start(
                            getExecutable("ffmpeg"), {
                                    "-ss",
                                    time,
                                    "-i",
                                    currentVideo,
                                    "-i",
                                    logoLoc,
                                    "-preset",
                                    "faster",
                                    "-crf",
                                    "21",
                                    "-maxrate",
                                    "4.2M",
                                    "-bufsize",
                                    "4.2M",
                                    "-movflags",
                                    "+faststart",
                                    "-max_muxing_queue_size",
                                    "9999",
                                    "-filter_complex",
                                    filter.arg(ui->sizeSpinBox->value() / 100).arg(
                                            ui->opacitySpinBox->value() / 100
                                    ).arg(
                                            xPos,
                                            yPos
                                    ),
                                    "-af",
                                    "\"loudnorm=I=-16:TP=-1.5:LRA=11" + str + '"',
                                    "-vbr",
                                    "5",
                                    "-ar",
                                    "44100",
                                    "-y",
                                    output
                            }
                    );
                }
                qDebug() << "Command: " << "ffmpeg " + ffmpeg->arguments().join(" ");
            }
    );
    loudnorm->start(
            getExecutable("ffmpeg"), {
                    "-i",
                    currentVideo,
                    "-vn",
                    "-af",
                    "loudnorm=I=-16:TP=-1.5:LRA=11:print_format=json",
                    "-hide_banner",
                    "-f",
                    "null",
                    "-",
            }
    );
}

void MainWindow::setEnableUi(bool enabled)
{
    QWidget *list[] = {
            ui->paddingSpinBox,
            ui->opacitySpinBox,
            ui->sizeSpinBox,
            ui->selectVideoButton,
            ui->watermarkButton,
            ui->sizeSlider,
            ui->paddingSlider,
            ui->opacitySlider,
            ui->uploadCheck,
            ui->setDefaultsButton,
            ui->logoNameComboBox
    };
    for (QWidget *i : list)
    {
        i->setEnabled(enabled);
    }

    overlay->setShowPlaceholders(enabled);
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

QString MainWindow::getOutputFilename(QString filename)
{
    auto pos = filename.lastIndexOf('.');
    auto outputFilename = filename.left(pos) + "_wmark.mp4";
    return outputFilename;
}

void MainWindow::setVideo(QString filename)
{
    currentVideo = filename;
    auto ffmpeg = new QProcess();
    ffmpeg->start(
            getExecutable("ffmpeg"),
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

                auto output = getOutputFilename(currentVideo);
                QFontMetrics metrics(ui->outputLabel->font());
                ui->outputLabel->setText(metrics.elidedText(output, Qt::ElideLeft, ui->outputLabel->width()));
                statusBar()->clearMessage();
                ui->urlLineEdit->hide();
                ui->copyButton->hide();

                QWidget *list[] = {
                        ui->paddingSpinBox,
                        ui->opacitySpinBox,
                        ui->sizeSpinBox,
                        ui->watermarkButton,
                        ui->sizeSlider,
                        ui->paddingSlider,
                        ui->opacitySlider,
                        ui->uploadCheck,
                        ui->setDefaultsButton,
                };
                for (QWidget *i : list)
                {
                    i->setEnabled(true);
                }
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
    overlay->logoChanged(pixmap);
    currentLogo = fileName;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete overlay;
}

