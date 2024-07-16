#include "project.h"

extern bool DEB;

Project::Project()
{
 Clean();
}

Project::Project(QString pf,bool CheckAfterLoad)
{
 Clean();

 if (!pf.isEmpty())
 {
  bool ret=ReadFromFile(pf);
  if (!ret)
   Clean();

  if (PFile.contains(PLATFORM_DIRSLASH))
  {
   int l=PFile.lastIndexOf(PLATFORM_DIRSLASH[0]);
   QString NPFile=PFile.mid(l+1);
   SetPFile(NPFile);
  }

  if (DEB)
  {
   cout << "Project " << PName.toStdString() << (ret ? " correctly " : " NOT correctly ") << "loaded from file " << pf.toStdString() << endl;
   cout << "The files to which this project will be saved are now " << (PDir+PFile).toStdString() << " and " << (PDir+LocalPFile).toStdString() << endl;
  }

  if (CheckAfterLoad)
  {
   bool ret=CheckProject();
   if (!ret)
   {
    Clean();
    QVector<QString> e;
    e.push_back(tr("Project has been checked and it HAS errors. Please, see message area or console for more information."));
    ErrWarn(false,e);
   }
  }

  if (DEB && !CheckAfterLoad)
   cout << "Project has not been checked for semantic correctness, at user request. It might contain errors.\n";

 }
 else
  Clean();

 ToBeSaved=false;
}

void Project::Clean()
{
 Reset(DEFAULT_PROJECT_NAME,DEFAULT_HTTPS_MODE,DEFAULT_PEMFILE,DEFAULT_PEMKEY,DEFAULT_PEMKEYSIZE,DEFAULT_DHFILE,DEFAULT_PFILE_NAME,
       HERE,DEFAULT_HTML_DIR,DEFAULT_CODE_DIR,"No comments yet",true,DEFAULT_CPP_COMPILER,DEFAULT_CPP_FLAGS,true);
 CurrentInitstate = FORBIDDEN_STATE_NAME;
}

void Project::Reset(QString Name,bool mode,QString pemfile,QString pemkey,QString pemkeysize,QString dhfile,QString File,QString Dir,QString HTDir,QString CDir,QString Comm,bool SDeb,QString Compiler,QString CompFlags,bool kill_st_tr)
{
 PName=Name;
 WithHTTPS=mode;
 PEMFile=pemfile;
 PEMKey=pemkey; 
 PEMKeySize=atoi(pemkeysize.toStdString().c_str());
 DHFile=dhfile;
 if (File != DEFAULT_PROJECT_NAME)
  PFile=File;
 PDir=Dir;
 HTMLDir=HTDir;
 if (!HTMLDir.endsWith(PLATFORM_DIRSLASH))
  HTMLDir+=PLATFORM_DIRSLASH;
 CodeDir=CDir;
 if (!CodeDir.endsWith(PLATFORM_DIRSLASH))
  CodeDir+=PLATFORM_DIRSLASH;
 Comments=Comm;
 ServerDebFlag=SDeb;
 CurrentInitstate="";
 AdditionalSources.clear();
 AdditionalSources.push_back(CONTEXT_SOURCE);
 AdditionalIncludeDirs.clear();
 AdditionalIncludeDirs.push_back(DEFAULT_POCO_INCLUDE_DIR);
 AdditionalIncludeDirs.push_back(DEFAULT_LOCAL_INCLUDE_DIR);
 AdditionalLibDirs.clear();
 AdditionalLibDirs.push_back(DEFAULT_LOCAL_LIB_DIR);
 
 AdditionalLibraries.clear();
 AdditionalLibraries.push_back(DEFAULT_HT5CPP_LIB);
 AdditionalLibraries.push_back(DEFAULT_POCOLIB1);
 AdditionalLibraries.push_back(DEFAULT_POCOLIB2);
 AdditionalLibraries.push_back(DEFAULT_POCOLIB3);
 AdditionalLibraries.push_back(DEFAULT_POCOLIB4);
 AdditionalLibraries.push_back(DEFAULT_POCOLIB5);
 AdditionalLibraries.push_back(DEFAULT_PTHREAD);

 ToBeSaved=false;
 CppCompiler=Compiler;
 QStringList dummy=CompFlags.split(QRegExp("\\s+"));
 for (int i=0;i<dummy.size();i++)
  if (dummy[i] != QString(""))
   CompilerFlags.push_back(dummy[i]);
 CompilerFlags.removeDuplicates();  
   
 ServerPort=DEFAULT_SERVER_PORT;

#ifdef GUI
 ma=NULL;
#endif

 if (kill_st_tr)
 {
  States.clear();
  Transitions.clear(); 
 }
}

void Project::ErrWarn(bool critical,QVector<QString> e)
{
#ifdef GUI
 if (ma==NULL)
 {
#endif
  cerr << (critical ? "Critical error:\n" : "Warning:\n");
  QString efinal=e[0];
  QString erep="%_";
  for (int i=1;i<e.size();i++)
  {
   erep[1]=char('0'+i);
   efinal.replace(erep,e[i]);
  }
  cerr << efinal.toStdString() << endl;
  if (critical)
   exit(1);
#ifdef GUI
 }
 else
 {
  switch (e.size())
  {
   case 1: emit E0(critical,e[0]); break;
   case 2: emit E1(critical,e[0],e[1]); break;
   case 3: emit E2(critical,e[0],e[1],e[2]); break;
   case 4: emit E3(critical,e[0],e[1],e[2],e[3]); break;
   case 5: emit E4(critical,e[0],e[1],e[2],e[3],e[4]); break;
   case 6: emit E5(critical,e[0],e[1],e[2],e[3],e[4],e[5]); break;
   default: cerr << "Error: function ErrWarn can only be used with a QVector of 1 to 6 values.\n"; exit(1); break;
  }
 }
#endif
}

bool Project::OpenToRead(QString fname,ifstream &g)
{
 g.open(fname.toStdString().c_str());
 if (!g.is_open())
 {
  QVector<QString> e;
  e.push_back(tr("Cannot open file %1 for reading. Check path and permissions."));
  e.push_back(fname);
  ErrWarn(false,e);
  return false;
 }
 return true;
}

bool Project::OpenToWrite(QString fname,ofstream &g)
{
 g.open(fname.toStdString().c_str());
 if (!g.is_open())
 {
  QVector<QString> e;
  e.push_back(tr("Cannot open file %1 to write. Please, check directory existance and permissions."));
  e.push_back(fname);
  ErrWarn(false,e);
  return false;
 }
 return true;
}


void Project::SetCompFlags(QString fl)
{
 QStringList dummy=fl.split(QRegExp("\\s+"));
 for (int i=0;i<dummy.size();i++)
  if (dummy[i] != QString(""))
   CompilerFlags.push_back(dummy[i]);
 CompilerFlags.removeDuplicates();  
}

bool Project::ReadFromFile(QString pf)
{
 PFile=pf;

 QFile xsdfile(XSD_FILE_NAME);
 if (!xsdfile.open(QFile::ReadOnly | QFile::Text))
 {
  QVector<QString> e;
  e.push_back(tr("Cannot read file %1:\n%2."));
  e.push_back(XSD_FILE_NAME);
  e.push_back(xsdfile.errorString());
  ErrWarn(true,e);
 }
 QTextStream inxsd(&xsdfile);

 QFile xsdlocalfile(XSD_LOCALFILE_NAME);
 if (!xsdlocalfile.open(QFile::ReadOnly | QFile::Text))
 {
  QVector<QString> e;
  e.push_back(tr("Cannot read file %1:\n%2."));
  e.push_back(XSD_LOCALFILE_NAME);
  e.push_back(xsdfile.errorString());
  ErrWarn(true,e);
 }
 QTextStream localxsd(&xsdlocalfile);

 QFile file(PFile);
 if (!file.open(QFile::ReadOnly | QFile::Text))
 {
  QVector<QString> e;
  e.push_back(tr("Cannot read file %1:\n%2."));
  e.push_back(PFile);
  e.push_back(file.errorString());
  ErrWarn(true,e);
 }
 QTextStream in(&file);

 LocalPFile=PFile;
 LocalPFile.replace(".emw","_localconf.emw");

 QFile localfile(LocalPFile);
 if (!localfile.open(QFile::ReadOnly | QFile::Text))
 {
  QVector<QString> e;
  e.push_back(tr("Cannot read file %1:\n%2."));
  e.push_back(LocalPFile);
  e.push_back(file.errorString());
  ErrWarn(true,e);
 }
 QTextStream localin(&localfile);

 bool ret=ReadFromTextStream(PFile,LocalPFile,inxsd,in,localxsd,localin);

 file.close();
 localfile.close();
 xsdfile.close();
 xsdlocalfile.close();

 if (DEB)
  cout << "File " << PFile.toStdString() << (ret ? " loaded.\n" : " could NOT be loaded.\n");

 return ret;
}

