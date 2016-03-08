#include "picanalyzeresult.h"
#include "ui_picanalyzeresult.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QImageReader>

class PicAnalyzeResultPrivate {
    Q_DECLARE_PUBLIC(PicAnalyzeResult)
public:
    PicAnalyzeResultPrivate(PicAnalyzeResult * parent) :
        q_ptr(parent) ,
        labelImage(new QLabel),
        modified(false)
    { }

    void loadFile(int r) {
        QImageReader reader(pictures.at(r));
        reader.setAutoTransform(true);
        const QImage img = reader.read();
        if (img.isNull()) {
            labelImage->setPixmap(QPixmap());
            labelImage->adjustSize();
            ui->comboBox->setCurrentIndex(-1);
        }
        else {
            labelImage->setPixmap(QPixmap::fromImage(img));
            labelImage->adjustSize();
            float scaleFactor = float(ui->widget->width())/(1.01*labelImage->width());
            labelImage->resize(scaleFactor * labelImage->pixmap()->size());
            ui->comboBox->setCurrentIndex(customActions.value(pictures.at(r), defaultAction));
        }
    }

    void setPictureAction(int newAction) {
        QString picture = pictures.at(ui->listWidget->currentRow());
        customActions[picture] = newAction;
    }

    void rotatePicture(const QString& fpath) {
        QTransform mtx;
        mtx = mtx.rotate(-90);
        QPixmap px = QPixmap(fpath).transformed(mtx, Qt::SmoothTransformation);
        px.save(fpath);
        modified = true;
    }

    void splitPicture(const QString& fpath) {
        QImageReader imgL(fpath);
        QImageReader imgR(fpath);
        QSize sz = imgL.size();
        QRect cLeft = readingLR ? QRect(0, 0, sz.width()/2, sz.height()) : QRect(sz.width()/2, 0, sz.width(), sz.height());
        QRect cRight = readingLR ? QRect(sz.width()/2, 0, sz.width(), sz.height()) : QRect(0, 0, sz.width()/2, sz.height());
        imgL.setClipRect(cLeft);
        imgR.setClipRect(cRight);
        imgL.read().save("/tmp/testeL.jpg");
        imgR.read().save("/tmp/testeR.jpg");
        modified = true;
    }

    Ui::PicAnalyzeResult *ui;
    PicAnalyzeResult * const q_ptr;
    QLabel *labelImage;
    QStringList pictures;
    QHash<QString, int> customActions;
    int defaultAction;
    bool readingLR;
    bool modified;
};

PicAnalyzeResult::PicAnalyzeResult(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PicAnalyzeResult),
    d_ptr(new PicAnalyzeResultPrivate(this))
{
    ui->setupUi(this);
}

PicAnalyzeResult::PicAnalyzeResult(const QStringList& pictures, int defAction, bool readingLR, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PicAnalyzeResult),
    d_ptr(new PicAnalyzeResultPrivate(this))
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->comboBox->setCurrentIndex(defAction);
    d_ptr->ui = ui;
    d_ptr->pictures = pictures;
    d_ptr->defaultAction = defAction;
    d_ptr->readingLR = readingLR;

    d_ptr->labelImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    d_ptr->labelImage->setScaledContents(true);

    ui->widget->setWidget(d_ptr->labelImage);
    ui->widget->setAlignment(Qt::AlignCenter);

    QRect screen = QApplication::desktop()->screenGeometry(parentWidget());
    resize(0.7*screen.width(), 0.9*screen.height());
    move(screen.x()+0.15*screen.width(), screen.y()+0.05*screen.height());
    setFixedSize(size());
    adjustSize();

    for (const QString& p : pictures) {
        QFileInfo f(p);
        ui->listWidget->addItem(f.fileName());
    }
    if (pictures.size()) {
        ui->listWidget->setCurrentRow(0);
        d_ptr->loadFile(0);
    }

    ui->listWidget->setFocus();
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, [=](int r){ d_ptr->loadFile(r); });
    connect(ui->comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int idx){ d_ptr->setPictureAction(idx); });
}

PicAnalyzeResult::~PicAnalyzeResult() {
    delete d_ptr;
    delete ui;
}

bool PicAnalyzeResult::modified() const {
    return d_ptr->modified;
}

void PicAnalyzeResult::accept() {
    ui->progressBar->setVisible(true);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(d_ptr->pictures.size()-1);
    ui->progressBar->setValue(0);

    // Do the image treatment to all double page images
    for (const QString& f : d_ptr->pictures) {
        QFileInfo fi(f);
        QString fn = fi.fileName();
        int action = d_ptr->customActions.value(f, d_ptr->defaultAction);
        switch (action) {
        case 0: // Do nothing
            ui->lblMessage->setText(tr("Skipping ")+fn+"...");
            break;
        case 1: // Rotate Picture
            ui->lblMessage->setText(tr("Rotating ")+fn+"...");
            d_ptr->rotatePicture(f);
            break;
        case 2: // Split Picture
            ui->lblMessage->setText(tr("Splitting ")+fn+"...");
            d_ptr->splitPicture(f);
            break;
        }
        ui->progressBar->setValue(ui->progressBar->value()+1);
        QApplication::processEvents();
    }

    ui->progressBar->setVisible(false);
    QDialog::accept();
}
