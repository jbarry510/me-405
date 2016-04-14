#include <QtGui/QApplication>
#include "bob.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    bob foo;
    foo.show();
    return app.exec();
}