bool Project::ReadFromTextStream(QString globalcf,QString localcf,QTextStream &inxsd,QTextStream &inf,QTextStream &localinxsd,QTextStream &localinf)
{
 {
  // First, the local conf. schema and the local conf. file are converted to byte arrays.
  const QByteArray valdata = localinxsd.readAll().toUtf8();
  const QByteArray docdata = localinf.readAll().toUtf8();

  MessageHandler msh;

  QXmlSchema sch;
  sch.setMessageHandler(&msh);

  // The schema loads the local conf. schema
  sch.load(valdata);

  bool errorOccurred = false;
  // This is previous: the local conf. schema could be a malformed XSD file...
  if (!sch.isValid())
   errorOccurred = true;
  else
  {
   // Once we know it is not, we check our configuration file for semantic correctness with respect to the schema
   QXmlSchemaValidator validator(sch);
   if (!validator.validate(docdata))
    errorOccurred = true;
  }

  if (errorOccurred)
  {
   QVector<QString> e;
   e.push_back(tr("Error reading project %1 file when validating with schema file %2:\nAt line %3, column %4\n%5"));
   e.push_back(localcf);
   e.push_back(XSD_LOCALFILE_NAME);
   {
    ostringstream o;
    o << msh.line();
    e.push_back(o.str().c_str());
   }
   {
    ostringstream o;
    o << msh.column();
    e.push_back(o.str().c_str());
   }
   e.push_back(msh.statusMessage());

   ErrWarn(true,e);
   return false;
  }

  QDomDocument configfile;
  QString errors;
  int l,c;
  if (!configfile.setContent(docdata,&errors,&l,&c))
  {
   // This will never happen, no syntactic errors can survive after validation, every valid document is also well-formed.
   QVector<QString> e;
   e.push_back(tr("Error reading project file %1 when validating with schema file %2:\nAt line %3, column %4\n%5"));
   e.push_back(localcf);
   e.push_back(XSD_LOCALFILE_NAME);
   {
    ostringstream o;
    o << l;
    e.push_back(o.str().c_str());
   }
   {
    ostringstream o;
    o << c;
    e.push_back(o.str().c_str());
   }
   e.push_back(errors);
   ErrWarn(true,e);

   return false;
  }

  QDomElement root = configfile.firstChildElement();
  for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling())
  {
   QDomElement el = node.toElement();
   if (el.tagName()=="HTTPSMode")
    SetHTTPSMode(el.text()=="true");
   if (el.tagName()=="ProjectDir")
    PDir=el.text();
    // OJO: comprobar aquí que sea un path absoluto??
   if (el.tagName()=="HTMLSubDir")
    HTMLDir=el.text();
   if (!HTMLDir.endsWith(PLATFORM_DIRSLASH))
    HTMLDir+=PLATFORM_DIRSLASH;
   if (el.tagName()=="CodeSubDir")
    CodeDir=el.text();
   if (el.tagName()=="ServerDebugFlag")
    ServerDebFlag=(el.text()=="true");
   if (!CodeDir.endsWith(PLATFORM_DIRSLASH))
    CodeDir+=PLATFORM_DIRSLASH;
   if (el.tagName()=="CPPCompiler")
    CppCompiler=el.text();
   if (el.tagName()=="CompilerFlags")
   {
    QStringList dummy=el.text().split(QRegExp("\\s+"));
    for (int i=0;i<dummy.size();i++)
     if (dummy[i] != QString(""))
      CompilerFlags.push_back(dummy[i]);
    CompilerFlags.removeDuplicates();  
   }
   if (el.tagName()=="AdditionalSources")
   {
    for (QDomNode node2 = node.firstChild(); !node2.isNull(); node2 = node2.nextSibling())
    {
     QDomElement el = node2.toElement();
     if (el.tagName()=="Source")
      AdditionalSources.push_back(el.text());
    }
   }
   if (el.tagName()=="AdditionalIncludeDirs")
   {
    for (QDomNode node2 = node.firstChild(); !node2.isNull(); node2 = node2.nextSibling())
    {
     QDomElement el = node2.toElement();
     if (el.tagName()=="IncludeDir")
      AdditionalIncludeDirs.push_back(el.text());
    }
   }
   if (el.tagName()=="AdditionalLibDirs")
   {
    for (QDomNode node2 = node.firstChild(); !node2.isNull(); node2 = node2.nextSibling())
    {
     QDomElement el = node2.toElement();
     if (el.tagName()=="LibDir")
      AdditionalLibDirs.push_back(el.text());
    }
   }
   if (el.tagName()=="AdditionalLibraries")
   {
    for (QDomNode node2 = node.firstChild(); !node2.isNull(); node2 = node2.nextSibling())
    {
     QDomElement el = node2.toElement();
     if (el.tagName()=="Library")
      AdditionalLibraries.push_back(el.text());
    }
   }
   if (el.tagName()=="SSLKeyFiles")
   {
    for (QDomNode node2 = node.firstChild(); !node2.isNull(); node2 = node2.nextSibling())
    {
     QDomElement el = node2.toElement();
     if (el.tagName()=="PEMFile")
      PEMFile=el.text();
     if (el.tagName()=="PEMKey")
      PEMKey=el.text();
     if (el.tagName()=="RSAKeySize")
      PEMKeySize=atoi(el.text().toStdString().c_str()); 
     if (el.tagName()=="DHFile")
      DHFile=el.text();
    }
   }
  }
  
  AdditionalSources.removeDuplicates();
  AdditionalIncludeDirs.removeDuplicates();
  AdditionalLibDirs.removeDuplicates();
  AdditionalLibraries.removeDuplicates();
  
  if (DEB)
   cout << "Project local file conforming to specifications. Good.\n";

  CreateSubdirectories();
 }

 {
  const QByteArray valdata = inxsd.readAll().toUtf8();
  const QByteArray docdata = inf.readAll().toUtf8();

  MessageHandler msh;

  QXmlSchema sch;
  sch.setMessageHandler(&msh);

  sch.load(valdata);

  bool errorOccurred = false;
  if (!sch.isValid())
   errorOccurred = true;
  else
  {
   QXmlSchemaValidator validator(sch);
   if (!validator.validate(docdata))
    errorOccurred = true;
  }

  if (errorOccurred)
  {
   QVector<QString> e;
   e.push_back(tr("Error reading project file %1 when validating with schema file %2:\nAt line %3, column %4\n%5"));
   e.push_back(globalcf);
   e.push_back(XSD_FILE_NAME);
   {
    ostringstream o;
    o << msh.line();
    e.push_back(o.str().c_str());
   }
   {
    ostringstream o;
    o << msh.column();
    e.push_back(o.str().c_str());
   }
   e.push_back(msh.statusMessage());

   ErrWarn(true,e);
   return false;
  }

  QDomDocument configfile;
  QString errors;
  int l,c;
  if (!configfile.setContent(docdata,&errors,&l,&c))
  {
   // This will never happen, no syntactic errors can survive after validation, every valid document is also well-formed.
   QVector<QString> e;
   e.push_back(tr("Error reading project file %1 when validating with schema file %2:\nAt line %3, column %4\n%5"));
   e.push_back(globalcf);
   e.push_back(XSD_FILE_NAME);
   {
    ostringstream o;
    o << l;
    e.push_back(o.str().c_str());
   }
   {
    ostringstream o;
    o << c;
    e.push_back(o.str().c_str());
   }
   e.push_back(errors);
   ErrWarn(true,e);
   return false;
  }

  QDomElement root = configfile.firstChildElement();
  bool InitialStateFound=false;
  for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling())
  {
   QDomElement el = node.toElement();
   if (el.tagName()=="ProjectName")
    PName=el.text();

   if (el.tagName()=="Comments")
    Comments=el.text();

   if (el.tagName()=="States")
    for (QDomNode secnode = node.firstChild(); !secnode.isNull(); secnode = secnode.nextSibling())
    {
     State newstate(secnode);
     States[newstate.GetName()]=newstate;
     if (newstate.GetStype()==Initial)
     {
      if (!InitialStateFound)
      {
       CurrentInitstate=newstate.GetName();
       InitialStateFound=true;
      }
      else
      {
       QVector<QString> e;
       e.push_back("More than one InitialState when reading project file %1");
       e.push_back(globalcf);
       ErrWarn(true,e);
       return false;
      }
     }
    }
   if (el.tagName()=="Transitions")
    for (QDomNode secnode = node.firstChild(); !secnode.isNull(); secnode = secnode.nextSibling())
    {
     Transition newtrans(secnode);
     Transitions[newtrans.GetName()]=newtrans;
    }
  }
  if (!InitialStateFound)
  {
   QVector<QString> e;
   e.push_back("No state marked as InitialState when reading project file %1. There should be exactly one initial state.");
   e.push_back(globalcf);
   ErrWarn(true,e);
   return false;
  }
  QList<QString> ltrans=Transitions.keys();
  QList<QString> lstates=States.keys();

  for (int i=0;i<ltrans.size();i++)
  {
   QList<StateType> fstypes;
   QList<QString> r=Transitions[ltrans[i]].GetFinalStates();
   for (int j=0;j<r.size();j++)
   {
    int k=0;
    while (k<lstates.size() && States[lstates[k]].GetName()!=r[j])
     k++;
    if (k<lstates.size())
    {
     StateType tp=States[lstates[k]].GetStype();
     fstypes.push_back(tp);
    }
    else
    {
     cerr << "Something wrong: state with name " << r[j].toStdString() << " not found. Setting as intermediate (but this is wrong).\n";
     fstypes.push_back(Intermediate);
    }
   }
   Transitions[ltrans[i]].SetFinalStTypes(fstypes);
  }

  if (DEB)
   cout << "Project file conforming to specifications. Good.\n";

 }

 return true;
}

#ifdef GUI
#define OUT(s) if (ma==NULL) cout << s.toStdString(); else ma->insertPlainText(s)
#else
#define OUT(s) cout << s.toStdString();
#endif

#define OK     { s="OK\n"; OUT(s); }

bool Project::CheckProject()
{
 // These errors messages will not be translated. They are not part of the program interface and it would be too cumbersome.

 QMap<QString,State>::iterator itS;
 QMap<QString,Transition>::iterator itR;

 QString s="Checking that there is exactly one initial state... ";
 OUT(s);
 int n=0;
 QString badState;
 itS=States.begin();
 while (itS!=States.end())
 {
  if (itS->GetStype()==Initial)
  {
   n++;
   badState=itS->GetName();
  }
  itS++;
 }
 if (n==1)
  OK
 else
 {
  s="ERROR: more than one initial state (one of these is "+badState+")\n";
  OUT(s);
  return false;
 }

 s="Checking that no transition arrives to the initial state... ";
 OUT(s);
 if (ThereIsOneTransArrivingTo(badState))
 {
  s="ERROR: at least one transition arrives to initial state '"+badState+"'\n";
  OUT(s);
  return false;
 }
 else
  OK

 s="Checking that no transition departs from any final state... ";
 OUT(s);
 itS=States.begin();
 n=0;
 while (itS!=States.end())
 {
  if ( (itS->GetStype()==Terminal) && (ThereIsOneTransDepartingFrom(itS->GetName())) )
  {
   badState=itS->GetName();
   n++;
  }
  itS++;
 }
 if (n>0)
 {
  s="ERROR: at least one final state ("+badState+") is the origin of a transition.\n";
  OUT(s);
  return false;
 }
 else
  OK

 s="Checking that all transitions have one existing origin state... ";
 OUT(s);
 itR=Transitions.begin();
 n=0;
 QString badTrName;
 while (itR!=Transitions.end())
 {
  if (!States.contains(itR->GetOriginState()))
  {
   n++;
   badTrName=itR->GetName();
  }
  itR++;
 }
 if (n>0)
 {
  s="ERROR: at least one transition ("+badTrName+") has an unknown or unset origin state.\n";
  OUT(s);
  return false;
 }
 else
  OK

 s="Checking that all transitions go to existing states... ";
 OUT(s);
 itR=Transitions.begin();
 n=0;
 while (itR!=Transitions.end())
 {
  QList<QString> L=itR->GetFinalStates();
  if (L.size()==0)
  {
   n++;
   badTrName=itR->GetName();
  }
  else
  {
   for (int i=0;i<L.size();i++)
    if (!States.contains(L[i]))
    {
     n++;
     badTrName=itR->GetName();
    }
  }
  itR++;
 }
 if (n>0)
 {
  s="ERROR: at least one transition ("+badTrName+") has an unknown or unset final state, or no final state at all.\n";
  OUT(s);
  return false;
 }
 else
  OK

 s="Checking that no non-terminal state is the origin of different transitions... ";
 OUT(s);
 itR=Transitions.begin();
 QList<QString> origins;
 while (itR!=Transitions.end())
 {
  origins.push_back(itR->GetOriginState());
  itR++;
 }
 n=0;
 for (int i=0;i<origins.size();i++)
  if (origins.count(origins[i])>1)
  {
   n++;
   badState=origins[i];
  }
 if (n>0)
 {
  s="ERROR: at least one non-terminal state ("+badState+") is the origin of more than one transition.\n";
  OUT(s);
  return false;
 }
 else
  OK

 // From here we don't consider the following errors but warnings indicating that the project is probably not finished yet...

 s="Checking that all non-terminal states are the origin of one transition... ";
 OUT(s);
 itS=States.begin();
 n=0;
 while(itS!=States.end())
 {
  if ( (itS->GetStype()!=Terminal) && (!ThereIsOneTransDepartingFrom(itS->GetName())) )
  {
   n++;
   badState=itS->GetName();
  }
  itS++;
 }
 if (n>0)
 {
  s="WARNING: at least one non-terminal state ("+badState+") is not the origin of any transition.\n";
  OUT(s);
  s="This is not an error, since the project might not be finished yet, but you should change the type of this/these state(s) to terminal, or erase it/them.\n";
  OUT(s);
 }
 else
  OK

 s="Checking that every state except the initial is the end of at least one transition... ";
 OUT(s);
 itS=States.begin();
 n=0;
 while(itS!=States.end())
 {
  if ( (itS->GetStype()!=Initial) && (!ThereIsOneTransArrivingTo(itS->GetName())) )
  {
   n++;
   badState=itS->GetName();
  }
  itS++;
 }
 if (n>0)
 {
  s="WARNING: at least one non-initial state ("+badState+") is not the destination of any transition.\n";
  OUT(s);
  s="This is not an error, since the project might not be finished yet, but you should erase this/these state(s).\n";
  OUT(s);
 }
 else
  OK

 return true;
}

