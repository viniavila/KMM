#include "kmm.h"
#include <QApplication>

#include "projarchive.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
//    a.setApplicationDisplayName(a.tr("Kindle Manga Maker"));
//    a.setApplicationName("Kindle Manga Maker");
//    a.setApplicationVersion("0.1");
//    a.setOrganizationName("VAJ Software");
//    a.setOrganizationDomain("com.vajsoftware.viniavila");
//    kmm w;
//    w.show();

//    return a.exec();
    ProjArchive p("/tmp/teste.zip", ProjArchive::InputMode);
    //p.save("/tmp/tempTeste");
    p.extract("/tmp/tmpXtract");
    return 0;
}
