#include "preferences.h"
#include "ui_preferences.h"
#include <QSettings>
#include <QFileDialog>

QString selectFile(QWidget* parent) {
    QString fN = QFileDialog::getOpenFileName(parent,
                                              Preferences::tr("Select Executable Path"),
                                      #ifdef Q_OS_LINUX
                                              QString("/usr/bin"),
                                      #else
                                              QString(),
                                      #endif
                                              QString(), 0, 0);
    return fN;
}

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);
    QSettings s;
    ui->txtKindlegen->setText(s.value("KINDLEGEN_PATH").toString());
    ui->txtPython->setText(s.value("PYTHON_PATH").toString());

    connect(ui->btnKindlegen, &QPushButton::clicked, [=]() { ui->txtKindlegen->setText(selectFile(this)); });
    connect(ui->btnPython, &QPushButton::clicked, [=]() { ui->txtPython->setText(selectFile(this)); });
}

Preferences::~Preferences() {
    delete ui;
}

void Preferences::accept() {
    QSettings s;
    s.setValue("KINDLEGEN_PATH", ui->txtKindlegen->text());
    s.setValue("PYTHON_PATH", ui->txtPython->text());
    QDialog::accept();
}