void Project::CreateDir(QString dir)
{
 QString cmd=MKDIR_COMMAND;
 QStringList args;
 args << "-p" << PDir+dir;
 QString cmdplusargs = cmd+" -p "+PDir+dir;
 
 QProcess process;
 process.start(cmd,args);
 process.waitForFinished(-1);

 QByteArray sout=process.readAllStandardOutput();
 QByteArray serr=process.readAllStandardError();

 QString ro(sout);
 QString re(serr);

 if ( process.exitCode()!=0 )
 {
#ifdef GUI
  if (ma!=NULL)
  {
   ma->append(tr("Running command '%1'").arg(cmdplusargs));
   QVector<QString> err;
   err.push_back(tr("Process %1 ended with error. See messages area").arg(cmdplusargs));
   err.push_back(cmd);
   ErrWarn(false,err);
   ma->append(tr("Process %1 ended with error").arg(cmdplusargs));
   ma->append(tr("Standard output was:"));
   ma->append(QString::fromUtf8(ro.toStdString().c_str()));
   ma->append(tr("Standard error was:"));
   ma->append(QString::fromUtf8(re.toStdString().c_str()));
  }
  else
  {
#endif
   cerr << "Process " << cmdplusargs.toStdString() << " ended with error.\n";
   cerr << "Standard output was:\n";
   cerr << ro.toStdString();
   cerr << "\nStandard error was:\n";
   cerr << re.toStdString();
#ifdef GUI
  }
#endif
 }
}

void Project::CopyFile(QString orig,QString dest)
{
 QString cmd=COPY_COMMAND;
 
 QStringList args;
 args << "-f" << orig << dest;
 
 QProcess process;
 process.start(cmd,args);
 process.waitForFinished(-1);

 QByteArray sout=process.readAllStandardOutput();
 QByteArray serr=process.readAllStandardError();

 QString ro(sout);
 QString re(serr);

 if ( process.exitCode()!=0 )
 {
  QString cmdplusargs=cmd;
  for (QStringList::iterator it=args.begin();it!=args.end();++it)
    cmdplusargs += " "+(*it);
#ifdef GUI
  if (ma!=NULL)
  {
   ma->append(tr("Running command '%1'").arg(cmdplusargs));
   QVector<QString> err;
   err.push_back(tr("Process %1 ended with error. See messages area").arg(cmdplusargs));
   err.push_back(cmdplusargs);
   ErrWarn(false,err);
   ma->append(tr("Process %1 ended with error").arg(cmdplusargs));
   ma->append(tr("Standard output was:"));
   ma->append(QString::fromUtf8(ro.toStdString().c_str()));
   ma->append(tr("Standard error was:"));
   ma->append(QString::fromUtf8(re.toStdString().c_str()));
  }
  else
  {
#endif
   cerr << "Process " << cmdplusargs.toStdString() << " ended with error.\n";
   cerr << "Standard output was:\n";
   cerr << ro.toStdString();
   cerr << "\nStandard error was:\n";
   cerr << re.toStdString();
#ifdef GUI
  }
#endif
 }
}


void Project::CreateSubdirectories()
{
 CreateDir(HTMLDir);
 CreateDir(CodeDir);
 CreateDir(CodeDir+STCODE_DIR);
 CreateDir(CodeDir+TRCODE_DIR);
 CreateDir(CodeDir+WRCODE_DIR);
 CreateDir(CodeDir+MAINCODE_DIR);
}

void Project::SetStateHTMLFile(QString name, QString newh)
{
 if (States.contains(name) && States[name].GetHTMLFile()!=newh)
 {
  States[name].SetHTMLFile(newh);
  ToBeSaved=true;
 }
}

void Project::SetStatePosition(QString name,QPoint newpos)
{
 if (States.contains(name) && States[name].GetPos()!=newpos)
 {
  States[name].SetPos(newpos);
  ToBeSaved=true;
 }
}

void Project::Extension(QPoint &pmax)
{
 pmax.setX(0);
 pmax.setY(0);
 if (States.size()==0)
  return;
 
 QPoint npos;
 for (QStringList::iterator it=States.keys().begin(); it!=States.keys().end(); ++it)
 {
  npos=States[*it].GetPos();
  if (npos.x()>pmax.x())
   pmax.setX(npos.x());
  if (npos.y()>pmax.y())
   pmax.setY(npos.y());
 } 
 for (QStringList::iterator it=Transitions.keys().begin(); it!=Transitions.keys().end(); ++it)
 {
  npos=Transitions[*it].GetPos();
  if (npos.x()>pmax.x())
   pmax.setX(npos.x());
  if (npos.y()>pmax.y())
   pmax.setY(npos.y());
 }
}

void Project::SetStateComments(QString name,QString newcom)
{
 if (States.contains(name) && States[name].GetComments()!=newcom)
 {
  States[name].SetComments(newcom);
  ToBeSaved=true;
 }
}

void Project::SetTransitionPosition(QString name,QPoint newpos)
{
 if (Transitions.contains(name) && Transitions[name].GetPos()!=newpos)
 {
  Transitions[name].SetPos(newpos);
  ToBeSaved=true;
 }
}

QPoint Project::GetTransitionInitialPosition(QString name)
{
 if (!Transitions.contains(name))
  return QPoint(0,0);
 QString iname=Transitions[name].GetOriginState();
 if (!States.contains(iname))
  return QPoint(0,0);
 return States[iname].GetPos();
}

QVector<QPoint> Project::GetTransitionFinalPositions(QString name)
{
 QVector<QPoint> ret;

 if (!Transitions.contains(name))
  return ret;

 QList<QString> slist=Transitions[name].GetFinalStates();
 for (int i=0;i<slist.size();i++)
  ret.push_back(GetStatePosition(slist[i]));

 return ret;
}

bool Project::ThereIsOneTransDepartingFrom(QString name)
{
 if (!States.contains(name))
  return false;

 QMap<QString,Transition>::iterator it=Transitions.begin();
 while (it!=Transitions.end() && it->GetOriginState()!=name)
  it++;

 return (it!=Transitions.end());
}

bool Project::ThereIsOneTransArrivingTo(QString name)
{
 if (!States.contains(name))
  return false;

 QMap<QString,Transition>::iterator it=Transitions.begin();
 bool found=false;
 while (it!=Transitions.end() && !found)
 {
  QList<QString> l=it->GetFinalStates();
  int j=0;
  while (j<l.size() && !found)
  {
   found = (l[j]==name);
   j++;
  }
  it++;
 }

 return found;
}

QStringList Project::GetAdditionalElements(QStringList Add,QString sep)
{
 QStringList ret;
 int l=Add.size();

 for (int i=0;i<l;i++)
 {
  if (sep!="")
   ret.push_back(sep);
  ret.push_back(Add[i]);
 }
 
 return ret;
}

QString Project::GetAdditionalElementsAsString(QStringList Add,QString sep)
{
 QString ret="";
 int l=Add.size();
 
 for (int i=0;i<l;i++)
 {
  if (sep!="")
   ret += sep;
  ret += Add[i];
 }
 
 return ret;
}

QStringList Project::GetAdditionalElements(QStringList Add,QString prepend,QString sep)
{
 QStringList ret;
 int l=Add.size();
 
 for (int i=0;i<l;i++)
 {
  if (sep!="")
   ret.push_back(sep);
  ret.push_back(prepend+Add[i]);
 }
 return ret;
}

QString Project::GetAdditionalElementsAsString(QStringList Add,QString prepend,QString sep)
{
 QString ret="";
 int l=Add.size();

 for (int i=0;i<l;i++)
 {
  if ((sep!="") && (i>0))
   ret += sep;
  ret += (prepend+Add[i]);
 }
 return ret;
}
 
bool Project::AddState(State st)
{
 if (States.contains(st.GetName()))
 {
  QVector<QString> err;
  err.push_back(tr("Cannot add a state named %1. We have already a state with such name."));
  err.push_back(st.GetName());
  ErrWarn(false,err);
  return false;
 }
 States[st.GetName()]=st;
 return true;
}

void Project::SearchAndModifySName(QString PrevName,QString NewName,StateType NewType)
{
 QMap<QString,Transition>::iterator it=Transitions.begin();

 while (it!=Transitions.end())
 {
  if (it->GetOriginState()==PrevName)
   it->SetOriginState(NewName);

  if (it->GetFallState()==PrevName)
   it->SetFallState(NewName);

  QList<QString> FNameList=it->GetFinalStates();
  QList<StateType> FTypeList=it->GetFinalStTypes();
  for (int i=0;i<FNameList.size();i++)
  {
   if (FNameList[i]==PrevName)
   {
    FNameList[i]=NewName;
    FTypeList[i]=NewType;
   }
  }
  it->SetFinalStates(FNameList,FTypeList);

  it++;
 }
}

