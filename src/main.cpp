#include <QApplication>
#include <QIcon>
#include "../include/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/pic/app_icon.ico"));
    MainWindow window;
    window.show();
    return app.exec();
} 