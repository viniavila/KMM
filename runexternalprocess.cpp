#include "runexternalprocess.h"
#include "ui_runexternalprocess.h"
#include <QProcess>

QProcess *process;
QString gcommand;
QStringList gargs;

RunExternalProcess::RunExternalProcess(const QString& command, const QStringList& args, const QString& workingDirectory, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunExternalProcess)
{
    ui->setupUi(this);
    gcommand = command;
    gargs = args;
    setWindowTitle(tr("Running") + " " + command);
    process = new QProcess;
    process->setProcessChannelMode(QProcess::MergedChannels);
    if (!workingDirectory.isEmpty())
        process->setWorkingDirectory(workingDirectory);
    connect(process, &QProcess::readyReadStandardOutput,
            [=](){ ui->plainTextEdit->appendPlainText(QString(process->readAllStandardOutput())); });
}

RunExternalProcess::~RunExternalProcess()
{
    process->terminate();
    if (!process->waitForFinished(10000))
        process->kill();
    delete ui;
}

int RunExternalProcess::exec() {
    ui->buttonBox->setEnabled(false);
    process->start(gcommand, gargs);
    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [=](int, QProcess::ExitStatus){ ui->buttonBox->setEnabled(true); });
    return QDialog::exec();
}