bool Project::AlterState(QString OldName,State st)
{
 if (States.contains(OldName))
 {
  if (st.GetName()==OldName)
  {
   States[OldName]=st;
   return true;
  }
  else
  {
   QMap<QString,State>::iterator it=States.begin();
   while ((it!=States.end()) && (it->GetName()!=OldName))
    it++;
   if (it==States.end())
   {
    QVector<QString> err;
    err.push_back(tr("Cannot edit the state %1. There is no a state with such name."));
    err.push_back(st.GetName());
    ErrWarn(false,err);
    return false;
   }
   States.erase(it);
   States[st.GetName()]=st;
   SearchAndModifySName(OldName,st.GetName(),st.GetStype());
   return true;
  }
 }
 else
 {
  QVector<QString> err;
  err.push_back(tr("Cannot edit the state %1. There is no a state with such name."));
  err.push_back(st.GetName());
  ErrWarn(false,err);
  return false;
 }
}

bool Project::DelState(QString namestate)
{
 if (States.contains(namestate))
 {
  // State is removed from the map of states...
  States.remove(namestate);
  // ... but any trace of it in the list of transitions must be deleted, too.
  // Let's run through the list of transitions. Notice that by doing with the list of existing keys we are sure using the [] operator of the QMap.
  QList<QString> k=Transitions.keys();
  for (int i=0;i<k.size();i++)
  {
   // If this transition has the deleted state as its origin, delete the transition completely
   // Also, transitions whose only destination is the state just deleted must be deleted, too.
   // Otherwise, delete the state from the list of final states if it is there (this is cheked by DelStateFromDestination)
   if ((Transitions[k[i]].GetOriginState()==namestate) || Transitions[k[i]].OnlyDestination(namestate) )
   {
    Transitions.remove(k[i]);
    QVector<QString> s;
    s.push_back(tr("By erasing state %1 you have also erased transition %2 that departs from it."));
    s.push_back(namestate);
    s.push_back(k[i]);
    ErrWarn(false,s);
   }
   else
    Transitions[k[i]].DelStateFromDestination(namestate);
  }
  ToBeSaved=true;
  return true;
 }
 else
 {
  QVector<QString> s;
  s.push_back(tr("Cannot erase state %1. There is no such state."));
  s.push_back(namestate);
  ErrWarn(false,s);
  return false;
 }
}

void Project::SetTransitionComments(QString name,QString newcom)
{
 if (Transitions.contains(name) && Transitions[name].GetComments()!=newcom)
 {
  Transitions[name].SetComments(newcom);
  ToBeSaved=true;
 }
}

QList<QString> Project::GetTransitionFinalStateList(QString name)
{
 if (Transitions.contains(name))
  return Transitions[name].GetFinalStates();
 else
 {
  QList<QString> ret;
  return ret;
 }
}

bool Project::AddTransition(Transition ntr)
{
 if (DEB)
  cout << "Into AddTransition(" << ntr.GetName().toStdString() << ")\n";

 QString InitialState=ntr.GetOriginState();
 if (ThereIsOneTransDepartingFrom(InitialState))
 {
  QVector<QString> err;
  err.push_back(tr("Cannot add a transition departing form state %1. We have already a transition departing form it; you wold have to erase it first."));
  err.push_back(InitialState);
  cout << "Error 2\n";
  ErrWarn(false,err);
  return false;
 }

 Transitions[ntr.GetName()]=ntr;
 return true;
}

bool Project::AlterTransition(QString OldName,Transition ntr)
{
 if (DEB)
  cout << "Into AlterTransition(" << ntr.GetName().toStdString() << ")\n";

 // First, a special provision for the case of creation: Oldname is empty and must be set to the current name
 if (OldName=="")
  OldName=ntr.GetName();
  
 if (!Transitions.contains(OldName))
 {
  QVector<QString> err;
  err.push_back(tr("Strange error. Transition %1 cannot be altered, since it did not exist."));
  err.push_back(OldName);
  ErrWarn(false,err);
  return false;
 }

 // First, the initial state must be checked. If it is the initial state of a different transition, we should erase such
 // transition, but this could provoke chained effects. Instead, we will let the user do it him/herself.
 // Also, we must check that the new original state, if the user has changed it, is not a terminal state.

 // But WARNING: to check these things we must use the original transition name to prevent the case in which the user
 // could have decided to change it.

 QString NewIState=Transitions[OldName].GetOriginState();
 if ( !States.contains(NewIState))
 {
  QVector<QString> err;
  err.push_back(tr("Strange error. State %1, supposed to be the new origin state of the new transitions, does not exist."));
  err.push_back(NewIState);
  ErrWarn(false,err);
  return false;
 }

 if ( (NewIState!=ntr.GetOriginState()) && ThereIsOneTransDepartingFrom(NewIState))
 {
  QVector<QString> err;
  err.push_back(tr("The new initial state (%1) has a transition that departs from it. You will have to erase such transition before using state %1 as origin of the new transition."));
  err.push_back(NewIState);
  ErrWarn(false,err);
  return false;
 }
 if ( States[NewIState].GetStype() == Terminal)
 {
  QVector<QString> err;
  err.push_back(tr("The desired initial state (%1) is a terminal state. No transition can depart from it."));
  err.push_back(NewIState);
  ErrWarn(false,err);
  return false;
 }
 // In the case of change of name, we must change the code file, too, and emit a warning.
 if ( OldName != ntr.GetName() )
 {
  QVector<QString> err;
  err.push_back(tr("By changing the transition name you are changing the code file name, too. Be sure to transfer your code from file %1 to file %2 if you have there code you want to keep."));
  err.push_back(OldName+DEFAULT_CPP_EXTENSION);
  err.push_back(ntr.GetName()+DEFAULT_CPP_EXTENSION);
  ErrWarn(false,err);
 }
 // Now that everything is OK, we can do the assignment...
 Transitions[ntr.GetName()]=ntr;
 // ... and the deletion of the old transition if needed
 if ( OldName != ntr.GetName() )
  Transitions.remove(OldName);

 return true;
}

bool Project::DelTransition(QString nametransition)
{
 if (Transitions.contains(nametransition))
 {
  Transitions.remove(nametransition);
  ToBeSaved=true;
  return true;
 }
 else
 {
  QVector<QString> s;
  s.push_back(tr("Cannot erase transition %1. There is no such transition."));
  s.push_back(nametransition);
  ErrWarn(false,s);
  return false;
 }
}

void Project::SaveProToTextStream(QTextStream &out)
{
 out << "<Project>\n";
 out << " <ProjectName>" << PName    << "</ProjectName>\n";

 if (!Comments.isEmpty())
  out << " <Comments>"   << Comments << "</Comments>\n";

 if (States.size()>0)
 {
  QList<QString> k=States.keys();
  out << " <States>\n";
  for (int i=0; i<States.size(); i++)
   States[k[i]].SaveToXML(out);
  out << " </States>\n";
 }

 if (Transitions.size()>0)
 {
  QList<QString> k=Transitions.keys();
  out << " <Transitions>\n";
  for (int i=0; i<Transitions.size(); i++)
   Transitions[k[i]].SaveToXML(out);
  out << " </Transitions>\n";
 }
 out << "</Project>\n";
}

void Project::SaveLocalConfToTextStream(QTextStream &out)
{
 out << "<ProjectLocalConf>\n";
 out << "<ProjectName>"  << PName    << "</ProjectName>\n";
 out << " <HTTPSMode>" << (WithHTTPS ? "true" : "false") << "</HTTPSMode>\n";
 out << " <ProjectDir>"  << PDir     << "</ProjectDir>\n";
 out << " <HTMLSubDir>"  << HTMLDir  << "</HTMLSubDir>\n";
 out << " <CodeSubDir>"  << CodeDir  << "</CodeSubDir>\n";
 out << " <ServerDebugFlag>" << ((ServerDebFlag) ? "true" : "false") << "</ServerDebugFlag>\n";
 out << " <CPPCompiler>" << CppCompiler << "</CPPCompiler>\n";
 if ((CompilerFlags.size()>1) || ((CompilerFlags.size()==1) && (CompilerFlags[0] != DEFAULT_CPP_FLAGS)))
 {
  out << " <CompilerFlags>";
  for (int i=0; i<CompilerFlags.size();i++)
   if (CompilerFlags[i] != DEFAULT_CPP_FLAGS)
    out << CompilerFlags[i] << " ";
  out << "</CompilerFlags>\n";
 }
 if (AdditionalSources.size()>0)
 {
  out << " <AdditionalSources>\n";
  for (int i=0;i<AdditionalSources.size();i++)
   if (!AdditionalSources[i].contains(CONTEXT_BASENAME))
    out << "  <Source>" << AdditionalSources[i] << "</Source>\n";
  out << " </AdditionalSources>\n";
 }
 if (AdditionalIncludeDirs.size()>0)
 {
  out << " <AdditionalIncludeDirs>\n";
  for (int i=0;i<AdditionalIncludeDirs.size();i++)
   if (AdditionalIncludeDirs[i] != DEFAULT_LOCAL_INCLUDE_DIR)
    out << "  <IncludeDir>" << AdditionalIncludeDirs[i] << "</IncludeDir>\n";
  out << " </AdditionalIncludeDirs>\n";
 }
 if (AdditionalLibDirs.size()>0)
 {
  out << " <AdditionalLibDirs>\n";
  for (int i=0;i<AdditionalLibDirs.size();i++)
   if (AdditionalLibDirs[i] != DEFAULT_LOCAL_LIB_DIR)
    out << "  <LibDir>" << AdditionalLibDirs[i] << "</LibDir>\n";
  out << " </AdditionalLibDirs>\n";
 }
 if ((AdditionalLibraries.size()>1) || ((AdditionalLibraries.size()==1) && (AdditionalLibraries[0] != DEFAULT_HT5CPP_LIB)))
 {
  out << " <AdditionalLibraries>\n";
  for (int i=0;i<AdditionalLibraries.size();i++)
   if (AdditionalLibraries[i] != DEFAULT_HT5CPP_LIB)
    out << "  <Library>" << AdditionalLibraries[i] << "</Library>\n";
  out << " </AdditionalLibraries>\n";
 }
 if (WithHTTPS)
 {
  out << " <SSLKeyFiles>\n";
  out << "  <PEMFile>" << PEMFile << "</PEMFile>\n";
  out << "  <PEMKey>" << PEMKey << "</PEMKey>\n";
  out << "  <RSAKeySize>" << PEMKeySize << "</RSAKeySize>\n";
  out << "  <DHFile>" << DHFile << "</DHFile>\n";
  out << " </SSLKeyFiles>\n";
 }
 out << "</ProjectLocalConf>\n";
}

