/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/


#ifndef CREATIONDIALOG_H
#define CREATIONDIALOG_H

#include <QVector>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QMenuBar>
#include <QMessageBox>

#include "project.h"

class CreationDialog : public QWidget
{
 Q_OBJECT

 public:
  explicit CreationDialog(QWidget *parent = 0,Project *Pp=NULL,QTextEdit *map=NULL);
  int Launch();

 protected:
  QDialog popup;

  // All the creation dialogs have in common two fields, name and comments, and two buttons: create (occasionally, modify) and cancel.
  QPushButton     bCreate;
  QPushButton     bCancel;
  QLabel          *lName;
  QLineEdit       *eName;
  QLabel          *lComments;
  QPlainTextEdit  *eComments;

  int FrameStyle;

  // The dialogs need also access to the project structure and to the global message area
  Project *project;
  QTextEdit *m;

  QList<QString> CSVToList(QString s);
  QString ListToCSV(QList<QString> l);
};

#endif // CREATIONDIALOG_H
