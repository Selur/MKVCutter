#include "mkvcutter.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MkvCutter w;
    w.show();
    return a.exec();
}
