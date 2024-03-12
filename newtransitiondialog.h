/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/


#ifndef NEWTRANSITIONDIALOG_H
#define NEWTRANSITIONDIALOG_H

#include "cssstyles.h"

//#include <QRadioButton>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QComboBox>
#include "creationdialog.h"

class NewTransitionDialog : public CreationDialog
{
 Q_OBJECT
 public:
  explicit NewTransitionDialog(QWidget *parent=0,Project *Pp=NULL,QTextEdit *ma=NULL,QString TName="",int px=0,int py=0,
                                         QString ISName="",QList<QString> ESName=QList<QString>(),QString FSName="");
 public slots:
  void DoTransitionCreation();
  void DoTransitionCancel();
  void DoTransitionEdition();
  void DoFinalStateAddition();
  void DoFinalStateRemoval();

 private:
  int pWherex;
  int pWherey;

  QLabel *lIStateName;
  QComboBox *mIStateName;

  QComboBox *mFall;
  QLabel *lFStateNames;

  QComboBox *mEditFStates;

  QString OrigTransitionName;

  bool PreviousChecks(QList<QString> &FSNames,QList<StateType> &FSTypes,bool creating);
};

#endif // NEWTRANSITIONDIALOG_H