bool Project::Save()
{
 QFile file(PDir+PFile);
 cout << "Saving project " << PName.toStdString() << " to file " << (PDir+PFile).toStdString() << endl;
 if (!file.open(QFile::WriteOnly | QFile::Text))
 {
  QVector<QString> e;
  e.push_back(tr("Cannot write file %1:\n%2."));
  e.push_back(PFile);
  e.push_back(file.errorString());
  ErrWarn(true,e);
  return false;
 }
 QTextStream out(&file);
 SaveProToTextStream(out);
 file.close();

 LocalPFile=PFile;
 LocalPFile.replace(".emw","_localconf.emw");

 QFile localfile(PDir+LocalPFile);
 cout << "Saving local configuration of project " << PName.toStdString() << " to file " << (PDir+LocalPFile).toStdString() << endl;
 if (!localfile.open(QFile::WriteOnly | QFile::Text))
 {
  QVector<QString> e;
  e.push_back(tr("Cannot write file %1:\n%2."));
  e.push_back(LocalPFile);
  e.push_back(file.errorString());
  ErrWarn(true,e);
  return false;
 }
 QTextStream localout(&localfile);
 SaveLocalConfToTextStream(localout);
 localfile.close();

 if (DEB)
  cout << "Files " << (PDir+PFile).toStdString() << " and " << (PDir+LocalPFile).toStdString() << " correctly saved.\n";

 ToBeSaved=false;
 return true;
}

bool Project::SaveAs(QString PFileWithPath)
{
 QString CurrentPFile=PFile;
 QString CurrentLocalPFile=LocalPFile;

 int l=PFileWithPath.lastIndexOf(PLATFORM_DIRSLASH);

 PDir=PFileWithPath.left(l+1);
 PFile=PFileWithPath.mid(l+1);

 if (!PFile.contains(".emw",Qt::CaseInsensitive))
  PFile+=".emw";

 LocalPFile=PFile;
 LocalPFile.replace(".emw","_localconf.emw");

 if (DEB)
  cout << "Saving project files " << PFile.toStdString() << " and " << LocalPFile.toStdString() << endl;

 if (!Save())
 {
  PFile=CurrentPFile;
  LocalPFile=CurrentLocalPFile;
  return false;
 }
 else
 {
  ToBeSaved=false;
  return true;
 }
}

QString Project::NameWithoutPath(QString s)
{
 int posp=s.length();
 while ( posp>0 && s[posp]!=PLATFORM_DIRSLASH[0] )
  posp--;
 return (posp>0) ? s.mid(posp+1,-1) : s;
}

void Project::InsertFile(ofstream &f, QString fname)
{
 ifstream g;
 if (!OpenToRead(fname,g))
  return;

 char c;
 while (!g.eof())
 {
  c=g.get();
  if (!g.eof())
   f.put(c);
 }
 g.close();
}

QString Project::FileAsQString(QString fname)
{
 ifstream g;
 if (!OpenToRead(fname,g))
  return "";
 
 QString ret="";

 char ch[2];
 ch[1]='\0';
 while (!g.eof())
 {
  ch[0]=g.get();
  if (!g.eof())
   ret.append(ch);
 }
 g.close();

 return ret;
}

void Project::InsertFileWithSubsts(ofstream &f,QString fname,QStringList to_search,QStringList to_replace)
{
 ifstream g;
 if (!OpenToRead(fname,g))
  return;
 
 QString fst=FileAsQString(fname);
 
 for (QStringList::iterator it1=to_search.begin(),it2=to_replace.begin(); it1<to_search.end(); ++it1,++it2)
  fst.replace(*it1,*it2);

 f << fst.toStdString(); 
}


bool Project::HasContent(QString r)
{
 if (r.isEmpty())
  return false;

 int i=0;
 while (i<r.length() && ( (r[i]==' ') || (r[i]==char(10)) || (r[i]==char(13)) ) )
  i++;
 return (i<r.length());
}

void Project::ExitIfNotAccessible(QString fname)
{
 if (!FileAccessible(fname))
 {
  QVector<QString> e;
  e.push_back(tr("The skeleton file %1 does not exists. Please, install the emintweb program correctly"));
  e.push_back(fname);
  ErrWarn(true,e);
  exit(1);
 }
}

// This functions generates the Project.h that will be included by Project.cpp and by Project_Call.cpp
void Project::GenCommonHeader(QString MainHeader,QString inith)
{ 
 // First, check that the two skeletons needed in this function exist
 QString skh1=INST_DIR+COMMONHEADER_1
 ExitIfNotAccessible(skh1);
 
 QString skh2=INST_DIR+COMMONHEADER_2
 ExitIfNotAccessible(skh2);
 
 // Writing the header file P.h that will be used by both, the single-instance main and the called-by-thread pseudo-main.
 ofstream fh;
 if (!OpenToWrite(MainHeader,fh))
  return;
  
 fh << "#ifndef " << GetName().toStdString() << "_H\n";
 fh << "#define " << GetName().toStdString() << "_H\n\n";
 
 fh << "#ifdef SECURE_HTTP\n";
 fh << "#define PEM_FILE   \"" << GetPEMFile().toStdString() << "\"" << endl;
 fh << "#define PEM_KEY    \"" << GetPEMKey().toStdString() << "\"" << endl;
 fh << "#define RSA_KEY_SIZE " << PEMKeySize << endl;
 fh << "#define DH_FILE    \"" << GetDHFile().toStdString() << "\"" << endl;
 fh << "#endif\n\n";
 
 InsertFile(fh,skh1);

 fh << "#include \"context" << DEFAULT_HEADER_EXTENSION.toStdString() << "\"\n\n";

 inith.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);
 QString inithtrim=NameWithoutPath(inith);
 fh << "#include \"" << inithtrim.toStdString() << "\"\n\n";

 QList<QString> Lt=GetTransitionNameList();
 for (int i=0;i<Lt.size();i++)
 {
  QString hfile=NameWithoutPath(GetTransitionCodeFile(Lt[i]));
  hfile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);
  fh << "#include \"" << hfile.toStdString() << "\"\n";
 }
 fh << endl;

 InsertFile(fh,skh2);
 
 fh << "#endif\n\n";

 fh.close();
}


void Project::GenMainHeaderCall(QString MainHeaderCall)
{
 // First, check that the keleton needed in this function exists
 QString skh=INST_DIR+MAINHEADER;
 ExitIfNotAccessible(skh);
 
 // Writing the header file that will be used only by the server multithreaded main. It has essentially the thread arguments and prototype of the thread call.
 ofstream fhc;
 if (!OpenToWrite(MainHeaderCall,fhc))
  return;

 QStringList l1,l2;
 l1.push_back("$PROJECT_NAME");
 l2.push_back(GetName());
 l1.push_back("$PRNAME_WITH_EXT");
 l2.push_back(GetName()+DEFAULT_HEADER_EXTENSION);
 
 InsertFileWithSubsts(fhc,skh,l1,l2);
 
 fhc.close();
}

void Project::GenSingleMainAndThreadHeaders(QString MainHeader,QString MainHeaderCall,QString inith)
{
 GenCommonHeader(MainHeader,inith);
 GenMainHeaderCall(MainHeaderCall);
}

void Project::GenSingleMainAndThreadBodies(QString MainSource,QString MainSourceCall,QString inith)
{
 // First, check that the fourskeletons needed in this function exist
 QString skmb=INST_DIR+MAIN_SKMB;
 ExitIfNotAccessible(skmb);
 
 QString ska=INST_DIR+MAIN_SKMA;
 ExitIfNotAccessible(ska);
 
 QString sktb=INST_DIR+MAIN_SKTB;
 ExitIfNotAccessible(sktb);
 
 QString skta=INST_DIR+MAIN_SKTA;
 ExitIfNotAccessible(skta);
 
 // Now, the main files for both cases will be written simultaneously, since they are very similar
 ofstream fc;
 if (!OpenToWrite(MainSource,fc))
  return;
 ofstream fcc;
 if (!OpenToWrite(MainSourceCall,fcc))
  return;
  
 fc  << "#include \"" << GetName().toStdString() << DEFAULT_HEADER_EXTENSION.toStdString() << "\"\n\n";
 fcc << "#include \"" << GetName().toStdString() << "_Call" << DEFAULT_HEADER_EXTENSION.toStdString() << "\"\n\n";
 
 // Skeleton for main:
 InsertFile(fc,skmb);
 
 // Skeleton for thread
 QStringList l1,l2;
 l1.push_back("$PROJECT_NAME");
 l2.push_back(GetName());
 InsertFileWithSubsts(fcc,sktb,l1,l2);
 
 // Now, for both again:
 QList<QString> Tl=GetTransitionNameList();
 ostringstream outcode;
 outcode << " map<string,string (*)(Poco::Net::NameValueCollection &,string &,Context &)> tf;\n";
 outcode << "\n map<string,string> nf;\n";
 for (int i=0;i<Tl.size();i++)
  outcode << " nf[\"" << GetTransitionInitialState(Tl[i]).toStdString() << "\"]=\"Wrapper_Tr_" << Tl[i].toStdString() << "\";\n";
 outcode << endl;
 for (int i=0;i<Tl.size();i++)
  outcode << " tf[\"" << GetTransitionInitialState(Tl[i]).toStdString() << "\"]=Wrapper_Tr_" << Tl[i].toStdString() << ";\n";
 outcode << endl;
 outcode << "\n string this_state=\"" << GetInitState().toStdString() << "\";\n";
 outcode << " string page = State_" << GetInitState().toStdString() << "(";
 inith.replace(DEFAULT_CPP_EXTENSION,"_call"+DEFAULT_CPP_EXTENSION);
 QString call=FileAsQString(inith);
 if (HasContent(call))
  outcode << call.toStdString() << ",";
 outcode << "theContext.GetIndentString());\n\n";
  
 fc  << outcode.str();
 fcc << outcode.str();
 
 // The rest of the files are done inserting a different skeleton for each one
 InsertFile(fc,ska);
 fc.close();
 
 InsertFile(fcc,skta);
 fcc.close();
}

void Project::GenServer(QString MainHeaderServer,QString MainSourceServer)
{
 // Finally, the complete server. First, the header. No prototype for it
 ofstream fcsh;
 if (!OpenToWrite(MainHeaderServer,fcsh))
  return;
 fcsh << "#ifndef " << GetName().toStdString() << "_Server_H\n";
 fcsh << "#define " << GetName().toStdString() << "_Server_H\n\n";

 fcsh << "#include <pthread.h>\n";
 fcsh << "#include \"" << GetName().toStdString() << "_Call" << DEFAULT_HEADER_EXTENSION.toStdString() << "\"\n\n";
 fcsh << "#define NUM_NATIVE_ARGS_TO_PROG 4\n\n";
 fcsh << "#ifdef SECURE_HTTP\n";
 fcsh << "bool SendInitialPage(string server_name,unsigned short newport,unsigned short entryport,Poco::Net::SecureStreamSocket &sts,ostream &errst);\n";
 fcsh << "bool SendBusyPage(unsigned short entry_port,Poco::Net::SecureStreamSocket &sts,ostream &errst);\n";
 fcsh << "#else\n";
 fcsh << "bool SendInitialPage(string server_name,unsigned short newport,unsigned short entryport,Poco::Net::StreamSocket &sts,ostream &errst);\n";
 fcsh << "bool SendBusyPage(unsigned short entry_port,Poco::Net::StreamSocket &sts,ostream &errst);\n";
 fcsh << "#endif\n\n";
 fcsh << "#define CALL_TO_THREAD " << GetName().toStdString() << "Call\n";
 fcsh << "using namespace std;\n";
 fcsh << "#endif\n";
 fcsh.close();
 
 // and then, the source, but fortunately it is almost invariant and can be copied as skeleton
 QString skser=INST_DIR+MAIN_SERVER;
 ExitIfNotAccessible(skser);
 
 ofstream fcsc;
 if (!OpenToWrite(MainSourceServer,fcsc))
  return;
 fcsc << "#include \"" << GetName().toStdString() << "_Server" << DEFAULT_HEADER_EXTENSION.toStdString() << "\"\n\n";
 InsertFile(fcsc,skser);
 fcsc.close();
}

