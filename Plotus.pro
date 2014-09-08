#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T12:08:48
#
#-------------------------------------------------

QT       += core gui
CONFIG   += qwt


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Plotus
TEMPLATE = app


SOURCES += src/main.cpp \
    src/bluejson.cpp \
    src/expression.cpp \
    src/inequalityinput.cpp \
    src/inequalityloader.cpp \
    src/variableinput.cpp \
    src/headerscrollarea.cpp \
    src/plotworker.cpp \
    src/exportdialog.cpp \
    src/inequality.cpp \
    src/plotus.cpp

HEADERS  += include/variableinput.h \
    include/bluejson.h \
    include/expression.h \
    include/inequality.h \
    include/inequalityinput.h \
    include/inequalityloader.h \
    include/variable.h \
    include/headerscrollarea.h \
    include/plotworker.h \
    include/exportdialog.h \
    include/plotus.h

FORMS    +=  \
    xml/inequalityinput.ui \
    xml/inequalityloader.ui \
    xml/variableinput.ui \
    xml/plotus.ui

OTHER_FILES +=

RESOURCES += \
    rsc/rsc.qrc
