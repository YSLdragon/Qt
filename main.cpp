#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.move(400, 100);      //设置主窗口位置
    w.setWindowTitle("系统监控器");
    w.show();

    return a.exec();
}