void Project::GenMain()
{
 // If name of project is P, then:
 // This is for the main progarm, called P.cpp and P.h
 // FiLE 1
 QString MainSource=GetPDir()+GetCodeDir()+MAINCODE_DIR+GetName()+DEFAULT_CPP_EXTENSION;
 // FILE 2
 QString MainHeader=GetPDir()+GetCodeDir()+MAINCODE_DIR+GetName()+DEFAULT_HEADER_EXTENSION;
 
 
 // This is for the version of the programa callable by the global server, called P_Call.cpp and P_call.h
 // FILE 3
 QString MainSourceCall=GetPDir()+GetCodeDir()+MAINCODE_DIR+GetName()+QString("_Call")+DEFAULT_CPP_EXTENSION;
 //FILE 4
 QString MainHeaderCall=GetPDir()+GetCodeDir()+MAINCODE_DIR+GetName()+QString("_Call")+DEFAULT_HEADER_EXTENSION;

 // And the main of such program, called P_Server.h and P_Server.cpp
 // FILE 5
 QString MainSourceServer=GetPDir()+GetCodeDir()+MAINCODE_DIR+GetName()+QString("_Server")+DEFAULT_CPP_EXTENSION;
 // FILE 6
 QString MainHeaderServer=GetPDir()+GetCodeDir()+MAINCODE_DIR+GetName()+QString("_Server")+DEFAULT_HEADER_EXTENSION;
 
 QString inith=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(GetInitState());

 // This generates files 2 and 4
 GenSingleMainAndThreadHeaders(MainHeader,MainHeaderCall,inith);
 
 // This generates file 1 and 3
 GenSingleMainAndThreadBodies(MainSource,MainSourceCall,inith);
  
 // This generetes file 5 and 6
 GenServer(MainHeaderServer,MainSourceServer);
}

void Project::GenMake()
{
 QString Mfile=GetPDir()+GetCodeDir()+DEFAULT_MAKEFILE;

 ofstream f;
 if (!OpenToWrite(Mfile,f))
  return;
 
 if (DEB)
  cout << "Generating Makefile\n";

 QString PFile=GetPDir()+GetPFile();

 f << "CODEDIR := " << (GetPDir()+GetCodeDir()).toStdString() << endl << endl;
 f << "HTMLDIR := " << (GetPDir()+GetHTMLDir()).toStdString() << endl << endl;
 f << "GENDEB := " << (GetServerDebFlag() ? "true" : "false") << endl << endl;
 f << "SRC_MAIN := " << HERE.toStdString() << MAINCODE_DIR.toStdString() << GetName().toStdString() << DEFAULT_CPP_EXTENSION.toStdString() << endl << endl;
 f << "SRC_SERVERMAIN := " << HERE.toStdString() << MAINCODE_DIR.toStdString() << GetName().toStdString() << "_Server" << DEFAULT_CPP_EXTENSION.toStdString() << endl << endl;
 f << "SRC_THREAD := " << HERE.toStdString() << MAINCODE_DIR.toStdString() << GetName().toStdString() << "_Call" << DEFAULT_CPP_EXTENSION.toStdString() << endl << endl;
 f << "SRC_STATES := ";
  
 QList<QString> Ls=GetStateNameList();
 for (int i=0;i<Ls.size();i++)
  f << HERE.toStdString() << STCODE_DIR.toStdString() << GetStateCodeFile(Ls[i]).toStdString() << " ";
 f << endl << endl;
 
 f << "SRC_TRANSITIONS := ";
 QList<QString> Lt=GetTransitionNameList();
 for (int i=0;i<Lt.size();i++)
  f << HERE.toStdString() << TRCODE_DIR.toStdString() << GetTransitionCodeFile(Lt[i]).toStdString() << " ";
 f << endl << endl;
 
 QString dummy=GetAdditionalSourcesAsString(HERE+TRCODE_DIR);
 f << "SRC_CONTEXT := " << GetAdditionalSourcesAsString(HERE+TRCODE_DIR).toStdString() << endl << endl;
 
 f << "INITSTATE_HEADER := " << (HERE+STCODE_DIR).toStdString() << GetStateCodeFile(GetInitState()).toStdString() << endl << endl;
 
 f << "CPP := " << GetCppCompiler().toStdString() << endl << endl;
 
 f << "CXXFLAGS := " << GetCompFlagsAsString().toStdString();
 if (GetHTTPSMode())
  f << " -DSECURE_HTTP ";
  
 f << GetAdditionalIncludeDirsAsString().toStdString() << " ";
 
 QString moreinc=" -I "+HERE+STCODE_DIR+" -I "+HERE+TRCODE_DIR;
 f << moreinc.toStdString() << endl << endl;
 
 f << "LDFLAGS := " << GetAdditionalLibDirsAsString().toStdString() << " " << GetAdditionalLibrariesAsString().toStdString() << endl << endl;
 
 f << "# Don't change anything from here:\n\n";
 f << "HDR_MAIN := $(SRC_MAIN:.cpp=.h)\n";
 f << "OBJ_MAIN := $(SRC_MAIN:.cpp=.o)\n";
 f << "HDR_SERVERMAIN := $(SRC_SERVERMAIN:.cpp=.h)\n";
 f << "OBJ_SERVERMAIN := $(SRC_SERVERMAIN:.cpp=.o)\n";
 f << "HDR_THREAD := $(SRC_THREAD:.cpp=.h)\n";
 f << "OBJ_THREAD := $(SRC_THREAD:.cpp=.o)\n";
 f << "EXE_FILE := $(SRC_MAIN:.cpp=)\n";
 f << "EXE_SERVER := $(SRC_SERVERMAIN:.cpp=)\n";
 f << "HDR_STATES := $(SRC_STATES:.cpp=.h)\n";
 f << "OBJ_STATES := $(SRC_STATES:.cpp=.o)\n";
 f << "HDR_TRANSITIONS := $(SRC_TRANSITIONS:.cpp=.h)\n";
 f << "OBJ_TRANSITIONS := $(SRC_TRANSITIONS:.cpp=.o)\n";
 f << "ITR1_TRANSITIONS := $(SRC_TRANSITIONS:.cpp=)\n";
 f << "ITR_TRANSITIONS := $(ITR1_TRANSITIONS:./Transitions=)\n";
 f << "SRC_WRAPPERS := $(subst Transitions/,Wrappers/wrapper_,$(SRC_TRANSITIONS))\n";
 f << "HDR_WRAPPERS := $(SRC_WRAPPERS:.cpp=.h)\n";
 f << "OBJ_WRAPPERS := $(SRC_WRAPPERS:.cpp=.o)\n";
 f << "HDR_CONTEXT := $(SRC_CONTEXT:.cpp=.h)\n";
 f << "OBJ_CONTEXT := $(SRC_CONTEXT:.cpp=.o)\n\n";
 f << "all: single server\n\n";
 f << "single: $(EXE_FILE)\n\n";
 f << "server: $(EXE_SERVER)\n\n";
 f << "cleanauxstates:\n";
 f << "\t-rm -f ./States/*_call.cpp ./States/*_hv.cpp ./States/*_hv.h ./States/*_vars.cpp\n\n";
 f << "cleanstates: cleanauxstates\n";
 f << "\t-rm -f $(HDR_STATES) $(SRC_STATES) $(OBJ_STATES)\n\n";
 f << "cleantransitions:\n";
 f << "\t-rm -f $(HDR_TRANSITIONS) $(OBJ_TRANSITIONS)\n\n";
 f << "cleanwrappers:\n";
 f << "\t-rm -f $(HDR_WRAPPERS) $(SRC_WRAPPERS) $(OBJ_WRAPPERS)\n\n";
 f << "cleanmain:\n";
 f << "\t-rm -f $(HDR_MAIN) $(SRC_MAIN) $(OBJ_MAIN) $(HDR_SERVERMAIN) ${SRC_SERVERMAIN} $(OBJ_SERVERMAIN) $(HDR_THREAD) ${SRC_THREAD} $(OBJ_THREAD) $(OBJ_CONTEXT) $(EXE_FILE) $(EXE_SERVER)\n\n";
 f << "clean: cleanstates cleantransitions cleanwrappers cleanmain\n\n";
 f << "$(SRC_SERVERMAIN): $(SRC_STATES) $(SRC_TRANSITIONS) $(HDR_CONTEXT) $(HDR_TRANSITIONS)\n";
 f << "\t" << (EIWCLI+" "+PFile).toStdString() << " -gmain\n\n";
 f << "$(SRC_THREAD): $(SRC_STATES) $(SRC_TRANSITIONS) $(HDR_CONTEXT) $(HDR_TRANSITIONS)\n";
 f << "\t" << (EIWCLI+" "+PFile).toStdString() << " -gmain\n\n";
 f << "$(SRC_MAIN): $(SRC_STATES) $(SRC_TRANSITIONS) $(HDR_CONTEXT) $(HDR_TRANSITIONS)\n";
 f << "\t" << (EIWCLI+" "+PFile).toStdString() << " -gmain\n\n";
 f << "$(OBJ_THREAD): $(SRC_THREAD) $(HDR_THREAD) $(HDR_MAIN)\n";
 f << "\t$(CPP) -c $(CXXFLAGS) $(SRC_THREAD) -o $(OBJ_THREAD)\n\n";
 f << "$(OBJ_SERVERMAIN): $(SRC_SERVERMAIN) $(HDR_SERVERMAIN)\n";
 f << "\t$(CPP) -c $(CXXFLAGS) $(SRC_SERVERMAIN) -o $(OBJ_SERVERMAIN)\n\n";
 f << "$(OBJ_MAIN): $(SRC_MAIN) $(HDR_MAIN)\n";
 f << "\t$(CPP) -c $(CXXFLAGS) $(SRC_MAIN) -o $(OBJ_MAIN)\n\n";
 f << "%.o: %.cpp %.h\n";
 f << "\t$(CPP) -c $(CXXFLAGS) $< -o $@\n\n";
 f << "$(EXE_FILE): $(OBJ_STATES) $(OBJ_CONTEXT) $(OBJ_TRANSITIONS) $(OBJ_WRAPPERS) $(OBJ_MAIN)\n";
 f << "\t$(CPP) $(OBJ_MAIN) $(OBJ_STATES) $(OBJ_CONTEXT) $(OBJ_TRANSITIONS) $(OBJ_WRAPPERS) -o $(EXE_FILE) $(LDFLAGS)\n\n";
 f << "$(EXE_SERVER): $(OBJ_STATES) $(OBJ_CONTEXT) $(OBJ_TRANSITIONS) $(OBJ_WRAPPERS) $(OBJ_THREAD) $(OBJ_SERVERMAIN)\n";
 f << "\t$(CPP) $(OBJ_SERVERMAIN) $(OBJ_THREAD) $(OBJ_STATES) $(OBJ_CONTEXT) $(OBJ_TRANSITIONS) $(OBJ_WRAPPERS) -o $(EXE_SERVER) $(LDFLAGS)\n\n";
 
 f << "# Rules for states\n\n";
 for (int i=0;i<Ls.size();i++)
 {
  f << (HERE+STCODE_DIR+GetStateCodeFile(Ls[i])).toStdString() << ": $(HTMLDIR)" << GetStateHTMLFile(Ls[i]).toStdString() << endl;
  f << "\t" << (EIWCLI+" "+PFile).toStdString() << " -gsc " << Ls[i].toStdString() << endl;
 }
 f << endl;
 f << "# Rules for transitions\n\n";
 QString tcode,theader,twcode;
 for (int i=0;i<Lt.size();i++)
 {
  tcode=HERE+TRCODE_DIR+GetTransitionCodeFile(Lt[i]);
  theader=tcode;
  theader.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);
  twcode=HERE+WRCODE_DIR+"wrapper_"+GetTransitionCodeFile(Lt[i]);
  f << theader.toStdString() << ": \n";
  f << "\t" << (EIWCLI+" "+PFile).toStdString() << " -gtc " << Lt[i].toStdString() << endl;
  f << tcode.toStdString() << ":\n";
  f << "\t" << (EIWCLI+" "+PFile).toStdString() << " -gtc " << Lt[i].toStdString() << endl;
  f << "\t$(CPP) -c $(CXXFLAGS) " << twcode.toStdString();
  twcode.replace(DEFAULT_CPP_EXTENSION,DEFAULT_OBJECT_EXTENSION);
  f << " -o " << twcode.toStdString() << endl;
 }
 f << endl;
 f << "# End of Makefile for project " << GetName().toStdString() << endl;
 f.close();
}

