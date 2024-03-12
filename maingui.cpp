#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include <QApplication>
#include <QCommonStyle>

#include "global.h"

#include "project.h"

#include "renderarea.h"
#include "appwindow.h"
#include "html5document.h"

bool DEB=true;

using namespace std;

int main(int argc, char *argv[])
{
 if ( (argc>3) || ((argc==3) && (QString(argv[1])!="-deb") && (QString(argv[2])!="-deb")) )
 {
  cout << "Usage: " << argv[0] << " [emw_project_file] [-deb]\n";
  exit(1);
 }

 QApplication app(argc,argv);
 app.setQuitOnLastWindowClosed(true);

 QStyle *style = new QCommonStyle();
 app.setStyle(style);
 QString css;
 css  = "QMenuBar { background-color: #909090 }\n";
 css += "QMenuBar::item { spacing: 5px; padding: 1px 4px; background-color: #C0C0C0; border-radius: 4px; }\n";
 css += "QMenuBar::item:selected { background: #A8A8A8; }\n";
 css += "QMenuBar::item:pressed { background: #909090; }\n";
 css += "QLabel { padding: 1px 2px; background-color: #C0C0C0; border-radius: 4px; }";
 app.setStyleSheet(css);

 QString pfn="";

 switch (argc)
 {
  case 1: break;
  case 2: if (QString(argv[1])=="-deb")
           DEB=true;
          else
           pfn=QString(argv[1]);
          break;
  case 3: DEB=true;
          pfn= (QString(argv[1])=="-deb") ? QString(argv[2]) : QString(argv[1]);
          break;
  default: exit(1); break;
 }

 AppWindow MainWin(pfn,&app);

 MainWin.show();

 return app.exec();
}
