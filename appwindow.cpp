#include "appwindow.h"

extern bool DEB;

AppWindow::AppWindow(QString projectfile,QApplication *theApp) : P(projectfile)
{
 myparent=theApp;
 // First, let's find the screen dimension, even if we are in a multi-screen system
 int w=100000,h=100000;
 foreach (QScreen *screen, QGuiApplication::screens())
 {
  QSize qs=screen->size();
  if (qs.height()<h)
   h=qs.height();
  if (qs.width()<w)
   w=qs.width();
 }
 if (DEB)
  cout << "Screen size set to (" << w << " x " << h << ")\n";
  
 setWindowTitle(APPLONGNAME);
 
 // These are the top-left corner, width and height where the window will be opened
 int winstartx=int(0.075*float(w));
 int winstarty=int(0.075*float(h));
 int winwidth=int(0.85*float(w));
 int winheight=int(0.85*float(h));
 
 setGeometry(winstartx,winstarty,winwidth,winheight);
 
 // Some values that needs to be initialized
 port="20000";

 for (int i=0;i<MAX_SON_PROCESSES;i++)
  processes[i]=NULL;

 // Initialization of the top bar for the main menu
 mbMain = new QMenuBar;
 mbMain->setMaximumHeight(30);
 mbMain->setStyleSheet(CSS_FOR_MENUBAR);
  
 // Initializations of all submenus in the main menu
 // Short-cut key will be:
 // n,l,s,a,v,x,d,e,t,i,p,g,m,k,r,b
 
 // Submenu File
 QMenu *mFile = new QMenu(tr("&File menu"),this);
 mFile->setStyleSheet(CSS_FOR_QMENU);

 QAction *aNewProject      = mFile->addAction(tr("&New project"));
 QAction *aLoadProject     = mFile->addAction(tr("&Load project"));
 QAction *aSaveProject     = mFile->addAction(tr("&Save project"));
 QAction *aSaveProjectAs   = mFile->addAction(tr("S&ave project as..."));
 QAction *aEditProject     = mFile->addAction(tr("Edit project..."));
 QAction *aSVGProject      = mFile->addAction(tr("Save canvas as S&VG"));
 QAction *aExit            = mFile->addAction(tr("E&xit"));
 
 mbMain->addMenu(mFile);

 // Add/delete state and add/delete transition are not submenus but buttons directly visible in the top menu
 QAction *aAddState        = mbMain->addAction(tr("A&dd State"));
 QAction *aDelState        = mbMain->addAction(tr("D&el State"));

 QAction *aAddTransition   = mbMain->addAction(tr("Add &Transition"));
 QAction *aDelTransition   = mbMain->addAction(tr("Del Trans&ition"));

 // Submenu build
 QMenu *mBuild = new QMenu(tr("&Build menu"),this);
 mBuild->setStyleSheet(CSS_FOR_QMENU);

 QAction *aEditContext      = mBuild->addAction(tr("Edit header and code context files"));
 QAction *aCompileContext   = mBuild->addAction(tr("Compile context code"));
 QAction *aGenStatesCode    = mBuild->addAction(tr("Generate and compile code for all states"));
 QAction *aGenTransCode     = mBuild->addAction(tr("Generate and compile code for all transitions"));
 QAction *aCheckProject     = mBuild->addAction(tr("Check project"));
 QAction *aGenMain          = mBuild->addAction(tr("Generate main"));
 QAction *aCompMain         = mBuild->addAction(tr("Compile main"));
 QAction *aGenMakefile      = mBuild->addAction(tr("Generate &Makefile"));
 QAction *aExeMake          = mBuild->addAction(tr("Execute ma&ke"));
 QAction *aExeMakeClean     = mBuild->addAction(tr("Execute make clean"));
 QAction *aSetPort          = mBuild->addAction(tr("Set server port"));
 QAction *aRunProg          = mBuild->addAction(tr("&Run built program"));
 QAction *aKillProg         = mBuild->addAction(tr("Kill running program"));

 mbMain->addMenu(mBuild);

 // Again, about is not a submenu but a button in main menu
 QAction *aAbout            = mbMain->addAction(tr("A&bout"));

 // Now, the second-line menu for states...
 mbStates = new QMenuBar;
 mbStates->setMaximumHeight(30);
 mbStates->setStyleSheet(CSS_FOR_MENUBAR);
 
 // that initially has just the label "States"
 QLabel *ns = new QLabel(this);
 ns->setText(tr("States: "));
 ns->setMinimumSize(110,20);
 mbStates->setCornerWidget(ns,Qt::TopLeftCorner);

 // Now, the third-line menu for transitions...
 mbTransitions = new QMenuBar;
 mbTransitions->setStyleSheet(CSS_FOR_MENUBAR);
 
 mbTransitions->setMaximumHeight(30);
 // that initially has just the label "Transitions"
 QLabel *ts = new QLabel(this);
 ts->setText(tr("Transitions: "));
 ts->setMinimumSize(110,20);
 mbTransitions->setCornerWidget(ts,Qt::TopLeftCorner);

 // The central canvas, which is embedded inside a scroll area:
 sccanvas = new QScrollArea;
  
 canvas = new RenderArea;
 canvas->SetPointers(this,&P);
 // Since we don't now how large the needed canvas can be, we oversize it. When a project is loaded it will be resized.
 canvas->setGeometry(0,0,4*w,4*h);
 
 if (DEB)
  cout << "Canvas geometry provisionally set to (" << 4*w << " x " << 4*h << ")\n";
  
 // We'll draw on the canvas with black-over-white
 canvas->setStyleSheet(CSS_FOR_RENDERAREA);
 
 // The canvas is a son of the scroll area
 sccanvas->setWidget(canvas);
 
 // and the style of the scroll are must be tailored with a complex style sheet
 sccanvas->setStyleSheet(CSS_FOR_SCROLL_BARS);
  
 // Minimum size of the canvas scroll is related with the window size (which was in turn related with the screen size)
 sccanvas->setMinimumSize(int(0.5*winwidth),int(0.70*winheight));
 
 // The lower message area, which is a read-only text edit area 
 maProgOut = new QTextEdit;
 
 // Minimum size of the messages scroll are is related with the window size (which was in turn related with the screen size)
 maProgOut->setMinimumSize(int(0.980*winwidth),int(0.10*winheight));
 
 maProgOut->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
 maProgOut->setStyleSheet(CSS_FOR_SCROLL_BARS);
 
 // These are specific customization for the message text area
 QFont font("Helvetica");
 font.setPixelSize(14);
 maProgOut->setFont(font);
 maProgOut->setReadOnly(true);
 P.SetMessageAreaPointer(maProgOut);
 
 // Finally, the global layout. The five elements are stacked on top of each other
 QGridLayout *mainLayout = new QGridLayout;
 mainLayout->addWidget(mbMain,0,0,1,1);
 mainLayout->addWidget(mbStates,1,0,1,1);
 mainLayout->addWidget(mbTransitions,2,0,1,1);
 mainLayout->addWidget(sccanvas,3,0,1,1);
 mainLayout->addWidget(maProgOut,4,0,1,1);
 setLayout(mainLayout);

 // Connections for the canvas edition functions
 connect(canvas,SIGNAL(CreateState(int,int)),this,SLOT(CreateState(int,int)));
 connect(canvas,SIGNAL(EraseState(QString)),this,SLOT(EraseState(QString)));
 connect(canvas,SIGNAL(CreateTransition(QString,QList<QString>,int,int)),this,SLOT(CreateTransition(QString,QList<QString>,int,int)));
 connect(canvas,SIGNAL(EraseTransition(QString)),this,SLOT(EraseTransition(QString)));
 connect(canvas,SIGNAL(AddStateMenu(QString)),this,SLOT(AddStateMenu(QString)));
 connect(canvas,SIGNAL(AddTransitionMenu(QString)),this,SLOT(AddTransitionMenu(QString)));
 connect(canvas,SIGNAL(CursorError(QString,QString)),this,SLOT(CursorError(QString,QString)));
 
 // Connections for the menu entries
 connect(aNewProject,     SIGNAL(triggered()), this, SLOT(NewProject()));
 connect(aLoadProject,    SIGNAL(triggered()), this, SLOT(LoadPFile()));
 connect(aSaveProject,    SIGNAL(triggered()), this, SLOT(SavePFile()));
 connect(aSaveProjectAs,  SIGNAL(triggered()), this, SLOT(SaveAsPFile()));
 connect(aEditProject,    SIGNAL(triggered()), this, SLOT(EditProject()));
 connect(aSVGProject,     SIGNAL(triggered()), this, SLOT(SaveSVG()));
 connect(aExit,           SIGNAL(triggered()), this, SLOT(CheckAndExit()));

 connect(aAddState,       SIGNAL(triggered()), this, SLOT(AddState()));
 connect(aDelState,       SIGNAL(triggered()), this, SLOT(DelState()));

 connect(aAddTransition,  SIGNAL(triggered()), this, SLOT(AddTransition()));
 connect(aDelTransition,  SIGNAL(triggered()), this, SLOT(DelTransition()));

 connect(aEditContext,    SIGNAL(triggered()), this, SLOT(EditContextCode()));
 connect(aCompileContext, SIGNAL(triggered()), this, SLOT(CompileContextCode()));
 connect(aGenStatesCode,  SIGNAL(triggered()), this, SLOT(GenCompStatesCode()));
 connect(aGenTransCode,   SIGNAL(triggered()), this, SLOT(GenCompTransCode()));
 connect(aCheckProject,   SIGNAL(triggered()), this, SLOT(CheckAll()));
 connect(aGenMain,        SIGNAL(triggered()), this, SLOT(ProjectGeneratesMain()));
 connect(aCompMain,       SIGNAL(triggered()), this, SLOT(CompMain()));
 connect(aGenMakefile,    SIGNAL(triggered()), this, SLOT(ProjectGeneratesMake()));
 connect(aExeMake,        SIGNAL(triggered()), this, SLOT(RunMake()));
 connect(aExeMakeClean,   SIGNAL(triggered()), this, SLOT(RunMakeClean()));

 connect(aSetPort,        SIGNAL(triggered()), this, SLOT(SetPort()));
 connect(aRunProg,        SIGNAL(triggered()), this, SLOT(RunProgram()));
 connect(aKillProg,       SIGNAL(triggered()), this, SLOT(KillProgram()));

 connect(aAbout,          SIGNAL(triggered()), this, SLOT(About()));
 
 // Connections to allow the project class to emit errors
 connect(&P,SIGNAL(E0(bool,QString)),                                        this,SLOT(E0(bool,QString)));
 connect(&P,SIGNAL(E1(bool,QString,QString)),                                this,SLOT(E1(bool,QString,QString)));
 connect(&P,SIGNAL(E2(bool,QString,QString,QString)),                        this,SLOT(E2(bool,QString,QString,QString)));
 connect(&P,SIGNAL(E3(bool,QString,QString,QString,QString)),                this,SLOT(E3(bool,QString,QString,QString,QString)));
 connect(&P,SIGNAL(E4(bool,QString,QString,QString,QString,QString)),        this,SLOT(E4(bool,QString,QString,QString,QString,QString)));
 connect(&P,SIGNAL(E5(bool,QString,QString,QString,QString,QString,QString)),this,SLOT(E5(bool,QString,QString,QString,QString,QString,QString)));

}

