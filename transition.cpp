/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#include "transition.h"

extern bool DEB;

Transition::Transition()
{
 OrigState="";
 EndStates.clear();
 EndStateTypes.clear();
 CfileName="";
 Comments="";
 Pos=QPoint(0,0);
}

Transition::Transition(QString TName,QString OState,QList<QString> EStates,QList<StateType> StTypes,QString FallSt,QString CfName,QString Co)
{
 if (DEB)
 {
  cout << "Into transition constructor.\n";
  cout.flush();
 }
 TransitionName = (TName != FORBIDDEN_TRANSITION_NAME) ? TName : "Didn't you have a better name?";
 OrigState = OState;
 EndStates  = EStates;
 EndStateTypes = StTypes;
 FallbackState = FallSt;
 CfileName = CfName;
 Comments = Co;
 Pos.setX(0);
 Pos.setY(0);
}

Transition::Transition(QDomNode node)
{
 Pos.setX(0);
 Pos.setY(0);
 for (QDomNode secnode = node.firstChild(); !secnode.isNull(); secnode = secnode.nextSibling())
 {
  QDomElement el = secnode.toElement();
  //cout << el.tagName().toStdString() << " --> " << secnode.nodeValue().toStdString() << endl;
  if (el.tagName()=="TransitionName")
   TransitionName=el.text();
  if (el.tagName()=="InitialStateName")
   OrigState=el.text();
  if (el.tagName()=="FallbackStateName")
   FallbackState=el.text();
  if (el.tagName()=="FinalStateNames")
  {
   for (QDomNode fnode = secnode.firstChild(); !fnode.isNull(); fnode = fnode.nextSibling())
    EndStates.push_back(fnode.toElement().text());
  }
  if (el.tagName()=="TransitionPosition")
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
   Pos.setX(d1);
   Pos.setY(d2);
  }
  if (el.tagName()=="CodeFile")
   CfileName=el.text();
  if (el.tagName()=="TransitionComments")
   Comments=el.text();
 }
}

void Transition::DelStateFromDestination(QString sname)
{
 int i=0;
 QList<QString>::iterator it=EndStates.begin();
 QList<StateType>::iterator itt=EndStateTypes.begin();
 while ( it != EndStates.end() )
 {
  if (EndStates[i]==sname)
  {
   EndStates.erase(it);
   EndStateTypes.erase(itt);
   return;
  }
  ++it;
  ++itt;
 }
}

bool Transition::Check(QVector<State> &l)
{
 int numfound=0;

 int i=0;
 while (i<l.size() && OrigState != l[i].GetName())
  i++;
 if (i<l.size())
  numfound++;

 for (int k=0;k<EndStates.size();k++)
 {
  int k1=0;
  while (k1<l.size() && EndStates[k] != l[k1].GetName())
   k1++;
  if (k1<l.size())
   numfound++;
 }

 if (DEB)
  cout << "from " << OrigState.toStdString() << " to (amongst others) " << EndStates[0].toStdString() << endl;

 return ( numfound==(EndStates.size()+1) );
}

void Transition::SaveToXML(QTextStream &out)
{
 out << "  <Transition>\n";
 out << "   <TransitionName>" << TransitionName << "</TransitionName>\n";
 out << "   <InitialStateName>" << OrigState         << "</InitialStateName>\n";
 out << "   <FallbackStateName>" << FallbackState  << "</FallbackStateName>\n";
 out << "   <FinalStateNames>\n";
 for (int k=0;k<EndStates.size();k++)
  out << "    <StateName>" << EndStates[k] << "</StateName>\n";
 out << "   </FinalStateNames>\n";
 out << "   <TransitionPosition>\n";
 out << "    <x>" << Pos.x() << "</x>\n";
 out << "    <y>" << Pos.y() << "</y>\n";
 out << "   </TransitionPosition>\n";
 out << "   <CodeFile>"         << CfileName << "</CodeFile>\n";
 if (!Comments.isEmpty())
  out << "   <TransitionComments>"        << Comments     << "</TransitionComments>\n";
 out << "  </Transition>\n";
}