void Project::GenCodeForState(QString NameState, bool GenTest)
{
 QString fnHtml=GetPDir()+GetHTMLDir()+GetHTMLFile(NameState);
 QString fnCfile=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(NameState);
 if (GenTest)
  fnCfile.replace(DEFAULT_CPP_EXTENSION,"_test"+DEFAULT_CPP_EXTENSION);

 QString basefile=fnCfile;
 basefile.replace(DEFAULT_CPP_EXTENSION,"");

 QString fnHfile=basefile+DEFAULT_HEADER_EXTENSION;

 int r=fnHfile.lastIndexOf(".");
 int r1=fnHfile.size();
 while ((r1>0) && fnHfile[r1]!=PLATFORM_DIRSLASH[0])
  r1--;

 QString htnameinc=fnHfile.mid(r1+1,r-r1-1);

 if (!FileAccessible(fnHtml))
 {
  QVector<QString> e;
  e.push_back(tr("The file %1 does not exists.\nPlease, create it first."));
  e.push_back(fnHtml);
  ErrWarn(false,e);
  return;
 }

 HDoc D(fnHtml.toStdString(),NameState.toStdString(),htnameinc.toStdString(),GenTest);

 ofstream fc;
 if (!OpenToWrite(fnCfile,fc))
  return;
 fc << D.GenCCode(GenTest);
 fc.close();

 ofstream fh;
 if (!OpenToWrite(fnHfile,fh))
  return;
 fh << D.GenHeader();
 fh.close();

 QString hvfile=basefile+"_hv"+DEFAULT_HEADER_EXTENSION;
 ofstream fhv;
 if (!OpenToWrite(hvfile,fhv))
  return;
 fhv << D.GenHV();
 fhv.close();

 QString callfile=basefile+"_call"+DEFAULT_CPP_EXTENSION;
 ofstream fcall;
 if (!OpenToWrite(callfile,fcall))
  return;
 fcall << D.GenCall();
 fcall.close();

 QString varsfile=basefile+"_vars"+DEFAULT_CPP_EXTENSION;
 ofstream fvars;
 if (!OpenToWrite(varsfile,fvars))
  return;
 fvars << D.GenVars();
 fvars.close();
}

