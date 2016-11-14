DEPENDPATH += .
INCLUDEPATH += .
TEMPLATE = app
QT += gui core widgets

CONFIG += c++11

FORMS += \
    md5calculator.ui

HEADERS += \
    md5calculator.h \
    md5.h \
    SHA1.h

SOURCES += \
    md5calculator.cpp \
    main.cpp \
    md5.cpp \
    SHA1.cpp

RESOURCES += \
    Icons.qrc

win32:RC_ICONS += md5.ico
win32:DEFINES += "_WINDOWS"

CONFIG(release, debug|release): CONFIG += release
CONFIG(debug, debug|release): CONFIG += debug
