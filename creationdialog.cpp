/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#include "creationdialog.h"

extern bool DEB;

CreationDialog::CreationDialog(QWidget *parent,Project *Pp,QTextEdit *ma) : QWidget(parent)
{
 project=Pp;
 m=ma;

 popup.setModal(true);

 if (Pp->IsEmpty())
  bCreate.setText(tr("Create"));
 else
  bCreate.setText(tr("Modify"));
  
 bCancel.setText(tr("Cancel"));

 FrameStyle = QFrame::Raised | QFrame::StyledPanel;
}

int CreationDialog::Launch()
{
 popup.show();
 return popup.exec();
}

QList<QString> CreationDialog::CSVToList(QString s)
{
 QRegExp r("[\\s\\t,]+");
 QStringList l=s.split(r,Qt::SkipEmptyParts);
 QList<QString> ret;
 for (int i=0;i<l.size();i++)
  if ( (l[i] != QString("")) && (l[i][0] != QChar('-')) )
   ret.push_back(l[i]);
 
 return ret;
}

QString CreationDialog::ListToCSV(QList<QString> l)
{
 QString ret="";
 for (int i=0;i<l.size();i++)
 {
  ret+=l[i];
  if (i!=l.size()-1)
   ret+=",";
 } 
 return ret;
}
