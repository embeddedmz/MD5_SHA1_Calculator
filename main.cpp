#define _CRT_SECURE_NO_WARNINGS // or use sprintf_s
#include <QApplication>
#include "MD5Calculator.h"

int main(int argc, char* argv[])
{
    QApplication App(argc, argv);

    Md5Calculator ProgramWindow;
    ProgramWindow.show();

    return App.exec();
}

