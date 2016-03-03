#include "kmm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName(a.tr("Kindle Manga Maker"));
    a.setApplicationName("Kindle Manga Maker");
    a.setApplicationVersion("0.1");
    a.setOrganizationName("VAJ Software");
    a.setOrganizationDomain("com.vajsoftware.viniavila");
    kmm w;
    w.show();

    return a.exec();
}
