QT += xml xmlpatterns svg core widgets gui
HEADERS     = \
    appwindow.h \
    project.h \
    renderarea.h \
    global.h \
    cssstyles.h \
    state.h \
    transition.h \
    messagehandler.h \
    creationdialog.h \
    newprojectdialog.h \
    newstatedialog.h \
    newtransitiondialog.h \
    hdoc.h \
    srclib/html5document.h \
    srclib/tag.h \
    srclib/taglabeldefs.h \
    srclib/tagshtml5.h \
    srclib/tostring
SOURCES     = \
    maingui.cpp \
    appwindow.cpp \
    project.cpp \
    renderarea.cpp \  
    state.cpp \
    transition.cpp \
    creationdialog.cpp \
    newprojectdialog.cpp \
    newstatedialog.cpp \
    newtransitiondialog.cpp \
    global.cpp \
    hdoc.cpp \
    srclib/html5document.cpp \
    srclib/tag.cpp \
    srclib/taglabeldefs.cpp

LIBS += -lgumbo
DEFINES += GUI
OBJECTS_DIR = gui
MOC_DIR = gui
INCLUDEPATH += ./srclib

# install
unix {
 target.path = /usr/local/bin
 sources.files = $$SOURCES $$HEADERS *.pro
 sources.path = ./
 INSTALLS += target 
}

OTHER_FILES += \
    ToDo.txt

