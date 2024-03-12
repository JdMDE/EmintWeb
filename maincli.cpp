#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include <QtCore/QCoreApplication>
#include "global.h"
#include "project.h"

bool DEB=false;

using namespace std;

void Usage(char *pname)
{
 cerr << "Usage:\n\n   " << pname << " emw_project_file [-deb] [-gsc stname] [-gtc trname] [-gasc] [-gatc] [-gmain] [-gcont] [-gmake] [-gall] [-ov]\n\n";
 cerr << "where:\n";
 cerr << "   -deb   Generate the project with debug flag on\n";
 cerr << "   -gsc   Generate the source code for the state named stname\n";
 cerr << "   -gtc   Generate the source code for the transition named trname\n";
 cerr << "   -gasc  Generate the source code for all states\n";
 cerr << "   -gatc  Generate the source code for all transitions\n";
 cerr << "   -gmain Generate the source code for the main program (single server and metaserver)\n";
 cerr << "   -gcont Generate the source code for the context\n";
 cerr << "   -gmake Generate the Makefile\n";
 cerr << "   -gall  Generate all the files needed to build and compile the project (i.e.: all the former ones)\n";
 cerr << "          without overwriting neither the existing transition code files nor the context code file.\n";
 cerr << "   -ov    If this flag is set, the requested files WILL BE OVERWRITTEN with the default version.\n";
 cerr << "          WARNING: this includes the code for the transitions AND the context.\n";
 cerr << "          It is recommended that you keep a copy.\n\n";
 cerr << "The project file is compulsory and MUST be the first argument, with .emw extension.\n";
 cerr << "If you pass a name like name.emw the file name_localconf.emw must exist in the same directory.\n";
 exit(1);
}

int main(int argc, char *argv[])
{
 if (argc<3)
  Usage(argv[0]);

 QCoreApplication app(argc,argv);

 QString pfn=QString(argv[1]);
 Project P(pfn,false);

 QList<QString> Ls=P.GetStateNameList();
 QVector<bool> Lsg(Ls.size());
 for (int i=0;i<Lsg.size();i++)
  Lsg[i]=false;
 QList<QString> Lt=P.GetTransitionNameList();
 QVector<bool> Ltg(Lt.size());
 for (int i=0;i<Ltg.size();i++)
  Ltg[i]=false;

 bool GenContext=false;
 bool GenMain=false;
 bool GenMake=false;
 bool Overwrite=false;

 for (int i=2;i<argc;i++)
 {
  if (QString(argv[i])=="-deb")
  {
   DEB=true;
   continue;
  }

  if (QString(argv[i])=="-ov")
  {
   Overwrite=true;
   continue;
  }

  if (QString(argv[i])=="-gsc")
  {
   if (argc<i+2)
    Usage(argv[0]);
   for (int j=0;j<Ls.size();j++)
    if (Ls[j]==QString(argv[i+1]))
    {
     Lsg[j]=true;
     i++;
    }
   continue;
  }

  if (QString(argv[i])=="-gtc")
  {
   if (argc<i+2)
    Usage(argv[0]);
   for (int j=0;j<Lt.size();j++)
    if (Lt[j]==QString(argv[i+1]))
    {
     Ltg[j]=true;
     i++;
    }
   continue;
  }

  if (QString(argv[i])=="-gasc")
  {
   for (int j=0;j<Ls.size();j++)
    Lsg[j]=true;
   continue;
  }

  if (QString(argv[i])=="-gatc")
  {
   for (int j=0;j<Lt.size();j++)
    Ltg[j]=true;
   continue;
  }

  if (QString(argv[i])=="-gmain")
  {
   GenMain=true;
   continue;
  }

  if (QString(argv[i])=="-gcont")
  {
   GenContext=true;
   continue;
  }

  if (QString(argv[i])=="-gmake")
  {
   GenMake=true;
   continue;
  }

  if (QString(argv[i])=="-gall")
  {
   GenMain=true;
   GenContext=true;
   GenMake=true;
   for (int j=0;j<Ls.size();j++)
    Lsg[j]=true;
   for (int j=0;j<Lt.size();j++)
    Ltg[j]=true;
   continue;
  }

  cerr << "Invalid argument: '" << argv[i] << "'\n";

  Usage(argv[0]);
 }

 for (int i=0;i<Ls.size();i++)
  if (Lsg[i])
   P.GenCodeForState(Ls[i],false);

 for (int i=0;i<Lt.size();i++)
  if (Ltg[i])
   P.GenCodeForTransition(Lt[i]);

 if (GenContext)
  P.GenContext(Overwrite);

 if (GenMain)
  P.GenMain();

 if (GenMake)
  P.GenMake();
}