void AppWindow::NewProject()
{
 if (!P.IsEmpty() && P.NeedsToBeSaved())
 {
  QMessageBox::StandardButton ret;
  ret = QMessageBox::warning(this, APPNAME,
                                  tr("The project has been modified.\n"
                                     "Do you want to save your changes?"),
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  switch (ret)
  {
   case QMessageBox::Discard: break;
   case QMessageBox::Save: P.Save(); break;
   case QMessageBox::Cancel: break;
   default: break;
  }
 }
 P.Clean();

 NewProjectDialog npd(this,&P,maProgOut);
 npd.Launch();

 if (!P.IsEmpty())
  P.CreateSubdirectories();

 canvas->update();
}

void AppWindow::E0(bool critical,QString e0)
{
 if (critical)
 {
  QMessageBox::critical(this,APPNAME,e0,QMessageBox::Ok);
  KillSonProcesses();
  QCoreApplication::exit(1);
 }
 else
  QMessageBox::warning(this,APPNAME,e0,QMessageBox::Ok);
}

void AppWindow::E1(bool critical,QString e0,QString e1)
{
 if (critical)
 {
  QMessageBox::critical(this,APPNAME,e0.arg(e1),QMessageBox::Ok);
  KillSonProcesses();
  QCoreApplication::exit(1);
 }
 else
  QMessageBox::warning(this,APPNAME,e0.arg(e1),QMessageBox::Ok);
}

void AppWindow::E2(bool critical,QString e0,QString e1,QString e2)
{
 if (critical)
 {
  QMessageBox::critical(this,APPNAME,e0.arg(e1).arg(e2),QMessageBox::Ok);
  KillSonProcesses();
  QCoreApplication::exit(1);
 }
 else
  QMessageBox::warning(this,APPNAME,e0.arg(e1).arg(e2),QMessageBox::Ok);
}

void AppWindow::E3(bool critical,QString e0,QString e1,QString e2,QString e3)
{
 if (critical)
 {
  QMessageBox::critical(this,APPNAME,e0.arg(e1).arg(e2).arg(e3),QMessageBox::Ok);
  KillSonProcesses();
  QCoreApplication::exit(1);
 }
 else
  QMessageBox::warning(this,APPNAME,e0.arg(e1).arg(e2).arg(e3),QMessageBox::Ok);
}

void AppWindow::E4(bool critical,QString e0,QString e1,QString e2,QString e3,QString e4)
{
 if (critical)
 {
  QMessageBox::critical(this,APPNAME,e0.arg(e1).arg(e2).arg(e3).arg(e4),QMessageBox::Ok);
  KillSonProcesses();
  QCoreApplication::exit(1);
 }
 else
  QMessageBox::warning(this,APPNAME,e0.arg(e1).arg(e2).arg(e3).arg(e4),QMessageBox::Ok);
}

void AppWindow::E5(bool critical,QString e0,QString e1,QString e2,QString e3,QString e4,QString e5)
{
 if (critical)
 {
  QMessageBox::critical(this,APPNAME,e0.arg(e1).arg(e2).arg(e3).arg(e4).arg(e5),QMessageBox::Ok);
  KillSonProcesses();
  QCoreApplication::exit(1);
 }
 else
  QMessageBox::warning(this,APPNAME,e0.arg(e1).arg(e2).arg(e3).arg(e4).arg(e5),QMessageBox::Ok);
}

void AppWindow::CursorError(QString mode,QString curfile)
{
 QMessageBox::information(this,"Cursor info",
            tr("Cursor bitmap for action %1 in file %1 could not be loaded.\nLoading default cursor.").arg(mode).arg(curfile),
            QMessageBox::Ok);
}

void AppWindow::LoadPFile()
{
 if (!P.IsEmpty() && P.NeedsToBeSaved())
 {
  QMessageBox::StandardButton ret;
  ret = QMessageBox::warning(this, APPNAME,
                                  tr("The project has been modified.\n"
                                     "Do you want to save your changes?"),
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  switch (ret)
  {
   case QMessageBox::Discard: break;
   case QMessageBox::Save: P.Save(); break;
   case QMessageBox::Cancel: break;
   default: break;
  }
 }

 QString pfileName = QFileDialog::getOpenFileName(this,
                      tr("Open Project"),( P.IsEmpty() ? HERE : P.GetPDir()),
                      tr("EMW Files (*%1)").arg(DEFAULT_PFILE_EXTENSION));
 if (!pfileName.isEmpty())
 {
  QString localpfileName=pfileName;
  localpfileName.replace(".emw","_localconf.emw");
  if ( !P.ReadFromFile(pfileName) )
   QMessageBox::warning(this,tr("Warning"),tr("File %1 or %2 not found or not correctly read.").arg(pfileName).arg(localpfileName));
  else
  {
   if (DEB)
    cout << "Project " << P.GetName().toStdString() << " correctly loaded from file " << pfileName.toStdString() << endl;
   if (pfileName.contains(PLATFORM_DIRSLASH))
   {
    int l=pfileName.lastIndexOf(PLATFORM_DIRSLASH[0]);
    pfileName=pfileName.mid(l+1);
    P.SetPFile(pfileName);
    if (DEB)
     cout << "The files to which this project will be saved are now " << (P.GetPDir()+P.GetPFile()).toStdString() << " and " << (P.GetPDir()+P.GetLocalPFile()).toStdString() << endl;
   }
   QPoint pmax;
   P.Extension(pmax);
   pmax.setX(pmax.x()+SWIDTH);
   pmax.setY(pmax.y()+int(float(SWIDTH)/float(ARE)));  
   canvas->setGeometry(0,0,int(1.5*pmax.x()),int(1.5*pmax.y()));
   canvas->update();
   QMessageBox::information(this,tr("Information"),tr("Files %1 and %2 correctly loaded.").arg(pfileName).arg(localpfileName));
  }
 }
}

void AppWindow::SavePFile()
{
 if ( !P.IsEmpty() )
  P.Save();
}

void AppWindow::SaveAsPFile()
{
 if ( !P.IsEmpty() )
 {
  QFileDialog qd(this);
  qd.setFileMode(QFileDialog::AnyFile);
  QString pfileName = qd.getSaveFileName(this);

  if (pfileName.isEmpty())
   return;

  if (!pfileName.endsWith(DEFAULT_PFILE_EXTENSION))
   pfileName += DEFAULT_PFILE_EXTENSION;

  P.SaveAs(pfileName);
 }
}

void AppWindow::EditProject()
{
 if (P.IsEmpty())
 {
  QMessageBox::warning(this, APPNAME,
                                  tr("There is no active project.\n"
                                     "Nothing to edit!"),
                                     QMessageBox::Ok);
  return;
 }

 NewProjectDialog npd(this,&P,maProgOut);
 npd.Launch();

 canvas->update();
}

void AppWindow::SaveSVG()
{
 if (!P.IsEmpty())
 {
  if (P.GetNumStates()>0)
   canvas->WriteSVG();
  else
   QMessageBox::warning(this,APPNAME,tr("There is no yet any state in the canvas!\nSVG file would be empty."));
 }
 else
  QMessageBox::warning(this,APPNAME,tr("Cannot save the SVG file. There is no active project.\nUse New project or Load project first."));
}

void AppWindow::CheckAndExit()
{
 if (P.IsEmpty() || !P.NeedsToBeSaved())
 {
  KillSonProcesses();
  exit(0);
 }

 QMessageBox::StandardButton ret;
 ret = QMessageBox::warning(this, APPNAME,
                                  tr("The project has been modified.\n"
                                     "Do you want to save your changes?"),
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

 switch (ret)
 {
  case QMessageBox::Save: P.Save(); break;
  case QMessageBox::Discard: KillSonProcesses(); QCoreApplication::exit(0); break;
  default: return;
 }
}

void AppWindow::AddStateMenu(QString NameState)
{
 QList<QAction *> l=mbStates->actions();
 bool found=false;
 for (QList<QAction *>::iterator it=l.begin();it!=l.end() && !found;++it)
  if ((*it)->text()==NameState)
   found=true;
 if (!found)
 {
  QMenu *mState = new QMenu(NameState,this);
  mState->setStyleSheet("* { background-color: #ABABAB; foreground-color: #000000; border: 1px solid black; }");
  QString label;
  QVariant ns(NameState);
  QVariant nsm("_"+NameState);

  label = tr("Edit State...");
  QAction *aEditState  = mState->addAction(label);
  aEditState->setData(ns);

  label = tr("Edit file %1").arg(P.GetHTMLFile(NameState));
  QAction *aEditFile   = mState->addAction(label);
  aEditFile->setData(ns);

  label = tr("Generate state C++ code...");
  QAction *aGenCode    = mState->addAction(label);
  aGenCode->setData(ns);

  label = tr("Compile state C++ code...");
  QAction *aCompCode   = mState->addAction(label);
  aCompCode->setData(ns);

  label = tr("Generate and compile C++ test program...");
  QAction *aGenCompTest = mState->addAction(label);
  // The action for generating and compiling objects and tests are almost the same, indeed, they use the
  // same code in a single function. Prepending an _ to the state name is just a way to differenciate
  // into the funcion call, since no more than one parameter can be passed with setData (or is difficult to do it)
  aGenCompTest->setData(nsm);

  label = tr("Done");
  QAction *aDone        = mState->addAction(label);
  aDone->setData(ns);

  mbStates->addMenu(mState);

  connect(aEditState,   SIGNAL(triggered()), this, SLOT(EditState()));
  connect(aEditFile,    SIGNAL(triggered()), this, SLOT(EditStateFile()));
  connect(aGenCode,     SIGNAL(triggered()), this, SLOT(GenStateCode()));
  connect(aCompCode,    SIGNAL(triggered()), this, SLOT(CompStateCode()));
  connect(aGenCompTest, SIGNAL(triggered()), this, SLOT(GenCompTestStateCode()));
  connect(aDone,        SIGNAL(triggered()), this, SLOT(MenuStateDone()));
 }
}

void AppWindow::AddState()
{
 // Here we just notify the canvas that we enter into add-state mode.
 cout << "AppWindow::AddState()\n";

 canvas->SetCurrentTask(RenderArea::AddingState);
}

// The canvas doesn't need notification in this case.
void AppWindow::EditState()
{
 QString namestate=(((QAction *)sender())->data()).toString();

 NewStateDialog nst(this,&P,maProgOut,namestate,0,0);
 nst.Launch();

 canvas->update();
}

// This is a slot called by a signal sent from the canvas when we have finished the grapical part of state addition
void AppWindow::CreateState(int px,int py)
{
 if (P.IsEmpty())
  return;

 NewStateDialog ntr(this,&P,maProgOut,"",px,py);
 ntr.Launch();

 canvas->update();
}

void AppWindow::DelState()
{
 // Here we just notify the canvas that we enter into delete-state mode.
 canvas->SetCurrentTask(RenderArea::DeletingState);
}

// This is a slot called by a signal sent from the canvas when we have finished the grapical part of state deletion
void AppWindow::EraseState(QString namestate)
{
 if (P.IsEmpty())
  return;
 bool ret=P.DelState(namestate);
 if (ret)
 {
  QList<QAction *> l=mbStates->actions();
  bool found=false;
  for (QList<QAction *>::iterator it=l.begin();it!=l.end() && !found;++it)
   if ((*it)->text()==namestate)
   {
    mbStates->removeAction(*it);
    found=true;
   }
  canvas->update();
 }
}

void AppWindow::AddTransition()
{
 // Here we just notify the canvas that we enter into add-transition mode.
 canvas->SetCurrentTask(RenderArea::AddingTransitionStep1);
}

// The canvas doesn't need notification in this case.
void AppWindow::EditTransition()
{
// OJO: revisarla. Es difícil
/*
 QString nametrans=(((QAction *)sender())->data()).toString();
 cout << "Editing transition " << nametrans.toStdString() << endl;

 QString inistate=P.GetTransitionInitialState(nametrans);
 QString fallstate=P.GetTransitionFallStateName(nametrans);
 QPoint point=P.GetTransitionPosition(nametrans);
 QList<QString> l=P.GetTransitionFinalStateList(nametrans);
 cout << "Calling NewTransitionDialog from AddTransition() with nametrans " << nametrans.toStdString() << endl;
 NewTransitionDialog nst(this,&P,maProgOut,nametrans,point.x(),point.y(),inistate,l,fallstate);
 nst.Launch();

 canvas->update();
*/
 QMessageBox::information(this,"Future work",
            tr("Sorry, this feature is not yet implemented.\nBy now, you will have to delete the transition and recreate it."),
            QMessageBox::Ok);
}

void AppWindow::AddTransitionMenu(QString nametrans)
{
 QList<QAction *> l=mbTransitions->actions();
 bool found=false;
 for (QList<QAction *>::iterator it=l.begin();it!=l.end() && !found;++it)
  if ((*it)->text()==nametrans)
   found=true;
 if (!found)
 {
  QMenu *tmenu = new QMenu(nametrans,this);
  tmenu->setStyleSheet("* { background-color: #ABABAB; foreground-color: #000000; border: 1px solid black; }");
  QString actlab;
  QVariant nt(nametrans);

  actlab = tr("Edit Transition...");
  QAction *editentity_action   = tmenu->addAction(actlab);
  editentity_action->setData(nt);

  actlab = tr("Generate transition code... ");
  QAction *gentrans_action     = tmenu->addAction(actlab);
  gentrans_action->setData(nt);

  actlab = tr("Edit transition code... ");
  QAction *editcode_action     = tmenu->addAction(actlab);
  editcode_action->setData(nt);

  actlab = tr("Compile C++ transition code...");
  QAction *compile_action      = tmenu->addAction(actlab);
  compile_action->setData(nt);

  actlab = tr("Done");
  QAction *done_action         = tmenu->addAction(actlab);
  done_action->setData(nt);

  mbTransitions->addMenu(tmenu);

  connect(editentity_action,   SIGNAL(triggered()), this, SLOT(EditTransition()));
  connect(gentrans_action,     SIGNAL(triggered()), this, SLOT(GenTransitionCode()));
  connect(editcode_action,     SIGNAL(triggered()), this, SLOT(EditTransitionFile()));
  connect(compile_action,      SIGNAL(triggered()), this, SLOT(ComTransitionCode()));
  connect(done_action,         SIGNAL(triggered()), this, SLOT(MenuTransitionDone()));
 }
}

// This is a slot called by a signal sent from the canvas when we have finished the grapical part of transition creation
void AppWindow::CreateTransition(QString nameorigstate,QList<QString> nameendstates,int px,int py)
{
 if (!P.StateExists(nameorigstate))
 {
  QMessageBox::warning(this, APPNAME,
                             tr("Cannot create transition starting in state %1. Such state does not exists.").arg(nameorigstate),
                             QMessageBox::Ok);
  return;
 }
 for (int i=0;i<nameendstates.size(); i++)
  if ( !P.StateExists(nameendstates[i]) )
  {
   QMessageBox::warning(this, APPNAME,
                              tr("Cannot create transition ending in state %1. Such state does not exists.").arg(nameendstates[i]),
                              QMessageBox::Ok);
   return;
  }

 if (DEB)
 {
  cout << "Creating transition from state " << nameorigstate.toStdString() << " to states ";
  for (int i=0;i<nameendstates.size(); i++)
   cout << nameendstates[i].toStdString() << " ";
  cout << endl; 
 }
 
 NewTransitionDialog ntr(this,&P,maProgOut,"",px,py,nameorigstate,nameendstates,"");
 ntr.Launch();

 canvas->update();
}

void AppWindow::DelTransition()
{
 // Here we just notify the canvas that we enter into delete-transition mode.
 canvas->SetCurrentTask(RenderArea::DeletingTransition);
}

// This is a slot called by a signal sent from the canvas when we have finished the grapical part of transition deletion
void AppWindow::EraseTransition(QString nametransition)
{
 bool ret=P.DelTransition(nametransition);
 if (ret)
 {
  QList<QAction *> l=mbTransitions->actions();
  bool found=false;
  for (QList<QAction *>::iterator it=l.begin();it!=l.end() && !found;++it)
  if ((*it)->text()==nametransition)
  {
   mbTransitions->removeAction(*it);
   found=true;
  }
  canvas->update();
 }
}

void AppWindow::EditContextCode()
{
 QString hf=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+CONTEXT_HEADER;
 QString cf=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+CONTEXT_SOURCE;
 QString hd=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+SERVERDEB_HEADER;
 
 QString BannerContent="";
 if (!FileAccessible(hf))
 {
  int ret=QMessageBox::warning(this,APPNAME,tr("File %1 does not exist. Do you want to create it from the template?").arg(hf),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (ret==QMessageBox::Cancel)
   return;
  if (ret==QMessageBox::Yes)
  {
   QString com=COPY_COMMAND;
   QStringList args;
   args << INST_DIR+CONTEXT_BASENAME+"_skeleton"+DEFAULT_HEADER_EXTENSION << hf;
   bool ret1=RunCommand(com,args,"",BannerContent,-1);
   if (!ret1)
    QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
  }
 }

 BannerContent="";
 if (!FileAccessible(cf))
 {
  int ret=QMessageBox::warning(this,APPNAME,tr("File %1 does not exist. Do you want to create it from the template?").arg(cf),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (ret==QMessageBox::Cancel)
   return;
  if (ret==QMessageBox::Yes)
  {
   QString com=COPY_COMMAND;
   QStringList args;
   args << INST_DIR+CONTEXT_BASENAME+"_skeleton"+DEFAULT_CPP_EXTENSION << cf;
   bool ret1=RunCommand(com,args,"",BannerContent,-1);
   if (!ret1)
    QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
  }
 }
 
 BannerContent="";
 if (!FileAccessible(hd))
 {
  int ret=QMessageBox::warning(this,APPNAME,tr("File %1 does not exist. Do you want to create it from the template?").arg(hd),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (ret==QMessageBox::Cancel)
   return;
  if (ret==QMessageBox::Yes)
  {
   QString com=COPY_COMMAND;
   QStringList args;
   args << INST_DIR+SERVERDEB_BASENAME+"_skeleton"+DEFAULT_HEADER_EXTENSION << hd;
   bool ret1=RunCommand(com,args,"",BannerContent,-1);
   if (!ret1)
    QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
  }
 }
 
 QString c=CPP_EDITOR_SAME_WIN;
 QStringList args;
 args << cf;
 BannerContent="";
 bool ret=RunCommand(c,args,"",BannerContent,0);
 if (!ret)
  QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
}

// WARNING: g++ specific (program options)
void AppWindow::CompileContextCode()
{
 QString sourcefile=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+CONTEXT_SOURCE;
 QString objectfile=sourcefile;
 objectfile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_OBJECT_EXTENSION);

 QString compiler=P.GetCppCompiler();
 QStringList args=P.GetCompFlags();

 QStringList includedirs=P.GetAdditionalIncludeDirs();
 if (!includedirs.isEmpty())
  for (QStringList::iterator it=includedirs.begin();it!=includedirs.end();++it)
    args << *it;
    
 args << "-c" << "-o" << objectfile << sourcefile;

 QString BannerContent;
 bool ret=RunCommand(compiler,args,"",BannerContent,-1,"Context code '%1' compiled.\n",sourcefile.toStdString());
 if (!ret)
  QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
}

void AppWindow::GenCompStatesCode()
{
 // If there is any error in the code generation or compilation for any state the inner functions will raise a warning.
 // The code for the other states will be generated and compiled in any case.
 QList<QString> l=P.GetStateNameList();
 for (int i=0;i<l.size();i++)
 {
  P.GenCodeForState(l[i],false);
  maProgOut->insertPlainText(tr("Code for state '%1' generated. ").arg(l[i]));
  myparent->processEvents();
  CompCodeForState(l[i]); 
  myparent->processEvents();
 }
}

void AppWindow::CompCodeForTransition(QString NameTrans)
{
 QString CodeFile=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+P.GetTransitionCodeFile(NameTrans);
 QString WCodeFile=P.GetPDir()+P.GetCodeDir()+WRCODE_DIR+"wrapper_"+P.GetTransitionCodeFile(NameTrans);

 if (!FileAccessible(WCodeFile))
 {
  QMessageBox::warning(this, APPNAME,tr("The file %1 does not exists.\nPlease, generate it first.").arg(WCodeFile),QMessageBox::Ok);
  return;
 }
 if (!FileAccessible(CodeFile))
 {
  QMessageBox::warning(this, APPNAME,tr("The file %1 does not exists.\nPlease, generate it first.").arg(WCodeFile),QMessageBox::Ok);
  return;
 }

 QString ObjectFile=CodeFile;
 ObjectFile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_OBJECT_EXTENSION);
 // WARNING: g++ specific
 QString com=P.GetCppCompiler();
 QStringList args=P.GetCompFlags();
 args.push_front(CodeFile);
 args.push_front(QString("-c"));
 
 QString APath=P.GetPDir()+P.GetCodeDir();
 args << QString("-I") << APath+STCODE_DIR;
 args << QString("-I") << APath+TRCODE_DIR;
 args << QString("-I") << APath+WRCODE_DIR;
 args << QString("-I") << APath+MAINCODE_DIR;
 
 QStringList AddInc=P.GetAdditionalIncludeDirs();
 for (int i=0;i<AddInc.size();i++)
  args << AddInc[i];
  
 args << QString("-o") << ObjectFile;

 QString cmdplusargs=com+" ";
 for (QStringList::iterator it=args.begin();it!=args.end();++it)
  cmdplusargs += " "+(*it);
 
 QString BannerError;
 bool ret=RunCommand(com,args,"",BannerError,-1,"Code for transition '%1' compiled. ",NameTrans.toStdString());
 if (!ret)
 {
  QMessageBox::warning(this, APPNAME,tr("Command %1 has failed. See messages area.").arg(cmdplusargs),QMessageBox::Ok);
  return;
 }

 ObjectFile=WCodeFile;
 ObjectFile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_OBJECT_EXTENSION);
 com  = P.GetCppCompiler();
 
 args.clear();
 args = P.GetCompFlags();
 args.push_front(WCodeFile);
 args.push_front(QString("-c"));
 
 args << "-I" << APath+STCODE_DIR;
 args << "-I" << APath+TRCODE_DIR;
 args << "-I" << APath+WRCODE_DIR;
 args << "-I" << APath+MAINCODE_DIR;

 for (int i=0;i<AddInc.size();i++)
  args << AddInc[i];

 args << "-o" << ObjectFile;

 cmdplusargs=com+" ";
 for (QStringList::iterator it=args.begin();it!=args.end();++it)
  cmdplusargs += " "+*it;
  
 BannerError="";
 ret=RunCommand(com,args,"",BannerError,-1,"Code for wrapper of transition '%1' compiled.\n",NameTrans.toStdString());
 if (!ret)
 {
  QMessageBox::warning(this, APPNAME,tr("Command %1 has failed. See messages area.").arg(cmdplusargs),QMessageBox::Ok);
  return;
 }
}

void AppWindow::GenCompTransCode()
{
 // If there is any error in the code generation or compilation for any transition the inner function will raise a warning.
 // The code for the other transitions will be generated and compiled in any case.
 QList<QString> l=P.GetTransitionNameList();
 for (int i=0;i<l.size();i++)
 {
  P.GenCodeForTransition(l[i]);
  //maProgOut->insertPlainText(tr("Code for state '%1' generated. ").arg(l[i]));
  myparent->processEvents();
  CompCodeForTransition(l[i]);
  myparent->processEvents();
 }
}

// WARNING: compiler-specific (-c and -I options)
void AppWindow::CompMain()
{
 QString MainSource=P.GetPDir()+P.GetCodeDir()+MAINCODE_DIR+P.GetName()+DEFAULT_CPP_EXTENSION;

 QString com=P.GetCppCompiler();
 QStringList args=P.GetCompFlags();
 args.push_front(MainSource);
 args.push_front("-c");
 
 args << QString("-I") << P.GetPDir()+P.GetCodeDir()+STCODE_DIR;
 args << QString("-I") << P.GetPDir()+P.GetCodeDir()+TRCODE_DIR;
 
 QStringList IDirs=P.GetAdditionalIncludeDirs();
 for (int i=0;i<IDirs.size();i++)
  args << IDirs[i];
 
 QString BannerError="";
 bool ret=RunCommand(com,args,"",BannerError,-1,"Main program in file '%1' compiled.\n",MainSource.toStdString());
 QString cmdplusargs=com+" ";
 for (int i=0;i<args.size();i++)
  cmdplusargs += args[i] + " ";
  
 if (!ret)
 {
  QMessageBox::warning(this, APPNAME,tr("Command %1 has failed. See messages area.").arg(cmdplusargs),QMessageBox::Ok);
  return;
 }
}

void AppWindow::CheckAll()
{
 if (!P.CheckProject())
  QMessageBox::warning(this,APPNAME,tr("Something wrong in the project description. Please, see messages area."),QMessageBox::Ok);
}

void AppWindow::ProjectGeneratesMain()
{
 P.GenMain();
 QString MainSource=P.GetPDir()+P.GetCodeDir()+MAINCODE_DIR+P.GetName()+DEFAULT_CPP_EXTENSION;
 maProgOut->insertPlainText(tr("Main program generated and written to file '%1'\n").arg(MainSource));
}

void AppWindow::ProjectGeneratesMake()
{
 P.GenMake();
 QString makefile=P.GetPDir()+P.GetCodeDir()+"Makefile";
 maProgOut->insertPlainText(tr("Makefile generated and written to file '%1'\n").arg(makefile));
}

// WARNING: we need a standard make program installed.
void AppWindow::RunMake()
{
 QString makedir=P.GetPDir()+P.GetCodeDir();
 QString c="make";
 QStringList args;
 args << "-f" << DEFAULT_MAKEFILE;
 
 QString BannerContent;
 bool ret=RunCommand(c,args,makedir,BannerContent,-1);
 if (!ret)
  QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
}

// WARNING: we need a standard make program installed.
void AppWindow::RunMakeClean()
{
 QString makedir=P.GetPDir()+P.GetCodeDir();
 QString c="make";
 QStringList args;
 args << "-f" << DEFAULT_MAKEFILE << "clean";

 QString BannerContent;
 bool ret=RunCommand(c,args,makedir,BannerContent,-1);
 if (!ret)
  QMessageBox::warning(this,APPNAME,BannerContent,QMessageBox::Ok);
}

void AppWindow::RunProgram()
{
/* OJO: rehacer. Faltan los argumentos adicionales, y además no parece funcionar incluso si no los hay
 if ((processes[MAX_SON_PROCESSES-1]!=NULL) && processes[MAX_SON_PROCESSES-1]->state()!=QProcess::NotRunning)
  processes[MAX_SON_PROCESSES-1]->close();
 processes[MAX_SON_PROCESSES-1] = new QProcess;

 QString cmd=P.GetPDir()+P.GetCodeDir()+MAINCODE_DIR+P.GetName();
 QStringList args;
 args.push_back(port);
 
 QString cmdplusargs = cmd+" "+port;
 
 processes[MAX_SON_PROCESSES-1]->start(cmd,args);
 if (!processes[MAX_SON_PROCESSES-1]->waitForStarted())
 {
  maProgOut->append(tr("Process %1 could not start.").arg(cmdplusargs));
  delete processes[MAX_SON_PROCESSES-1];
  return;
 }

 QByteArray sout=processes[MAX_SON_PROCESSES-1]->readAllStandardOutput();
 QByteArray serr=processes[MAX_SON_PROCESSES-1]->readAllStandardError();

 QString ro(sout);
 QString re(serr);

 maProgOut->append(tr("Running command '%1'").arg(cmdplusargs));

 if ( processes[MAX_SON_PROCESSES-1]->exitCode()==0 )
 {
  maProgOut->append(tr("Command '%1' returns 0. Good.").arg(cmdplusargs));
  return;
 }

 delete processes[MAX_SON_PROCESSES-1];
 processes[MAX_SON_PROCESSES-1]=NULL;

 maProgOut->append(tr("Process %1 ended with error.").arg(cmdplusargs));
 maProgOut->append(tr("Standard output was:"));
 maProgOut->append(QString::fromUtf8(ro.toStdString().c_str()));
 maProgOut->append(tr("Standard error was:"));
 maProgOut->append(QString::fromUtf8(re.toStdString().c_str()));
*/
 QMessageBox::information(this,"Future work",
            tr("Sorry, this feature is not yet implemented.\nBy now, you will have to run the executable from the command line."),
            QMessageBox::Ok);
}

void AppWindow::KillProgram()
{
/* OJO: rehacer cuando funcione bien RunProgram()
 if ((processes[MAX_SON_PROCESSES-1]!=NULL) && processes[MAX_SON_PROCESSES-1]->state()==QProcess::Running)
 {
  long long int pid=processes[MAX_SON_PROCESSES-1]->processId();
  maProgOut->append(tr("Killing process %1").arg(pid));
  processes[MAX_SON_PROCESSES-1]->close();
 }
 if (processes[MAX_SON_PROCESSES-1]!=NULL)
  delete processes[MAX_SON_PROCESSES-1];
 processes[MAX_SON_PROCESSES-1]=NULL;
*/
 QMessageBox::information(this,"Future work",
            tr("Sorry, this feature is not yet implemented.\nBy now, you will have to kill the executable with kill -9 <process> or Ctril-C"),
            QMessageBox::Ok);
}

void AppWindow::About()
{
 QMessageBox::information(this,tr("Information"),tr(INFO_STRING));
}

void AppWindow::EditStateFile()
{
 QString namestate=(((QAction *)sender())->data()).toString();

 QString fname=P.GetPDir()+P.GetHTMLDir()+P.GetHTMLFile(namestate);

 if (DEB)
  cout << "Editing state file " << fname.toStdString() << " for state " << namestate.toStdString() << endl;

 if (!FileAccessible(fname))
 {
  int ret=QMessageBox::warning(this, APPNAME,tr("The file %1 does not exist yet.\nDo you want to create it?").arg(fname),QMessageBox::Ok | QMessageBox::Cancel);
  if ( ret == QMessageBox::Cancel )
   return;

  ofstream f(fname.toStdString().c_str(),ofstream::out);
  if (f.is_open())
  {
   f << HTML5HEADER;
   f << "<html>\n<head>\n</head>\n<body>\n</body>\n</html>\n";
   f.close();
  }
  else
  {
   QMessageBox::warning(this, APPNAME,tr("The file %1 could not be created.\nMay be a permission problem?").arg(fname),QMessageBox::Ok);
   return;
  }
 }


 QString command=HTML_EDITOR;
 QStringList args;
 #ifdef HTML_EDITOR_ARG1
 args << HTML_EDITOR_ARG1;
 #endif
 args << fname;

 QString BannerContent;
 bool ret=RunCommand(command,args,"",BannerContent,0);
 if (!ret)
  QMessageBox::warning(this,APPNAME,tr("Could not call HTML editor. See messages area."));
}

void AppWindow::GenStateCode()
{
 QString ns=(((QAction *)sender())->data()).toString();

 if (ns[0]=='_')
 {
  P.GenCodeForState(ns.mid(1),true);
  maProgOut->insertPlainText(tr("Code for testing state '%1' in a complete program generated. ").arg(ns.mid(1)));
 }
 else
 {
  P.GenCodeForState(ns,false);
  maProgOut->insertPlainText(tr("Code for state '%1' generated. ").arg(ns));
 }
}

void AppWindow::CompStateCode()
{
 QString ns=(((QAction *)sender())->data()).toString();

 if (ns[0]=='_')
  CompCodeForStateWithMain(ns.mid(1));
 else
  CompCodeForState(ns);
}

void AppWindow::CompCodeForState(QString NameState)
{
 QString CodeFile=P.GetPDir()+P.GetCodeDir()+STCODE_DIR+P.GetStateCodeFile(NameState);
 QString ContextObjectFile;
 
 QString ExecFile=CodeFile;
 ExecFile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_EXE_EXTENSION);

 QString ObjectFile=CodeFile;
 ObjectFile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_OBJECT_EXTENSION);

 if (!FileAccessible(CodeFile))
 {
  QMessageBox::warning(this, APPNAME,tr("The file %1 does not exists.\nPlease, create it first.").arg(CodeFile),QMessageBox::Ok);
  return;
 }

// WARNING: compiler-specific
 QString com=P.GetCppCompiler();
 QStringList args=P.GetCompFlags();
 args.push_front(QString("-c"));

 QStringList IDirs=P.GetAdditionalIncludeDirs();
 for (int i=0;i<IDirs.size();i++)
  args << IDirs[i];
 // This is because the context code file is into the transition directory.
 args << QString("-I") << P.GetPDir()+P.GetCodeDir()+TRCODE_DIR;
 
 args << CodeFile << QString("-o") << ObjectFile;

 QString complusargs = com+" ";
  for (int i=0;i<args.size();i++)
   complusargs += args[i] + " ";
 
 QString BannerError;
 bool ret=RunCommand(com,args,"",BannerError,-1,"Code for state '%1' compiled to object file.\n",NameState.toStdString());
 if (!ret)
  QMessageBox::warning(this, APPNAME, tr("Command %1 failed. See messages area.").arg(complusargs),QMessageBox::Ok);
}

void AppWindow::CompCodeForStateWithMain(QString NameState)
{
 QString CodeFile=P.GetPDir()+P.GetCodeDir()+STCODE_DIR+P.GetStateCodeFile(NameState);
 QString ContextObjectFile;

 CodeFile.replace(DEFAULT_CPP_EXTENSION,"_test"+DEFAULT_CPP_EXTENSION);
 
 QString ExecFile=CodeFile;
 ExecFile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_EXE_EXTENSION);

 if (!FileAccessible(CodeFile))
 {
  QMessageBox::warning(this, APPNAME,tr("The file %1 does not exists.\nPlease, create it first.").arg(CodeFile),QMessageBox::Ok);
  return;
 }

// WARNING: compiler-specific
 QString com=P.GetCppCompiler();
 QStringList args=P.GetCompFlags();

 QStringList IDirs=P.GetAdditionalIncludeDirs();
 //IDirs << QString("-I") << P.GetPDir()+P.GetCodeDir()+TRCODE_DIR;
 for (int i=0;i<IDirs.size();i++)
  args << IDirs[i];

 args << CodeFile;
 
 QStringList LDirs=P.GetAdditionalLibDirs();
 if (!LDirs.isEmpty())
  for (int i=0;i<LDirs.size();i++)
  {
   if (LDirs[i] != QString(""))
    args << LDirs[i];
  }
 // WARNING: the order here IS important (at least, for g++).
 // Used libraries must go at the end.
 args << QString("-o") << ExecFile;

 QStringList Libs=P.GetAdditionalLibraries();
 if (!Libs.isEmpty())
  for (int i=0;i<Libs.size();i++)
  {
   if (Libs[i] != QString(""))
    args << Libs[i];
  }
 
 QString complusargs = com+" ";
  for (int i=0;i<args.size()-1;i++)
   complusargs += (args[i] + " ");
 complusargs += args[args.size()-1];
 
 QString BannerError;
 bool ret=RunCommand(com,args,"",BannerError,-1,"Test code for state '%1' compiled to executable program.\n",NameState.toStdString());
 if (!ret)
  QMessageBox::warning(this, APPNAME, tr("Command %1 failed. See messages area.").arg(complusargs),QMessageBox::Ok);
}

