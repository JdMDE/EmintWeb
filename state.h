/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#ifndef STATE_H
#define STATE_H

#include "global.h"

#include <iostream>
#include <fstream>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QPoint>
#include <QDomNode>

#define FORBIDDEN_STATE_NAME "CURSOR_IS_OUT"

class State
{
 public:
  State();
  State(QString name,QString file,QString co,unsigned px,unsigned py,StateType s);
  State(QDomNode node);

  QString GetName()                  { return StateName; };
  void SetName(QString newname)      { StateName = (newname!=FORBIDDEN_STATE_NAME) ? newname : "Didn't you have a better name?"; };

  QString GetHTMLFile()              { return HTMLFileName; };
  void SetHTMLFile(QString name);

  QString GetCodeFile()              { return CfileName; };
  QString GetHeaderFile()            { QString ret=CfileName; ret.replace(DEFAULT_CPP_EXTENSION,DEFAULT_HEADER_EXTENSION); return ret; };
  void SetCodeFile(QString name)     { CfileName=name; };

  QPoint GetPos()                    { return pos; };
  void SetPos(QPoint p)              { pos=p; };
  void SetPos(unsigned x,unsigned y) { pos.setX(x); pos.setY(y); };

  QString GetComments()              { return Comments; };
  void SetComments(QString c)        { Comments=c; };

  StateType GetStype()               { return Stype; };
  void SetStype( StateType s )       { Stype=s; };

  void SaveToXML(QTextStream &out);

  QPoint GetStatePosition(int numstate);

 private:
  QString StateName;
  QString HTMLFileName;
  QString CfileName;
  QPoint  pos;
  QString Comments;
  StateType Stype;
 };

#endif // STATE_H
