#-------------------------------------------------
#
# Project created by QtCreator 2014-01-22T17:13:21
#
#-------------------------------------------------

QT       -= core gui

TARGET = ht5cplus
TEMPLATE = lib
CONFIG += staticlib

SOURCES += srclib/html5document.cpp \
    srclib/tag.cpp \
    srclib/taglabeldefs.cpp

HEADERS += srclib/html5document.h \
    srclib/tag.h \
    srclib/taglabeldefs.h \
    srclib/tagshtml5.h \
    srclib/tostring

OBJECTS_DIR = lib

unix {
    target.path = /usr/local/lib
    headers.files = srclib/*.h srclib/tostring
    headers.path = /usr/local/include
    INSTALLS += target headers
}
