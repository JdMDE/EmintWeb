/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/
#include "state.h"

State::State()
{
 StateName="";
 HTMLFileName="";
 CfileName="";
 pos.setX(100);
 pos.setY(100);
}

State::State(QString SName,QString SFile,QString co,unsigned px,unsigned py,StateType s)
{
 StateName= ( SName!=FORBIDDEN_STATE_NAME ) ? SName : "Didn't you have a better name?";
 HTMLFileName=CfileName=SFile;
 if ( CfileName.endsWith(".html") )
  CfileName.replace(".html",DEFAULT_CPP_EXTENSION);
 else
  CfileName += DEFAULT_CPP_EXTENSION;
 pos.setX(px);
 pos.setY(py);
 Comments=co;
 Stype=s;
}

void State::SetHTMLFile(QString file)
{
 HTMLFileName=CfileName=file;
 if ( CfileName.endsWith(".html") )
  CfileName.replace(".html",DEFAULT_CPP_EXTENSION);
 else
  CfileName += DEFAULT_CPP_EXTENSION;
}

State::State(QDomNode node)
{
 for (QDomNode secnode = node.firstChild(); !secnode.isNull(); secnode = secnode.nextSibling())
 {
  QDomElement el = secnode.toElement();
  //cout << el.tagName().toStdString() << " --> " << el.text().toStdString() << endl;

  if (el.tagName()=="StateName")
   StateName=el.text();
  if (el.tagName()=="HTMLFile")
  {
   HTMLFileName=el.text();
   CfileName=HTMLFileName;
   if ( CfileName.endsWith(".html") )
    CfileName.replace(".html",DEFAULT_CPP_EXTENSION);
   else
    CfileName += DEFAULT_CPP_EXTENSION;
  }
  if (el.tagName()=="StateComments")
   Comments=el.text();
  if (el.tagName()=="StatePosition")
  {
   QDomNode ternode=secnode.firstChild();
   QString tag=ternode.toElement().tagName();
   int d1=ternode.toElement().text().toInt();
   ternode=ternode.nextSibling();
   int d2=ternode.toElement().text().toInt();
   ternode=ternode.nextSibling();
   if (tag=="y")
   {
    int dummy=d1;
    d1=d2;
    d2=dummy;
   }
   pos.setX(d1);
   pos.setY(d2);
  }
  if (el.tagName()=="StateType")
  {
   if (el.text()=="Initial")
    Stype=Initial;
   if (el.text()=="Terminal")
    Stype=Terminal;
   if (el.text()=="Intermediate")
    Stype=Intermediate;
  }
 }
}

void State::SaveToXML(QTextStream &out)
{
 out << "  <State>\n";
 out << "   <StateName>" << StateName    << "</StateName>\n";
 out << "   <HTMLFile>"  << HTMLFileName << "</HTMLFile>\n";
 out << "   <StatePosition>\n";
 out << "    <x>" << pos.x() << "</x>\n";
 out << "    <y>" << pos.y() << "</y>\n";
 out << "   </StatePosition>\n";
 out << "   <StateType>";
 switch (Stype)
 {
  case Initial: out << "Initial"; break;
  case Terminal: out << "Terminal"; break;
  case Intermediate: out << "Intermediate"; break;
  case NO_TYPE: out << "NO_TYPE"; break;
 }
 out << "</StateType>\n";
 if (!Comments.isEmpty())
  out << "   <StateComments>" << Comments << "</StateComments>\n";
 out << "  </State>\n";
}
