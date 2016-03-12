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
    explicit RunExternalProcess(const QString& command, const QStringList& args, const QString& workingDirectory = QString(), QWidget *parent = 0);
    ~RunExternalProcess();

public slots:
    virtual int exec();

private:
    Ui::RunExternalProcess *ui;
};

#endif // RUNEXTERNALPROCESS_H
