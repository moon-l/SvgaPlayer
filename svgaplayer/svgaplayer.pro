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

HEADERS += svgaplayer.h
SOURCES += svgaplayer.cpp
HEADERS += svgaresource.h
SOURCES += svgaresource.cpp
HEADERS += svgavideoentity.h
SOURCES += svgavideoentity.cpp
HEADERS += svgavideospriteentity.h
SOURCES += svgavideospriteentity.cpp
HEADERS += svgavideospriteframeentity.h
SOURCES += svgavideospriteframeentity.cpp
HEADERS += svgapath.h
SOURCES += svgapath.cpp
HEADERS += svgagdicanvas.h
SOURCES += svgagdicanvas.cpp
