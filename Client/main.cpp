#include "authorization.h"
#include "client.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);

    QApplication a(argc, argv);
    Client clientWindow;
    Authorization authoWindow;
    authoWindow.setClient(clientWindow);
    authoWindow.setFixedSize(435, 190);
    authoWindow.show();

    return a.exec();
}