void AppWindow::GenCompTestStateCode()
{
 QString ns=(((QAction *)sender())->data()).toString();

 if (ns[0]=='_')
 {
  P.GenCodeForState(ns.mid(1),true);
  maProgOut->insertPlainText(tr("Code for testing state '%1' in a complete program generated. ").arg(ns.mid(1)));
  CompCodeForStateWithMain(ns.mid(1));
 }
 else
 {
  P.GenCodeForState(ns,false);
  maProgOut->insertPlainText(tr("Code for state '%1' generated. ").arg(ns));
  CompCodeForState(ns);
 }
}

void AppWindow::CreateContextFilesIfNeeded()
{
 QString ConHFile=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+CONTEXT_HEADER;
 QString ConCppFile=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+CONTEXT_SOURCE;

 if (!FileAccessible(ConHFile))
 {
  QString ConSkHFile=INST_DIR+CONTEXT_HEADER;
  ConSkHFile.replace(DEFAULT_HEADER_EXTENSION,"_skeleton"+DEFAULT_HEADER_EXTENSION);
  ifstream hfk(ConHFile.toStdString().c_str(),ifstream::in);
  if (!hfk.is_open())
  {
   QMessageBox::critical(this,APPNAME,tr("The skeleton file %1 does not exists. Please, install the emintweb program correctly").arg(ConSkHFile));
   KillSonProcesses();
   exit(1);
  }
  ofstream hf1(ConHFile.toStdString().c_str(),ifstream::out);
  char ch;
  while (hfk >> ch)
   hf1 << ch;
  hfk.close();
  hf1.close();
  maProgOut->append(tr("The file %1 did not exist. It has been created.").arg(ConHFile));
 }

 if (!FileAccessible(ConCppFile))
 {
  QString ConSkCFile=INST_DIR+CONTEXT_SOURCE;
  ConSkCFile.replace(DEFAULT_CPP_EXTENSION,"_skeleton"+DEFAULT_CPP_EXTENSION);
  if (!FileAccessible(ConSkCFile))
  {
   QMessageBox::critical(this,APPNAME,tr("The skeleton file %1 does not exists. Please, install the emintweb program correctly").arg(ConSkCFile));
   KillSonProcesses();
   exit(1);
  }
  QString com=COPY_COMMAND;
  QStringList args;
  args << ConSkCFile << ConCppFile;

  QString BannerError;
  bool ret=RunCommand(com,args,"",BannerError,-1);
  if (!ret)
  {
   QMessageBox::warning(this,APPNAME,tr("The skeleton file %1 has NOT been copied to %2. Please, check for directory existance and permissions.").arg(ConSkCFile).arg(ConCppFile));
   return;
  }
  else
   maProgOut->append(tr("The file %1 did not exist. It has been created.").arg(ConCppFile));
 }
}

