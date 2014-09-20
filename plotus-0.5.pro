#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T12:08:48
#
#-------------------------------------------------

QT       += core gui
CONFIG   += qwt


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

# ----------------
#
# Compiling
#
# ----------------

TARGET = plotus
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

# ----------------
#
# Building
#
# ----------------

BUILDDIR = build

OBJECTS_DIR = $$BUILDDIR
MOC_DIR	= $$BUILDDIR
UI_DIR	= $$BUILDDIR
RCC_DIR	= $$BUILDDIR

# ----------------
#
# Installation
#
# ----------------

unix:!mac{
	# VARIABLES
	isEmpty(PREFIX) {
		PREFIX = /usr
	}

	# GNU STANDARD DIRECTORY VARIABLES
	BINDIR	= $$PREFIX/bin
	LIBDIR	= $$PREFIX/lib/$${TARGET}
	DATADIR	= $$PREFIX/share
	MANDIR	= $$DATADIR/man
	DOCDIR	= $$DATADIR/doc/$${TARGET}

	# SET RPATH
	QMAKE_RPATH =
	QMAKE_RPATHDIR =
	QMAKE_RPATHLINKDIR =
	QMAKE_LFLAGS += -Wl,-rpath,$$LIBDIR

	# MAKE INSTALL
	target.files += $${TARGET}
  	target.path = $$BINDIR

	libfiles.files += lib/*
	libfiles.path = $$LIBDIR

	pluginfiles.files += lib/plugins/*
	pluginfiles.path = $$LIBDIR/plugins

	manfiles.files += data/man/plotus.1
	manfiles.path = $$MANDIR/man1

	docfiles.files += data/doc/* README.md
	docfiles.path = $$DOCDIR

	menufiles.files += data/menu/*
	menufiles.path = $$DATADIR/applications

	iconfiles.files += rsc/plotus32.xpm rsc/plotus64.png
	iconfiles.path = $$DATADIR/pixmaps

	INSTALLS += target libfiles pluginfiles manfiles docfiles menufiles iconfiles
}


