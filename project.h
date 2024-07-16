#ifndef PROJECT_H
#define PROJECT_H

#include <QList>
#include <QMap>
#include <QTextStream>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QDomDocument>
#include <QFile>
#include <QProcess>
#ifdef GUI
#include <QObject>
#include <QTextEdit>
#include <QColor>
#include <cstdlib>
#endif

#include "global.h"
#include "state.h"
#include "transition.h"
#include "messagehandler.h"
#include "hdoc.h"
#include "drawsizes.h"

#define HTML5HEADER "<!DOCTYPE html>\n"

#ifndef GUI
#define tr(x) x
#endif

#ifdef GUI
class Project : public QObject
{
 Q_OBJECT
#else
class Project
{
#endif
 public:
  Project();
  Project(QString pf,bool CheckAfterLoad=true);

  void Reset(QString Name,bool mode,QString pemfile,QString pemkey,QString pemkeysize,QString dhfile,QString File,QString Dir,QString HTDir,QString CDir,QString Comm,bool SDeb,QString Compiler,QString CompFlags,bool keep_st_tr);

#ifdef GUI
  void SetMessageAreaPointer( QTextEdit *m)  { ma=m; };
#endif

  bool ReadFromFile(QString pf);
  bool Save();
  bool SaveAs(QString pf);

  bool IsEmpty()                { return CurrentInitstate==FORBIDDEN_STATE_NAME; };
  void NotEmpty()               { CurrentInitstate=""; };
  QString GetInitState()        { return CurrentInitstate; };

  bool GetHTTPSMode()           { return WithHTTPS; };
  void SetHTTPSMode(bool mode)  { WithHTTPS=mode; };
  QString GetName()             { return PName; };
  void SetName(QString Name)    { PName=Name; };
  QString GetPFile()            { return PFile; };
  void SetPFile(QString File)   { PFile=File; LocalPFile=PFile; LocalPFile.replace(".emw","_localconf.emw"); };
  QString GetLocalPFile()       { return LocalPFile; };
  QString GetPDir()             { return PDir; };
  void SetPDir(QString Dir)     { PDir=Dir; };
  QString GetHTMLDir()          { return HTMLDir; };
  void SetHTMLDir(QString HDir) { HTMLDir=HDir; };
  QString GetCodeDir()          { return CodeDir; };
  void SetCodeDir(QString CDir) { CodeDir=CDir; };
  QString GetComments()         { return Comments; };
  void SetComments(QString C)   { Comments=C; };

  QString GetCppCompiler()      { return CppCompiler; };
  void SetCppCompiler(QString c) { CppCompiler=c; };

  QStringList GetCompFlags()     { return CompilerFlags; };
  QString GetCompFlagsAsString() { return GetAdditionalElementsAsString(CompilerFlags,COMP_FLAGS_SEPS); }
  
  void SetCompFlags(QString Fl);

  bool GetServerDebFlag()       { return ServerDebFlag; };
  void SetServerDebFlag(bool flag) { ServerDebFlag=flag; };

  bool NeedsToBeSaved()         { return ToBeSaved; };
  void Modified() { ToBeSaved=true; };
  void Saved()    { ToBeSaved=false; };

  unsigned GetNumStates()  { return States.size(); };
  
  void Extension(QPoint &pmax);

  QList<QString> GetStateNameList()                          { return States.keys(); };
  QList<QString> GetTransitionNameList()                     { return Transitions.keys(); };
  QString GetStateHTMLFile(QString name)                     { return (States.contains(name)) ? States[name].GetHTMLFile() : "Inexistent_file.html"; };
  void SetStateHTMLFile(QString name,QString newh);
  QPoint GetStatePosition(QString name)                      { return (States.contains(name)) ? States[name].GetPos() : QPoint(0,0); };
  void SetStatePosition(QString name,QPoint newpos);
  QString GetStateComments(QString name)                     { return (States.contains(name)) ? States[name].GetComments() : ""; };
  void SetStateComments(QString name,QString newcom);

