#include "upconfig.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UpConfig w;
    w.show();

    return a.exec();
}
