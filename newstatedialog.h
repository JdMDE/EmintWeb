/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#ifndef NEWSTATEDIALOG_H
#define NEWSTATEDIALOG_H

#include <QRadioButton>
#include <QTextEdit>
#include "creationdialog.h"

class NewStateDialog : public CreationDialog
{
 Q_OBJECT

 public:
  explicit NewStateDialog(QWidget *parent = 0,Project *Pp=NULL,QTextEdit *ma=NULL,QString SName="",int px=0,int py=0);
 public slots:
  void DoStateCreation();
  void DoStateCancel();
  void DoStateEdition();
  //void SetSType(StateType s) { t=s; };

 private:
  QLineEdit *eName;
  QLineEdit *eFile;
  QRadioButton *sttype_buttons[3];
  QTextEdit *m;
  int pWherex;
  int pWherey;
  QString OrigStateName;
  StateType OrigStateType;
};

#endif // NEWSTATEDIALOG_H