  QPoint GetTransitionPosition(QString name)                 { return (Transitions.contains(name)) ? Transitions[name].GetPos() : QPoint(0,0); };
  void SetTransitionPosition(QString name,QPoint newpos);
  QString GetTransitionComments(QString name)                { return (Transitions.contains(name)) ? Transitions[name].GetComments() : ""; };
  void SetTransitionComments(QString name,QString newcom);
  //QString GetTransitionInitialState(QString name)            { return (Transitions.contains(name)) ? Transitions[name].GetOriginState() : "Inexistent_state"; };
  QPoint GetTransitionInitialPosition(QString name);
  QVector<QPoint> GetTransitionFinalPositions(QString name);
  QString GetTransitionInitialState(QString name)            { return (Transitions.contains(name)) ? Transitions[name].GetOriginState() : ""; };
  QList<QString> GetTransitionFinalStateList(QString name);
  QString GetTransitionFallStateName(QString name)           { return (Transitions.contains(name)) ? Transitions[name].GetFallState() : ""; };

  QString GetHTMLFile(QString name)                          { return (States.contains(name)) ? States[name].GetHTMLFile() : "Inexistent_state.html"; };
  QString GetStateCodeFile(QString name)                     { return (States.contains(name)) ? States[name].GetCodeFile() : "Inexistent_state"+DEFAULT_CPP_EXTENSION; };
  QString GetStateHeaderFile(QString name)                   { return (States.contains(name)) ? States[name].GetHeaderFile() : "Inexistent_state"+DEFAULT_HEADER_EXTENSION; };
  QString GetTransitionCodeFile(QString name)                { return (Transitions.contains(name)) ? Transitions[name].GetNameCodeFile() : "Inexistent_transition"+DEFAULT_CPP_EXTENSION; };

  StateType GetStateType(QString name)                       { return (States.contains(name)) ? States[name].GetStype() : NO_TYPE; };
  void SetStateType(QString name,StateType t)                { if (States.contains(name)) States[name].SetStype(t); };

  bool ThereIsOneTransDepartingFrom(QString name);
  bool ThereIsOneTransArrivingTo(QString name);

  QString GetPEMFile()                                       { return PEMFile; };
  QString GetPEMKey()                                        { return PEMKey; };
  int GetPEMKeySize()                                        { return PEMKeySize; };
  QString GetDHFile()                                        { return DHFile; };
  
  QStringList GetAdditionalSources(QString prepend)          { return GetAdditionalElements(AdditionalSources,prepend,ADDIT_SOURCES_SEP); };
  QString GetAdditionalSourcesAsString(QString prepend)	     { return GetAdditionalElementsAsString(AdditionalSources,prepend,ADDIT_SOURCES_SEPS); };
  void SetAdditionalSources(QList<QString> &AddS)            { AdditionalSources.clear(); for (int i=0;i<AddS.size();i++) AdditionalSources.push_back(AddS[i]); };
  
  QStringList GetAdditionalIncludeDirs()                     { return GetAdditionalElements(AdditionalIncludeDirs,ADDIT_IDIRS_SEP); };
  QString GetAdditionalIncludeDirsAsString()                 { return GetAdditionalElementsAsString(AdditionalIncludeDirs,ADDIT_IDIRS_SEPS); };
  void SetAdditionalIncludeDirs(QList<QString> &AddI)        { AdditionalIncludeDirs.clear(); AdditionalIncludeDirs=AddI; };
  
  QStringList GetAdditionalLibDirs()                         { return GetAdditionalElements(AdditionalLibDirs,ADDIT_LDIRS_SEP); };
  QString GetAdditionalLibDirsAsString()                     { return GetAdditionalElementsAsString(AdditionalLibDirs,ADDIT_LDIRS_SEPS); };
  void SetAdditionalLibDirs(QList<QString> &AddLd)           { AdditionalLibDirs.clear(); AdditionalLibDirs=AddLd; };
  