void AppWindow::EditTransitionFile()
{
 QString nametrans=(((QAction *)sender())->data()).toString();
 cout << "Editing transition file for transition " << nametrans.toStdString() << endl;

 QString fname=P.GetPDir()+P.GetCodeDir()+TRCODE_DIR+P.GetTransitionCodeFile(nametrans);

 if (DEB)
  cout << "Editing transition code " << fname.toStdString() << endl;

 if (!FileAccessible(fname))
 {
  QMessageBox::warning(this, APPNAME, tr("The file %1 does not exists.\nYou will have to generate it first.").arg(fname),QMessageBox::Ok);
  return;
 }

 QString command=CPP_EDITOR;
 QStringList args;
 #ifdef CPP_EDITOR_ARG1
 args << CPP_EDITOR_ARG1
 #endif
 args << fname;

 QString BannerRes;
 bool ret=RunCommand(command,args,"",BannerRes,0);
 if (!ret)
  QMessageBox::warning(this,APPNAME,tr("Could not call C++ editor. See messages area."),QMessageBox::Ok);
}

void AppWindow::GenTransitionCode()
{
 QString nametrans=(((QAction *)sender())->data()).toString();
 cout << "Generating transition code for transition " << nametrans.toStdString() << endl;

 P.GenCodeForTransition(nametrans);
}


