#include "kmm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kmm w;
    w.showMaximized();

    return a.exec();
}
