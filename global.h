/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <QString>

using namespace std;

#define APPNAME     "EmIntWeb"
#define APPLONGNAME "EmIntWeb Creator IDE"
#define INFO_STRING "EmIntWeb Creator\nA framework for creation of\n[Em]bedded [Int]eractive [Web] applications.\n@ by Juan Domingo, 2023\n\nBeta version for evaluation.\n\nThis program is distributed in source from under the GNU license."

#if (defined unix) || (defined __unix) || (defined __unix__) || (defined __APPLE__) || (defined __MACH__)
 #define PLATFORM_DIRSLASH QString("/")
 #define HERE QString("")
 #if (defined __APPLE__) || (defined __MACH__)
  #define INST_DIR QString("/sw/usr/share/emintweb/")
 #else
  #define INST_DIR QString("/usr/share/emintweb/")
 #endif
 #define DEFAULT_PEMFILE     QString("NoPemFile")
 #define DEFAULT_PEMKEY      QString("NoPemKey")
 #define DEFAULT_PEMKEYSIZE  QString("512")          // Even defined internally as int, the Reset constructor needs a QString
 #define DEFAULT_DHFILE      QString("NoDHFile")
 #define DEFAULT_HTML_DIR QString("HTMLFiles/")
 #define DEFAULT_CODE_DIR QString("CodeFiles/")
 #define STCODE_DIR    QString("States/")
 #define TRCODE_DIR    QString("Transitions/")
 #define WRCODE_DIR    QString("Wrappers/")
 #define MAINCODE_DIR  QString("MainProg/")
 #define MKDIR_COMMAND QString("mkdir")
 #define COPY_COMMAND  QString("cp")
#endif

/* Too dificult... Too many things to change not solved by these constants....
#ifdef Q_WS_WIN
 #define PLATFORM__DIRSLASH "\\"
 #define HERE ".\\"
 #define INST_DIR QString("C:\\emintweb\\")
 #define CODEPIECES_DIR INST_DIR+QString("codepieces\\")
 #define MKDIR_COMMAND QString("md ")      // Will this work...?
 #warning "This program has not been compiled for Windows platforms."
 #warning "I don't promise anything, and I won't be able to provide assistance. Use at you own risk."
#endif
*/

#if (defined Q_WS_QWS) || (defined Q_WS_WIN)
 #error "This program cannot be compiled for Windows or for mobile Linux platforms"
#endif

#ifndef PLATFORM_DIRSLASH
 #warning "I don't know in which platform we are compiling. I'll assume a Unix-like system"
 #define PLATFORM_DIRSLASH "/"
 #define HERE "./"
#endif

#define XSD_FILE_NAME           INST_DIR+QString("emintweb.xsd")
#define XSD_LOCALFILE_NAME      INST_DIR+QString("emintweb_localconf.xsd")

#define DEFAULT_PROJECT_NAME    QString("Untitled")
#define KEEP_PROJECT_NAME	QString("DontChangeMyName")
#define DEFAULT_HTTPS_MODE      false
#define DEFAULT_PFILE_EXTENSION QString(".emw")
#define DEFAULT_PFILE_BASENAME  QString("untitled")
#define DEFAULT_PFILE_NAME      DEFAULT_PFILE_BASENAME+DEFAULT_PFILE_EXTENSION

#define DEFAULT_MAKEFILE        QString("Makefile")
#define EIWCLI                  QString("eiwcli")

//#define CURSOR_SIZE  "big.xbm"
#define CURSOR_SIZE  "small.xbm"
#define NUM_CURSORS   5
#define CUR_DEFAULT   0
#define CUR_ADD_STATE 1
#define CUR_BITMAP_ADDST INST_DIR+"cursors"+PLATFORM_DIRSLASH+"ast_"
#define CUR_DELETE    2
#define CUR_BITMAP_DELETE INST_DIR+"cursors"+PLATFORM_DIRSLASH+"del_"
#define CUR_ADD_TRINI 3
#define CUR_BITMAP_TRINI INST_DIR+"cursors"+PLATFORM_DIRSLASH+"s1c_"
#define CUR_ADD_TRFIN 4
#define CUR_BITMAP_TRFIN INST_DIR+"cursors"+PLATFORM_DIRSLASH+"s2c_"