void AppWindow::ComTransitionCode()
{
 QString nametrans=(((QAction *)sender())->data()).toString();
 if (DEB)
  cout << "Compiling code for transition " <<  nametrans.toStdString() << endl;

 CompCodeForTransition(nametrans);
}


// Just to elliminate the state from the secondary menu
void AppWindow::MenuStateDone()
{
 QString namestate=(((QAction *)sender())->data()).toString();
 QList<QAction *> l = mbStates->actions();
 QList<QAction *>::iterator itfound;
 bool found=false;
 for (QList<QAction *>::iterator it=l.begin();it!=l.end() && !found;++it)
  if ((*it)->text()==namestate)
  {
   itfound=it;
   found=true;
  }

 if (found)
  mbStates->removeAction(*itfound);
}

// Just to elliminate the transition from the secondary menu
void AppWindow::MenuTransitionDone()
{
 QString nametrans=(((QAction *)sender())->data()).toString();
 QList<QAction *> l = mbTransitions->actions();
 QList<QAction *>::iterator itfound;
 bool found=false;
 for (QList<QAction *>::iterator it=l.begin();it!=l.end() && !found;++it)
  if ((*it)->text()==nametrans)
  {
   itfound=it;
   found=true;
  }

 if (found)
  mbTransitions->removeAction(*itfound);
}

