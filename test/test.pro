TEMPLATE = app
TARGET = 

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

LIBS += svgaplayer.lib

SOURCES += main.cpp
