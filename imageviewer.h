#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QDialog>

namespace Ui {
class ImageViewer;
}

class ImageViewerPrivate;
class ImageViewer : public QDialog {
    Q_OBJECT
    Q_DECLARE_PRIVATE(ImageViewer)
public:
    ImageViewer(QWidget *parent = 0);
    ImageViewer(const QStringList& files, QWidget *parent = 0);
    ImageViewer(const QString& path, QWidget *parent = 0);
    ~ImageViewer();

signals:
    void imageRotated();

private:
    Ui::ImageViewer *ui;
    ImageViewerPrivate * const d_ptr;
};

#endif // IMAGEVIEWER_H