void AppWindow::SetPort()
{
 AskPort=new QDialog(this);
 AskPort->setModal(true);

 QPushButton *butrun = new QPushButton("Set",AskPort);
 int frameStyle = QFrame::Raised | QFrame::StyledPanel;
 QLabel *LPort = new QLabel(tr("Port to open server: "));
 LPort->setFrameStyle(frameStyle);
 EPort = new QLineEdit();
 EPort->setText(port);

 QGridLayout *layout = new QGridLayout;
 layout->addWidget(LPort,0,0);
 layout->addWidget(EPort,0,1);
 layout->addWidget(butrun,1,0);

 connect(butrun,SIGNAL(clicked()),this,SLOT(GetPort()));

 AskPort->setLayout(layout);
 AskPort->show();
 AskPort->exec();
}

void AppWindow::GetPort()
{
 QString portstring=EPort->text();

 int i=0;
 while (i<portstring.size() && (portstring[i]>='0') && (portstring[i]<='9'))
  i++;
 if (i<portstring.size())
 {
  QMessageBox::warning(this, APPNAME, tr("Error: %1 is not a number").arg(portstring),QMessageBox::Ok);
  return;
 }
 unsigned portnum=0;
 i=0;
 char ch;
 while (i<portstring.size())
 {
  ch=portstring.toStdString()[i];
  portnum=(10*portnum+int(ch-'0'));
  i++;
 }
 if (DEB)
  cout << "The number for port is " << portnum << endl;

 if ((portnum<1025) || (portnum>32768))
 {
  QMessageBox::warning(this, APPNAME, tr("Error: port must be between 1025 and 32768"),QMessageBox::Ok);
  return;
 }
 port=portstring;
 maProgOut->append(tr("Server port set to %1").arg(port));
 AskPort->done(0);
}

