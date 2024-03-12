#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdlib>

#include <QDialog>
#include <QtGui>
#include <QtGui/QScreen>
#include <QTextEdit>
#include <QWidget>
#include <QScrollArea>
#include <QPalette>
#include <QScrollBar>
#include <QSlider>
#include <QApplication>
#include "project.h"

#include "cssstyles.h"
#include "renderarea.h"
#include "newprojectdialog.h"
#include "newstatedialog.h"
#include "newtransitiondialog.h"
  
class AppWindow : public QWidget
{
 Q_OBJECT

 public:
  AppWindow(QString projectfile,QApplication *theApp);

 // All these slots are triggered by the canvas through signals explicitly emitted by emit (and which, therefore, can have parameters)
 public slots:
  void CreateState(int px,int py);
  void EraseState(QString StateName);
  void CreateTransition(QString IniStateName,QList<QString> EndStatesNames,int px,int py);
  void EraseTransition(QString TransitionName);
  void AddStateMenu(QString NameState);
  void AddTransitionMenu(QString NameTrans);
  void CursorError(QString mode,QString curfile);

 // These slots are triggered by whichever routine that wants to show an error or warning.
  void E0(bool critical,QString e0);
  void E1(bool critical,QString e0,QString e1);
  void E2(bool critical,QString e0,QString e1,QString e2);
  void E3(bool critical,QString e0,QString e1,QString e2,QString e3);
  void E4(bool critical,QString e0,QString e1,QString e2,QString e3,QString e4);
  void E5(bool critical,QString e0,QString e1,QString e2,QString e3,QString e4,QString e5);

 private slots:
  // Slots triggered by the File menu (first item of the first menubar, the main menu):
  void NewProject();
  void LoadPFile();
  void SavePFile();
  void SaveAsPFile();
  void EditProject();
  void SaveSVG();
  void CheckAndExit();

  // Slot triggered by button which is second item of main menu:
  void AddState();
  // Slot triggered by button which is third item of main menu:
  void DelState();

  // Slot triggered by button which is fourth item of main menu:
  void AddTransition();
  // Slot triggered by button which is fifth item of main menu:
  void DelTransition();

  // Slots triggered by the Build menu (sixth item of main menu):
  void EditContextCode();
  void CompileContextCode();
  void GenCompStatesCode();
  void GenCompTransCode();
  void CheckAll();
  void ProjectGeneratesMain();
  void CompMain();
  void ProjectGeneratesMake();
  void RunMake();
  void RunMakeClean();
  void SetPort();
  void GetPort();
  void RunProgram();
  void KillProgram();

  // Slot triggered by button which is seventh item of main menu:
  void About();

  // Slots triggered by any of the dinamically created state-named menus in the State menubar (second menubar)
  // The state on which these slots act is passed by a QVariant
  void EditState();
  void EditStateFile();
  void GenStateCode();
  void CompStateCode();
  void GenCompTestStateCode();
  void MenuStateDone();

  // Slots triggered by any of the dinamically created transition-named menus in the Transition menubar (third menubar)
  // The transition on which these slots act is passed by a QVariant
  void EditTransition();
  void EditTransitionFile();
  void GenTransitionCode();
  void ComTransitionCode();
  void MenuTransitionDone();

 private:
  QApplication *myparent;
  Project P;
  QString pfile;
  QMenuBar *mbMain,*mbStates,*mbTransitions;
  QTextEdit *maProgOut;
  QScrollArea *sccanvas;
  RenderArea *canvas;
  QString port;
  QDialog *AskPort;
  QLineEdit *EPort;

  QProcess *processes[MAX_SON_PROCESSES];

  void CreateContextFilesIfNeeded();
  void CompCodeForState(QString NameState);
  void CompCodeForStateWithMain(QString NameState);
  void CompCodeForTransition(QString NameTrans);

  bool RunCommand(QString cmd,QStringList args,QString dir,QString &output,int waittime,string message="",string message_arg="");
  void KillSonProcesses();
};

#endif
