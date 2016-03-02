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
    explicit kmm(QWidget *parent = 0);
    ~kmm();

private:
    Ui::kmm *ui;
    kmmPrivate * const d_ptr;
};

#endif // KMM_H
