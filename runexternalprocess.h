#ifndef RUNEXTERNALPROCESS_H
#define RUNEXTERNALPROCESS_H

#include <QDialog>

namespace Ui {
class RunExternalProcess;
}

class RunExternalProcess : public QDialog
{
    Q_OBJECT

public:
    explicit RunExternalProcess(QWidget *parent = 0);
    ~RunExternalProcess();

private:
    Ui::RunExternalProcess *ui;
};

#endif // RUNEXTERNALPROCESS_H
