TEMPLATE = lib
TARGET = svgaplayer

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../include

QMAKE_CXXFLAGS += /MP
QMAKE_CXXFLAGS += /GS
QMAKE_CXXFLAGS_RELEASE += /Zi

QMAKE_LFLAGS += /MACHINE:X86
QMAKE_LFLAGS_DEBUG += /debugtype:cv,fixup
QMAKE_LFLAGS_RELEASE += /DEBUG
QMAKE_LFLAGS_RELEASE += /OPT:REF /OPT:ICF

CONFIG(debug, debug|release) {
    DESTDIR = ../bin/debug
}
else {
    DESTDIR = ../bin/release
}

QMAKE_LIBDIR += "$(TargetDir)"

CharacterSet = 1

DEFINES += BUILD_SVGA

PRECOMPILED_HEADER = stable.h

include(jsoncpp/jsoncpp.pri)

HEADERS += zip/unzipex.h
SOURCES += zip/unzipex.cpp

HEADERS += svgaresource.h
SOURCES += svgaresource.cpp
HEADERS += svgawidget.h
SOURCES += svgawidget.cpp
HEADERS += gdisvgaplayer.h
SOURCES += gdisvgaplayer.cpp
