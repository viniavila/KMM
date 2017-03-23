#include "kmm.h"
#include <QApplication>
#include <QDebug>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName(a.tr("Kindle Manga Maker"));
    a.setApplicationName("Kindle Manga Maker");
    a.setApplicationVersion("0.1");
    a.setOrganizationName("VAJ Software");
    a.setOrganizationDomain("com.vajsoftware.viniavila");

    kmm *w;
    if (argc > 1) {
        QFileInfo fi(argv[1]);
        w = new kmm(fi.absoluteFilePath());
    }
    else
        w = new kmm();

    w->show();

    int r = a.exec();
    delete w;
    return r;
}
