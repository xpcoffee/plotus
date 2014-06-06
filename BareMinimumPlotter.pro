#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T12:08:48
#
#-------------------------------------------------

QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = BareMinimumPlotter
TEMPLATE = app


SOURCES += bareminimumplotter.cpp \
    qcustomplot.cpp \
    deplot/src/expression.cpp \
    main.cpp \
    variable_input/variableinput.cpp \
    inequality_input/inequalityinput.cpp

HEADERS  += bareminimumplotter.h \
    qcustomplot.h \
    deplot/include/expression.h \
    deplot/include/inequality.h \
    deplot/include/variable.h \
    variable_input/variableinput.h \
    inequality_input/inequalityinput.h

FORMS    += bareminimumplotter.ui \
    inequality_input/inequalityinput.ui \
    variable_input/variableinput.ui
