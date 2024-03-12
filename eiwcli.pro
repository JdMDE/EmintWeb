QT += core xml xmlpatterns 
HEADERS     = \
    project.h \
    global.h \
    cssstyles.h \
    state.h \
    transition.h \
    messagehandler.h \
    hdoc.h \
    srclib/html5document.h \
    srclib/tag.h \
    srclib/taglabeldefs.h \
    srclib/tagshtml5.h \
    srclib/tostring
SOURCES     = \
    maincli.cpp \
    project.cpp \
    state.cpp \
    transition.cpp \
    global.cpp \
    hdoc.cpp \
    srclib/html5document.cpp \
    srclib/tag.cpp \
    srclib/taglabeldefs.cpp
OBJECTS_DIR = cli
MOC_DIR = cli
LIBS += -lgumbo 
INCLUDEPATH += ./srclib

# install
unix {
 target.path = /usr/local/bin
 sources.files = $$SOURCES $$HEADERS *.pro
 sources.path = ./
 INSTALLS += target 
}
