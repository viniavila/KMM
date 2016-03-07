#include "imageviewer.h"
#include "ui_imageviewer.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QScrollArea>
#include <QImageReader>
#include <QScrollBar>

#define DS QDir::separator()

class ImageViewerPrivate {
    Q_DECLARE_PUBLIC(ImageViewer)
public:
    ImageViewerPrivate(ImageViewer * parent) :
        q_ptr(parent),
        labelImage(new QLabel)
    { }
    ImageViewer * const q_ptr;

    void initialize(Ui::ImageViewer *ui) {
        ui->setupUi(q_ptr);
        labelImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        labelImage->setScaledContents(true);

        ui->image_viewer_scroll_area->setWidget(labelImage);
        ui->image_viewer_scroll_area->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);

        QRect screen = QApplication::desktop()->screenGeometry(q_ptr->parentWidget());
        q_ptr->resize(0.7*screen.width(), 0.9*screen.height());
        q_ptr->move(screen.x()+0.15*screen.width(), screen.y()+0.05*screen.height());
        q_ptr->setFixedSize(q_ptr->size());
        q_ptr->adjustSize();

        q_ptr->connect(ui->btnZoomIn, &QPushButton::clicked, [=](){ scaleImage(ui, 1.25); });
        q_ptr->connect(ui->btnZoomOut, &QPushButton::clicked, [=](){ scaleImage(ui, 0.8); });
        q_ptr->connect(ui->btnFit, &QPushButton::clicked, [=](){ fitToWindow(ui); });
        q_ptr->connect(ui->btnOriginal, &QPushButton::clicked, [=](){ labelImage->adjustSize(); scaleFactor = 1.0; });
        q_ptr->connect(ui->btnRotateRight, &QPushButton::clicked, [=](){ rotateRight(ui); });
        q_ptr->connect(ui->btnRotateLeft, &QPushButton::clicked, [=](){ rotateLeft(ui); });
    }

    void enableControls(Ui::ImageViewer *ui, bool b) {
        ui->btnFit->setEnabled(b);
        ui->btnOriginal->setEnabled(b);
        ui->btnRotateLeft->setEnabled(b);
        ui->btnRotateRight->setEnabled(b);
        ui->btnZoomIn->setEnabled(b);
        ui->btnZoomOut->setEnabled(b);
    }

    void scaleImage(Ui::ImageViewer *ui, float factor) {
        scaleFactor *= factor;
        labelImage->resize(scaleFactor * labelImage->pixmap()->size());

        adjustScrollBar(ui->image_viewer_scroll_area->horizontalScrollBar(), factor);
        adjustScrollBar(ui->image_viewer_scroll_area->verticalScrollBar(), factor);

        ui->btnZoomIn->setEnabled(scaleFactor < 3.0);
        ui->btnZoomOut->setEnabled(scaleFactor > 0.333);
    }

    void adjustScrollBar(QScrollBar* bar, float factor) {
        bar->setValue(int(factor * bar->value() + ((factor - 1) * bar->pageStep()/2)));
    }

    void fitToWindow(Ui::ImageViewer *ui) {
        labelImage->adjustSize();
        scaleFactor = 1.0;
        float nScaleFactor;
        if ((float)ui->image_viewer_scroll_area->width()/ui->image_viewer_scroll_area->height() < (float)labelImage->width()/labelImage->height())
            nScaleFactor = (ui->image_viewer_scroll_area->width()) / (1.01*labelImage->width());
        else
            nScaleFactor = (ui->image_viewer_scroll_area->height()) / (1.01*labelImage->height());
        scaleImage(ui, nScaleFactor);
    }

    void loadFile(Ui::ImageViewer *ui, const QString& path) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        const QImage img = reader.read();
        if (img.isNull()) {
            labelImage->setPixmap(QPixmap());
            labelImage->adjustSize();
            currentFile.clear();
        }
        else {
            labelImage->setPixmap(QPixmap::fromImage(img));
            enableControls(ui, true);
            fitToWindow(ui);
            currentFile = path;
        }
    }

    void loadList(Ui::ImageViewer *ui, const QString& path) {
        QDir d(path);
        QStringList files = d.entryList(QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name|QDir::LocaleAware);
        if (files.isEmpty()) return;

        ui->picList->addItems(files);
        ui->picList->setCurrentRow(0);
        loadFile(ui, path + DS + files.first());
    }

    void rotateRight(Ui::ImageViewer *ui) {
        QPixmap px = QPixmap(*labelImage->pixmap());
        QTransform mtx;
        mtx = mtx.rotate(90);
        px = px.transformed(mtx, Qt::SmoothTransformation);
        px.save(currentFile);
        labelImage->setPixmap(px);
        fitToWindow(ui);
        emit q_ptr->imageRotated();
    }

    void rotateLeft(Ui::ImageViewer *ui) {
        QTransform mtx;
        mtx = mtx.rotate(-90);
        QPixmap px = labelImage->pixmap()->transformed(mtx, Qt::SmoothTransformation);
        px.save(currentFile);
        labelImage->setPixmap(px);
        fitToWindow(ui);
        emit q_ptr->imageRotated();
    }

    QLabel * labelImage;
    QString currentFile;
    float scaleFactor;
};

ImageViewer::ImageViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageViewer),
    d_ptr(new ImageViewerPrivate(this))
{
    d_ptr->initialize(ui);
}

ImageViewer::ImageViewer(const QString& path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageViewer),
    d_ptr(new ImageViewerPrivate(this))
{
    d_ptr->initialize(ui);

    QFileInfo fi(path);
    if (fi.isDir()) {
        // path is a dir
        d_ptr->loadList(ui, path);
        connect(ui->picList, &QListWidget::currentRowChanged, [=](int row){ d_ptr->loadFile(ui, path + DS + ui->picList->item(row)->text()); });
    }
    else {
        // path is a file
        ui->picList->setVisible(false);
        adjustSize();
        d_ptr->loadFile(ui, path);
    }
}


ImageViewer::~ImageViewer() {
    delete d_ptr;
    delete ui;
}