  QStringList GetAdditionalLibraries()                       { return GetAdditionalElements(AdditionalLibraries,ADDIT_LIBS_SEP); };
  QString GetAdditionalLibrariesAsString()                   { return GetAdditionalElementsAsString(AdditionalLibraries,ADDIT_LIBS_SEPS); };
  void SetAdditionalLibraries(QList<QString> &AddLib)        { AdditionalLibraries.clear(); AdditionalLibraries=AddLib; };

  bool StateExists(QString namestate)                        { return States.contains(namestate); };
  bool AddState(State st);
  bool AlterState(QString OldName,State st);
  bool DelState(QString namestate);

  bool TransitionExists(QString nametrans)                   { return Transitions.contains(nametrans); };
  bool AddTransition(Transition tr);
  bool AlterTransition(QString OldName,Transition tr);
  bool DelTransition(QString nametransition);

  void Clean();
  void CreateSubdirectories();

  bool CheckProject();

  void GenContext(bool Overwrite);

#ifdef GUI
 public slots:
#endif
  void GenMain();
  void GenMake();
  void GenCodeForTransition(QString NameTrans);
  void GenCodeForState(QString NameState,bool GenTest);

#ifdef GUI
 signals:
  void E0(bool critical,QString e0);
  void E1(bool critical,QString e0,QString e1);
  void E2(bool critical,QString e0,QString e1,QString e2);
  void E3(bool critical,QString e0,QString e1,QString e2,QString e3);
  void E4(bool critical,QString e0,QString e1,QString e2,QString e3,QString e4);
  void E5(bool critical,QString e0,QString e1,QString e2,QString e3,QString e4,QString e5);
#endif

 private:
  QString PName;
  bool WithHTTPS;
  QString PFile;
  QString LocalPFile;
  QString PDir;
  QString HTMLDir;
  QString CodeDir;
  QString Comments;
  QString CurrentInitstate;

  bool ToBeSaved;

  bool ServerDebFlag;
  QString ServerPort;
  QString CppCompiler;
  QString PEMFile;
  QString PEMKey;
  int PEMKeySize;
  QString DHFile;
  QStringList CompilerFlags;
  QStringList AdditionalSources;
  QStringList AdditionalIncludeDirs;
  QStringList AdditionalLibDirs;
  QStringList AdditionalLibraries;
  
  QMap<QString,State> States;
  QMap<QString,Transition> Transitions;

#ifdef GUI
  QTextEdit *ma;
#endif

  QStringList GetAdditionalElements(QStringList Add,QString sep);
  QString GetAdditionalElementsAsString(QStringList Add,QString sep);
  QStringList GetAdditionalElements(QStringList Add,QString prepend,QString sep);
  QString GetAdditionalElementsAsString(QStringList Add,QString prepend,QString sep);
  
  void ExitIfNotAccessible(QString fname);
  void InsertFile(ofstream &f,QString fname);
  QString FileAsQString(QString fname);
  void InsertFileWithSubsts(ofstream &f,QString fname,QStringList to_search,QStringList to_replace);
  bool HasContent(QString r);
  QString NameWithoutPath(QString s);

  void SearchAndModifySName(QString PrevName,QString NewName,StateType NewType);
  void CreateDir(QString dir);
  void CopyFile(QString orig,QString dest);
  bool OpenToRead(QString fname,ifstream &f);
  bool OpenToWrite(QString fname,ofstream &f);
  void ErrWarn(bool critical,QVector<QString> e);

  bool ReadFromTextStream(QString globalcf,QString localcf,QTextStream &inxsd,QTextStream &inf,QTextStream &localinxsd,QTextStream &localinf);
  void SaveProToTextStream(QTextStream &out);
  void SaveLocalConfToTextStream(QTextStream &out);
  
  void GenCommonHeader(QString MainHeader,QString inith);
  void GenMainHeaderCall(QString MainHeaderCall);
  void GenSingleMainAndThreadHeaders(QString MainHeader,QString MainHeaderCall,QString inith);
  void GenSingleMainAndThreadBodies(QString MainSource,QString MainSourceCall,QString inith);
    
  void GenServer(QString MainHeaderServer,QString MainSourceServer);
};

#endif // PROJECT_H
