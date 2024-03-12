/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#ifndef TRANSITION_H
#define TRANSITION_H

#include <QList>
#include <QStringList>
#include <QTextStream>
#include <QVector>

#include "global.h"
#include "state.h"

#define FORBIDDEN_TRANSITION_NAME "CURSOR_IS_OUT"

class Transition
{
 public:
  Transition();
  Transition(QString TName,QString OState,QList<QString> EStates,QList<StateType> StTypes,QString FallSt,QString CfName,QString Co);
  Transition(QDomNode node);

  QString GetName()                    { return TransitionName; };
  void SetName(QString newname)        { TransitionName = (newname!=FORBIDDEN_TRANSITION_NAME) ? newname : "Didn't you have a better name?"; };
  QString GetOriginState()             { return OrigState; };
  void SetOriginState(QString newname) { OrigState=newname; };

  QList<QString> GetFinalStates()                                     { return(EndStates); };
  void SetFinalStates( QList<QString> newf, QList<StateType> newt ) { EndStates=newf; EndStateTypes=newt; };

  QString GetFallState()               { return(FallbackState); };
  void SetFallState(QString stname)    { FallbackState=stname; };

  QList<StateType> GetFinalStTypes()             { return EndStateTypes; };
  void SetFinalStTypes(QList<StateType> newtype) { EndStateTypes=newtype; };

  QString GetNameCodeFile()       { return(CfileName); };
  void SetNameCodeFile(QString n) { CfileName=n; };

  QString GetComments()       { return(Comments); };
  void SetComments(QString c) { Comments=c; };

  QPoint GetPos()       { return Pos; };
  void SetPos(QPoint p) { Pos=p; };
  bool OnlyDestination( QString sname )   { return (EndStates.size()==1 && EndStates[0]==sname); };
  void DelStateFromDestination( QString sname );

  void SaveToXML(QTextStream &out);
  bool Check(QVector<State> &l);

 private:
  QString TransitionName;
  QString OrigState;
  QList<QString> EndStates;
  QList<StateType> EndStateTypes;
  QString FallbackState;
  QString CfileName;
  QString Comments;
  QPoint Pos;
};

#endif // TRANSITION_H