#define MAX_SON_PROCESSES 128

// These editors have not been choosen at will. We need to have programs that can
// run concurrently multiple instances of themselves and quit from one of them without
// killing the other instances. This is the purpose of the --new-window flag to gedit
// or --new-instance to geany and can be done, too, with kate -n. Unfortunately, bluefish
// doesn't allow such behaviour. geany allows it, but you should configure it for NOT opening formerly
// loaded files each time it is launched.
// Furthermore, it your platform is not Linux you will have to look for appropiate editors...
#define HTML_EDITOR               QString("/usr/bin/geany")
#define HTML_EDITOR_ARG1	  QString("--new-instance")

#define CPP_EDITOR                QString("/usr/bin/kate")
#define CPP_EDITOR_ARG1		  QString("-n");
#define CPP_EDITOR_SAME_WIN       QString("/usr/bin/kate")

#define COM_CPPTEST_ARGS          QString("-L/usr/local/lib -lht5cplus ")

// WARNING: this assumes g++...
#define COMP_FLAGS_SEP            QString("")
#define COMP_FLAGS_SEPS           QString(" ")
#define ADDIT_SOURCES_SEP         QString("")
#define ADDIT_SOURCES_SEPS        QString(" ") 
#define ADDIT_IDIRS_SEP           QString("-I")
#define ADDIT_IDIRS_SEPS          QString(" -I ")
#define ADDIT_LDIRS_SEP           QString("-L")
#define ADDIT_LDIRS_SEPS          QString(" -L ")
#define ADDIT_LIBS_SEP            QString("-l")
#define ADDIT_LIBS_SEPS           QString(" -l ")

#define DEFAULT_HEADER_EXTENSION  QString(".h")
#define DEFAULT_CPP_EXTENSION     QString(".cpp")
#define DEFAULT_OBJECT_EXTENSION  QString(".o")
#define DEFAULT_EXE_EXTENSION     QString("")

#define DEFAULT_SERVER_PORT       QString("20000")
#define DEFAULT_CPP_COMPILER      QString("g++")
#define DEFAULT_CPP_FLAGS         QString("-Wall")
#define DEFAULT_POCO_INCLUDE_DIR  QString("/usr/include/Poco")
#define DEFAULT_LOCAL_INCLUDE_DIR QString("/usr/local/include/")
#define DEFAULT_LOCAL_LIB_DIR     QString("/usr/local/lib/")
#define CONTEXT_BASENAME          QString("context")
#define CONTEXT_HEADER            CONTEXT_BASENAME+DEFAULT_HEADER_EXTENSION
#define CONTEXT_SOURCE            CONTEXT_BASENAME+DEFAULT_CPP_EXTENSION
#define SERVERDEB_BASENAME	  QString("serverdeb")
#define SERVERDEB_HEADER	  SERVERDEB_BASENAME+DEFAULT_HEADER_EXTENSION
#define DEFAULT_HT5CPP_LIB        QString("ht5cplus")
#define DEFAULT_POCOLIB1	  QString("PocoNet")
#define DEFAULT_POCOLIB2	  QString("PocoUtil")
#define DEFAULT_POCOLIB3	  QString("PocoFoundation")
#define DEFAULT_POCOLIB4	  QString("PocoNet")
#define DEFAULT_POCOLIB5	  QString("PocoNetSSL")
#define DEFAULT_PTHREAD		  QString("pthread")

#define MAIN_SKMB                 QString("main_skeleton_before")+DEFAULT_CPP_EXTENSION;
#define MAIN_SKMA                 QString("main_skeleton_after")+DEFAULT_CPP_EXTENSION;
#define MAIN_SKTA                 QString("main_skeleton_thread_after")+DEFAULT_CPP_EXTENSION;
#define MAIN_SERVERCALL		  QString("main_skeleton_servercall")+DEFAULT_CPP_EXTENSION;
#define MAIN_SERVER		  QString("main_skeleton_server")+DEFAULT_CPP_EXTENSION;

//#define COM_MAIN_COMMAND    QString("g++ -Wall -");

enum StateType { Initial, Terminal, Intermediate, NO_TYPE };

bool FileAccessible(QString fname);

#endif // GLOBAL_H
