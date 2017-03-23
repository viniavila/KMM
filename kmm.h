#ifndef KMM_H
#define KMM_H

#include <QMainWindow>

namespace Ui {
class kmm;
}

class kmmPrivate;
class kmm : public QMainWindow {
    Q_OBJECT
    Q_DECLARE_PRIVATE(kmm)
public:
    kmm(QWidget *parent = 0);
    kmm(const QString& filepath, QWidget* parent = 0);
    ~kmm();

protected:
    virtual void closeEvent(QCloseEvent* e);

private:
    Ui::kmm *ui;
    kmmPrivate * const d_ptr;
};

#endif // KMM_H