void Project::GenCodeForTransition(QString NameTrans)
{
 QString CodeFile=GetPDir()+GetCodeDir()+TRCODE_DIR+GetTransitionCodeFile(NameTrans);

 QString HeaderFile=CodeFile;
 HeaderFile.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);

 QString HeaderFileWithoutPath=NameWithoutPath(HeaderFile);

 QString WCodeFile=GetPDir()+GetCodeDir()+WRCODE_DIR+"wrapper_"+GetTransitionCodeFile(NameTrans);

 QList<QString> FinalStates=GetTransitionFinalStateList(NameTrans);
 for (int i=0;i<FinalStates.size();i++)
 {
  bool allgen=true,genok;

  QString SCode=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(FinalStates[i]);
  QString NotGen="";
  
  QString NeededCode=SCode;
  genok = FileAccessible(NeededCode);
  if (!genok)
   NotGen += (NeededCode+" ");
  allgen &= genok;
  
  NeededCode=SCode;
  NeededCode.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);
  genok = FileAccessible(NeededCode);
  if (!genok)
   NotGen += (NeededCode+" ");
  allgen &= genok;
 
  NeededCode=SCode;
  NeededCode.replace(DEFAULT_CPP_EXTENSION,"_hv"+DEFAULT_HEADER_EXTENSION);
  genok = FileAccessible(NeededCode);
  if (!genok)
   NotGen += (NeededCode+" ");
  allgen &= genok;
  
  NeededCode=SCode;
  NeededCode.replace(DEFAULT_CPP_EXTENSION,"_call"+DEFAULT_CPP_EXTENSION);
  genok = FileAccessible(NeededCode);
  
  allgen &= FileAccessible(NeededCode);
  if (!genok)
   NotGen += (NeededCode+" ");
  allgen &= genok;
  
  NeededCode=SCode;
  NeededCode.replace(DEFAULT_CPP_EXTENSION,"_vars"+DEFAULT_CPP_EXTENSION);
  genok = FileAccessible(NeededCode);
  if (!genok)
   NotGen += (NeededCode+" ");
  allgen &= genok;
  
  if (!allgen)
  {
   QVector<QString> e;
   e.push_back(tr("Code for state %1, which is one of the final states of transition %2, has not yet been generated. This/These file(s) is/are missing: $3"));
   e.push_back(FinalStates[i]);
   e.push_back(NameTrans);
   e.push_back(NotGen);
   ErrWarn(false,e);
   return;
  }
 }
 
 if (DEB)
  cout << "Generating header file " << HeaderFile.toStdString() << endl;
  
 string cext=DEFAULT_CPP_EXTENSION.toStdString();
 string hext=DEFAULT_HEADER_EXTENSION.toStdString();
 QString IniState=GetTransitionInitialState(NameTrans);
 QString FallState=GetTransitionFallStateName(NameTrans);

 // First we generate the header file of the transition
 ofstream f;
 if (!OpenToWrite(HeaderFile,f))
  return;

 f << "#ifndef _Tr_" << NameTrans.toStdString() << "_H\n";
 f << "#define _Tr_" << NameTrans.toStdString() << "_H\n";
 f << "#include <string>\n";
 f << "#include <utility>\n";
 f << "#include <vector>\n";
 f << "#include <Poco/Net/NameValueCollection.h>\n";
 f << "#include <html5document.h>\n\n";
 f << "#include \"context" << DEFAULT_HEADER_EXTENSION.toStdString() << "\"\n\n";

 f << "// Here we include the headers of all the states to which this transition may lead, including the fallback state\n\n";

 QString dummy;
 for (int i=0;i<FinalStates.size();i++)
 {
  QString dummy=GetStateCodeFile(FinalStates[i]);
  dummy.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);
  f << "#include \"" << dummy.toStdString() << "\"\n";
 }
 dummy=GetStateCodeFile(FallState);
 dummy.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION);
 f << "#include \"" << dummy.toStdString() << "\"\n";

 f << "\ntypedef vector< pair<string,string> > OVARS;\n\n";

 f << "// This is the prototype of an internal wrapper whose code is automatically built. You can forget it (but don't erase it)\n";
 f << "string Wrapper_Tr_" << NameTrans.toStdString() << "( Poco::Net::NameValueCollection &ival, string &next_state, Context &usercontext);\n\n";
 if (FinalStates.size()>1)
 {
  f << "// A function you will have to fill to know to which one of the next states we will have to go\n";
  f << "string Tr_" << NameTrans.toStdString() << "_choice(OVARS &ov, Context &usercontext);\n\n";
 }

 f << "// The function(s) that you will have to fill with the real work of your program...\n\n";
 if (FinalStates.size()>1)
 {
  f << "// All these functions should return 'true' if they perform as expected.\n";
  f << "// In this case the next state will be as given by the former Tr_" << NameTrans.toStdString() << "_choice function\n";
  f << "// If a function returns 'false' the next state will be the fallback state for this transition.\n";
 }
 else
 {
  f << "// This function should return 'true' if it performs as expected.\n";
  f << "// If it returns 'false' the next state will be the fallback state for this transition.\n";
 }
 for (int i=0;i<FinalStates.size();i++)
 {
  f << "bool " << NameTrans.toStdString() << "_to_" << FinalStates[i].toStdString() << "( OVARS &ov";
  string FinState=GetStateCodeFile(FinalStates[i]).toStdString();
  QString nfile=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(FinalStates[i]);
  nfile.replace(DEFAULT_CPP_EXTENSION,"_hv"+DEFAULT_HEADER_EXTENSION);
  InsertFile(f,nfile);

  f << ", Context &usercontext);\n";
 }
 f << "\n#endif\n";
 f.close();

 // Now, we generate the wrapper of the transition (C++ code that the user will not see or change)
 if (DEB)
  cout << "Generating wrapper code file " << WCodeFile.toStdString() << endl;

 ofstream fhc;
 if (!OpenToWrite(WCodeFile,fhc))
  return;

 fhc << "// C++ function for the transition from state ["+IniState.toStdString()+"] to state(s) ";
 for (int i=0;i<FinalStates.size();i++)
  fhc << FinalStates[i].toStdString() << " ";
 fhc << "with fallback state [" << FallState.toStdString() << "]\n" << endl << endl;

 fhc << "// This is automatically generated code.\n";
 fhc << "// DO NOT EDIT manually this file, your changes will be lost the next time it is generated.\n\n";
 fhc << "#include \"" << HeaderFileWithoutPath.toStdString() << "\"\n\n";
 fhc << "using namespace std;\n\n";
 fhc << "string Wrapper_Tr_" << NameTrans.toStdString() << "( Poco::Net::NameValueCollection &ival, string &next_state, Context &usercontext )\n";
 fhc << "{\n";
 fhc << " OVARS ov;\n";
 fhc << " for (Poco::Net::NameValueCollection::ConstIterator it=ival.begin(); it!=ival.end(); ++it)\n";
 fhc << " {\n";
 fhc << "  pair<string,string> p;\n";
 fhc << "  p.first=it->first;\n";
 fhc << "  p.second=it->second;\n";
 fhc << "  ov.push_back(p);\n";
 fhc << " }\n\n";

 if (FinalStates.size()>1)
  fhc << " string chosen=Tr_" << NameTrans.toStdString() << "_choice(ov, usercontext);\n\n";

 fhc.flush();

 QString vfile;
 QString callfile;
 QString r;

 for (int i=0;i<FinalStates.size();i++)
 {
  if (FinalStates.size()>1)
  {
   if (GetStateType(FinalStates[i])==Terminal)
    fhc << " if (chosen==\"End_of_program\")\n";
   else
    fhc << " if (chosen==\"" << FinalStates[i].toStdString()<< "\")\n";
   fhc << " {\n";
  }

  vfile=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(FinalStates[i]);
  vfile.replace(DEFAULT_CPP_EXTENSION,"_vars"+DEFAULT_CPP_EXTENSION);
  InsertFile(fhc,vfile);

  /* This debug detail was excessive...
  fhc << "  if (SERVERDEB)\n";
  fhc << "  {\n";
  fhc << "   usercontext.GetOutletStream() << \"Wrapper just before calling function " << NameTrans.toStdString() << "_to_" << FinalStates[i].toStdString() << "\";\n";
  fhc << "   usercontext.GetOutletStream().flush();\n";
  fhc << "  }\n";
  */
  fhc << "  if ( " << NameTrans.toStdString() << "_to_" << FinalStates[i].toStdString() << "(ov";
  callfile=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(FinalStates[i]);
  callfile.replace(DEFAULT_CPP_EXTENSION,"_call"+DEFAULT_CPP_EXTENSION);
  r=FileAsQString(callfile);
  if (HasContent(r))
   fhc << "," << r.toStdString();

  fhc << ", usercontext) )\n  {\n";
  if (GetStateType(FinalStates[i])==Terminal)
   fhc << "   next_state = \"End_of_program\";\n";
  else
   fhc << "   next_state = \"" << FinalStates[i].toStdString() << "\";\n";

  fhc << "   return State_" << FinalStates[i].toStdString() << "(";

  if (HasContent(r))
   fhc << r.toStdString() << ",";

  fhc << "usercontext.GetIndentString());\n  }\n";

  if (FinalStates.size()>1)
   fhc << " }\n\n";
  fhc.flush();
 }

 fhc << "\n // If we are here, an error has happened and we resort to the fallback state\n";
 fhc << " // (that might well be one of the previous ones...)\n";
 fhc << " next_state = \"" << FallState.toStdString() << "\";\n";
 fhc << " return State_" << FallState.toStdString() << "(";

 callfile=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(FallState);
 callfile.replace(DEFAULT_CPP_EXTENSION,"_call"+DEFAULT_CPP_EXTENSION);
 r=FileAsQString(callfile);
 if (HasContent(r))
  fhc << r.toStdString() << ",";

 fhc << "usercontext.GetIndentString());\n}\n";

 fhc.close();

 // and finally, the empty code for the transition, but only if the file didn't exist yet.
 // Otherwise, we would kill the valuable user code (and the user would kill us..)
 
 if (DEB)
  cout << "Attempt to generate code file " << CodeFile.toStdString() << "... ";
  
 if (FileAccessible(CodeFile))
 {
  if (DEB)
   cout << "File " << CodeFile.toStdString() << " is NOT regenerated. (It exists and could contain user code).\n";
#ifdef GUI
  OUT(tr("[[ File %1 is NOT regenerated. (It exists and could contain user code) ]] ").arg(CodeFile));
#else
  QVector<QString> e;
  e.push_back(tr("File %1 is NOT regenerated.\n(It exists and could contain user code)."));
  e.push_back(CodeFile);
  ErrWarn(false,e);
#endif
  return;
 }

 ofstream fhc2;
 if (!OpenToWrite(CodeFile,fhc2))
  return;
  
 if (DEB)
  cout << "Generating for the first time such user code file.\n";

 fhc2 << "// This code has been automatically generated only once\n";
 fhc2 << "// The changes that you introduce here will not be lost, but if the prototype of the state function changes,\n";
 fhc2 << "// you will have to change it here or you will get compilation errors.\n";
 fhc2 << "// You can consult the right prototype at each moment in file " << HeaderFile.toStdString() << endl;
 fhc2 << "// Don't get rid of the include of that file. Apart from it, you can of course include any files you need.\n";
 fhc2 << "// And remember that OVARS has been defined as vector < pair<string,string> > so you can get the variables\n";
 fhc2 << "// with something like this: 'for (int i=0;i<ov.size();i++) { string vname=ov[i].first; string vval=ov[i].second; etc... }'\n\n";
 fhc2 << "#include \"" << HeaderFileWithoutPath.toStdString() << "\"\n\n";
 if (FinalStates.size()>1)
 {
  fhc2 << "string Tr_" << NameTrans.toStdString() << "_choice(OVARS &ov, Context &usercontext)\n";
  fhc2 << "{\n";
  for (int i=0;i<FinalStates.size();i++)
  {
   fhc2 << " if ( true /* Substitute this 'true' by the condition to be satisfied for the next state to be " << FinalStates[i].toStdString() << " */ )\n";
   if (GetStateType(FinalStates[i])==Terminal)
    fhc2 << "  return(\"End_of_program\");\n\n";
   else
    fhc2 << "  return(\"" << FinalStates[i].toStdString() << "\");\n\n";
  }
  fhc2 << " // Don't return any string different from the avobe-mentioned or you will get run-time errors\n";
  fhc2 << "}\n\n";
 }

 for (int i=0;i<FinalStates.size();i++)
 {
  fhc2 << "bool " << NameTrans.toStdString() << "_to_" << FinalStates[i].toStdString() << "(OVARS &ov";
  // The header call variables for this final state
  QString nfile=GetPDir()+GetCodeDir()+STCODE_DIR+GetStateCodeFile(FinalStates[i]);
  nfile.replace(DEFAULT_CPP_EXTENSION,"_hv"+DEFAULT_HEADER_EXTENSION);
  InsertFile(fhc2,nfile);
  fhc2 << ",Context &usercontext)\n{\n\n try\n ";
  fhc2 << "{\n  // You should have returned false before this point if something has gone wrong.\n ";
  fhc2 << "}\n catch (const exception &e)\n {\n";
  fhc2 << "  cerr << \"Exception in function " << NameTrans.toStdString() << "_to_" << FinalStates[i].toStdString() << "\\n\";\n";
  fhc2 << "  cerr << \"Exception reason is \" << e.what() << \"\\n\";\n";
  fhc2 << "  throw;\n";
  fhc2 << " }\n";
  fhc2 << " return true;\n}\n\n";
 }
 fhc2.close();
}

void Project::GenContext(bool Overwrite)
{
 // This generates the serverdeb.h file which contains only the define of the SERDERDEB constant to true or false
 QString hsvf=GetPDir()+GetCodeDir()+TRCODE_DIR+"serverdeb"+DEFAULT_HEADER_EXTENSION;
 ofstream fhsvf(hsvf.toStdString().c_str());
 fhsvf << "#define SERVERDEB " << ((GetServerDebFlag()==true) ? "true" : "false") << endl;
 fhsvf.close();
 
 // This is to generate the context files (header and source)
 QString hf=GetPDir()+GetCodeDir()+TRCODE_DIR+CONTEXT_HEADER;
 QString cf=GetPDir()+GetCodeDir()+TRCODE_DIR+CONTEXT_SOURCE;

 // Generation of context header if it did not exist
 QString origf=INST_DIR+CONTEXT_BASENAME+"_skeleton"+DEFAULT_HEADER_EXTENSION;

 ifstream fh(hf.toStdString().c_str());
 // If it was not there, copy it from the skeleton
 if (!fh.is_open())
 {
  QVector<QString> e;
  e.push_back(tr("File %1 did not exist. It will created from the template."));
  e.push_back(hf);
  ErrWarn(false,e);
  CopyFile(origf,hf);
 }
 else
 {
  fh.close();
  // If it was there, but we want to regenerate, we overwrite it
  if (Overwrite)
  {
   QVector<QString> e;
   e.push_back(tr("File %1 EXISTS. It will be overwritten with the template, according to your directions."));
   e.push_back(hf);
   ErrWarn(false,e);
   //ofstream fh(hf.toStdString().c_str());
   //fh << "#define SERVERDEB " << ((GetServerDebFlag()==true) ? "true" : "false") << endl << endl;
   //fh.close();
   CopyFile(origf,hf);
  }
  else
  {
   QVector<QString> e;
   e.push_back(tr("File %1 already exists.\n(It will not be overwritten with the template, according to your directions)."));
   e.push_back(hf);
   ErrWarn(false,e);   
  }
 }
 
 // Generation of context source if it did not exist
 QString origc=INST_DIR+CONTEXT_BASENAME+"_skeleton"+DEFAULT_CPP_EXTENSION;

 ifstream fc(cf.toStdString().c_str());
 // If it was not there, copy it from the skeleton
 if (!fc.is_open())
 {
  QVector<QString> e;
  e.push_back(tr("File %1 did not exist. It will created from the template."));
  e.push_back(cf);
  ErrWarn(false,e);
  CopyFile(origc,cf);
 }
 else
 {
  fc.close();
  // If it was there, but we want to regenerate, we overwrite it
  if (Overwrite)
  {
   QVector<QString> e;
   e.push_back(tr("File %1 EXISTS. It will be overwritten with the template, according to your directions."));
   e.push_back(cf);
   ErrWarn(false,e);
   CopyFile(origc,cf);
  }
  else
  {
   QVector<QString> e;
   e.push_back(tr("File %1 already exists.\n(It will not be overwritten with the template, according to your directions)."));
   e.push_back(cf);
   ErrWarn(false,e);
  }
 }

}
