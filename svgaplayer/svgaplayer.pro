TEMPLATE = lib
TARGET = svgaplayer

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../include
INCLUDEPATH += ../3rd/directx-jun2010/include
INCLUDEPATH += ../3rd/glew-2.1.0/include
INCLUDEPATH += ../3rd/zlib-1.2.11/include
INCLUDEPATH += ../3rd/protobuf-3.3.0/include

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
QMAKE_LIBDIR += ../../3rd/directx-jun2010/lib/release

LIBS += opengl32.lib
LIBS += ../../3rd/glew-2.1.0/lib/libglew32.lib
LIBS += ../../3rd/zlib-1.2.11/lib/zlibstatic.lib
LIBS += ../../3rd/protobuf-3.3.0/lib/libprotobuf.lib

CharacterSet = 1

DEFINES += BUILD_SVGA

PRECOMPILED_HEADER = stable.h

include(jsoncpp/jsoncpp.pri)
HEADERS += zip/unzipex.h
SOURCES += zip/unzipex.cpp

HEADERS += proto/svga.pb.h
SOURCES += proto/svga.pb.cc

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
HEADERS += svgadx9canvas.h
SOURCES += svgadx9canvas.cpp
HEADERS += svgadx11canvas.h
SOURCES += svgadx11canvas.cpp
HEADERS += svgaglcanvas.h
SOURCES += svgaglcanvas.cpp
