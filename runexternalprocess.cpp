#include "runexternalprocess.h"
#include "ui_runexternalprocess.h"

RunExternalProcess::RunExternalProcess(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunExternalProcess)
{
    ui->setupUi(this);
}

RunExternalProcess::~RunExternalProcess()
{
    delete ui;
}
