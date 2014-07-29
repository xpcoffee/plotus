#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T12:08:48
#
#-------------------------------------------------

QT       += core gui
CONFIG	+= qwt


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = BareMinimumPlotter
TEMPLATE = app


SOURCES += src/main.cpp \
    src/bareminimumplotter.cpp \
    src/bluejson.cpp \
    src/expression.cpp \
    src/inequalityinput.cpp \
    src/inequalityloader.cpp \
    src/variableinput.cpp \

HEADERS  += include/variableinput.h \
    include/bareminimumplotter.h \
    include/bluejson.h \
    include/expression.h \
    include/inequality.h \
    include/inequalityinput.h \
    include/inequalityloader.h \
    include/variable.h \

FORMS    +=  xml/bareminimumplotter.ui \
    xml/inequalityinput.ui \
    xml/inequalityloader.ui \
    xml/variableinput.ui

OTHER_FILES +=
