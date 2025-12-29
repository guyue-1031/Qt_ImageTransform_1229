#include "imagetransform.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageTransform w;
    w.resize(900, 600);
    w.show();
    return a.exec();
}
