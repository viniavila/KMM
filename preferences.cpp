#include "preferences.h"
#include "ui_preferences.h"
#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include <QDebug>

QString selectFile(const QString& path, QWidget* parent) {
    QString dir;
    if (path.isEmpty()) {
#ifdef Q_OS_LINUX
        dir = QString("/usr/bin");
#endif
    }
    else {
        QDir d(path);
        d.cdUp();
        dir = d.absolutePath();
    }
    QString fN = QFileDialog::getOpenFileName(parent, Preferences::tr("Select Executable Path"), dir, QString(), 0, 0);
    if (fN.isEmpty())
        return path;
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

    connect(ui->btnKindlegen, &QPushButton::clicked, [=]() { ui->txtKindlegen->setText(selectFile(ui->txtKindlegen->text(), this)); });
    connect(ui->btnPython, &QPushButton::clicked, [=]() { ui->txtPython->setText(selectFile(ui->txtPython->text(), this)); });
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