void AppWindow::KillSonProcesses()
{
 // The generated program, if running, will not be killed.
 for (int i=0;i<MAX_SON_PROCESSES-1;i++)
  if (processes[i]!=NULL)
  {
   if (processes[i]->state()==QProcess::Running)
   {
    if (DEB)
     cout << "Killing process " << processes[i]->processId() << endl;
    processes[i]->close();
   }
   else
    if (DEB)
     cout << "Process " << processes[i]->processId() << " was not running.\n";
  }
}

bool AppWindow::RunCommand(QString cmd,QStringList args,QString dir,QString &BannerContent,int waitms,string message,string message_arg)
{
 QProcess *process;
 int posp;

 if (waitms!=0)
 {
  process = new QProcess;
  posp=-1;
 }
 else
 {
  int i=0;
  while ( (i<MAX_SON_PROCESSES-1) && (processes[i]!=NULL) && (processes[i]->state()!=QProcess::NotRunning) )
   i++;
  if (i>=MAX_SON_PROCESSES-1)
  {
   BannerContent=tr("Error trying to open son process to run command %1. No more available processes in the pool.").arg(cmd);
   maProgOut->append(tr("Command '%1' could not start. The pool of processes is exhausted.").arg(cmd));
   return false;
  }
  posp=i;
  if (DEB)
   cout << "Command " << cmd.toStdString() << " in position " << i << " of the pool.\n";

  processes[posp]=new QProcess;
  process=processes[posp];
 }

 if (!dir.isEmpty())
  process->setWorkingDirectory(dir);

 process->start(cmd,args);
 
 QString cmdplusargs=cmd+" ";
 for (int i=0;i<args.size()-1;i++)
  cmdplusargs += args[i] + " ";
 cmdplusargs += args[args.size()-1]; 
 if (message=="")
  maProgOut->append(tr("Running command '%1'").arg(cmdplusargs));
 
 if (!process->waitForStarted())
 {
  BannerContent=tr("Process %1 could not start.").arg(cmdplusargs);
  if (waitms>0)
  {
   delete process;
   if (posp>=0)
    processes[posp]=NULL;
  }
  return false;
 }

 if (waitms!=0)
  process->waitForFinished(waitms);

 QByteArray sout=process->readAllStandardOutput();
 QByteArray serr=process->readAllStandardError();

 QString ro(sout);
 QString re(serr);

 if ( process->exitCode()==0 )
 {
  if (message=="")
   maProgOut->append(tr("Command '%1' returns 0. Good.").arg(cmdplusargs));
  else
   maProgOut->insertPlainText(tr(message.c_str()).arg(message_arg.c_str()));
  if (waitms>0)
  {
   delete process;
   if (posp>=0)
    processes[posp]=NULL;
  }
  return true;
 }

 if (dir.isEmpty())
  maProgOut->append(tr("Command '%1' ended with error.\n").arg(cmdplusargs));
 else
  maProgOut->append(tr("Command '%1' executed from directory '%2' ended with error.").arg(cmdplusargs).arg(dir));
  
 maProgOut->append(tr("Standard output was:"));
 maProgOut->append(QString::fromUtf8(ro.toStdString().c_str()));
 maProgOut->append(tr("Standard error was:"));
 maProgOut->append(QString::fromUtf8(re.toStdString().c_str()));

 if (dir.isEmpty())
  BannerContent=tr("Process %1 ended with error\nSee messages area.").arg(cmdplusargs);
 else
  BannerContent=tr("Process %1 executed from directory %2 ended with error.\nSeeMessages area.").arg(cmdplusargs).arg(dir);

 delete process;
 if (posp>=0)
  processes[posp]=NULL;

 return false;
}
