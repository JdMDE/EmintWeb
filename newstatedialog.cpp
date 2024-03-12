/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#include "newstatedialog.h"

extern bool DEB;

NewStateDialog::NewStateDialog(QWidget *parent,Project *Pp,QTextEdit *ma,QString SName,int px,int py) : CreationDialog(parent,Pp,ma)
{
 if (DEB)
  cout << "Constructor of new state is to be created at (" << px << "," << py << ")\n";

 pWherex=px;
 pWherey=py;

 if (Pp->IsEmpty())
 {
  QMessageBox::warning(parent,APPNAME,tr("Cannot create state. A non-empty project must exist first"),QMessageBox::Ok);
  return;
 }

 QLabel    *lName = new QLabel(tr("State Name: "));
 lName->setFrameStyle(FrameStyle);
 eName = new QLineEdit();
 if (!SName.isEmpty())
 {
  eName->setText(SName);
  OrigStateName=SName;
  OrigStateType=project->GetStateType(SName);
 }

 QLabel    *lFile = new QLabel(tr("HTML file: "));
 lFile->setFrameStyle(FrameStyle);
 eFile = new QLineEdit();
 if (!SName.isEmpty())
  eFile->setText(Pp->GetStateHTMLFile(SName));

 lComments = new QLabel(tr("Comments: "));
 lComments->setFrameStyle(FrameStyle);
 eComments = new QPlainTextEdit;
 eComments->setMaximumHeight(10*(eComments->fontMetrics()).xHeight());
 eComments->setMinimumWidth(50*(eComments->fontMetrics()).averageCharWidth());
 if (!SName.isEmpty())
  eComments->setPlainText(Pp->GetStateComments(SName));

 sttype_buttons[0] = new QRadioButton("Intermediate state",this);
 sttype_buttons[1] = new QRadioButton("Terminal state",this);
 sttype_buttons[2] = new QRadioButton("Initial state",this);

 if (SName.isEmpty())
  sttype_buttons[0]->setChecked(true);
 else
  switch (Pp->GetStateType(SName))
  {
   case Initial: sttype_buttons[2]->setChecked(true); break;
   case Terminal: sttype_buttons[1]->setChecked(true); break;
   case NO_TYPE:
   case Intermediate: sttype_buttons[0]->setChecked(true); break;
  }

 QGridLayout *layout = new QGridLayout;
 layout->addWidget(lName,     0, 0); layout->addWidget(eName,     0, 1);
 layout->addWidget(lFile,     1, 0); layout->addWidget(eFile,     1, 1);
 layout->addWidget(lComments, 2, 0); layout->addWidget(eComments, 2, 1);
 layout->addWidget(sttype_buttons[0], 3, 0);
 layout->addWidget(sttype_buttons[1], 4, 0);
 layout->addWidget(sttype_buttons[2], 5, 0);
 layout->addWidget(&bCreate, 6, 0, Qt::AlignHCenter);
 layout->addWidget(&bCancel, 6, 1, Qt::AlignHCenter);

 popup.setLayout(layout);
 if (SName.isEmpty())
  popup.setWindowTitle(tr("State creation"));
 else
  popup.setWindowTitle(tr("State edition"));

 if (SName.isEmpty())
  connect(&bCreate,SIGNAL(clicked()),this,SLOT(DoStateCreation()));
 else
  connect(&bCreate,SIGNAL(clicked()),this,SLOT(DoStateEdition()));

 connect(&bCancel,SIGNAL(clicked()),this,SLOT(DoStateCancel()));
}

void NewStateDialog::DoStateCreation()
{
 StateType t=NO_TYPE;
 if (sttype_buttons[0]->isChecked())
  t=Intermediate;
 if (sttype_buttons[1]->isChecked())
  t=Terminal;
 if (sttype_buttons[2]->isChecked())
  t=Initial;

 State st(eName->text(),eFile->text(),eComments->document()->toPlainText(),pWherex,pWherey,t);

 bool retval=project->AddState(st);

 if (DEB)
  cout << "State " << eName->text().toStdString() << (retval ? " created.\n" : " not created.\n");

 if (retval)
  project->Modified();

 popup.done( retval ?  1 : 0);
}

void NewStateDialog::DoStateEdition()
{
 QString SName=eName->text();
 StateType t=NO_TYPE;
 if (sttype_buttons[0]->isChecked())
  t=Intermediate;
 if (sttype_buttons[1]->isChecked())
  t=Terminal;
 if (sttype_buttons[2]->isChecked())
  t=Initial;

 cout << "Editing state " << SName.toStdString() << endl;

 if ((t!=OrigStateType) && (t==Terminal) && project->ThereIsOneTransDepartingFrom(OrigStateName) )
 {
  QMessageBox::warning(this, APPNAME,
                             tr("State cannot be made Terminal, since a transition departs from it.\nErase first such transition."),
                             QMessageBox::Ok);
  return;
 }

 if ((t!=OrigStateType) && (t==Initial))
 {
  if ( project->ThereIsOneTransArrivingTo(OrigStateName) )
  {
   QMessageBox::warning(this, APPNAME,
                              tr("State cannot be made Initial, since a transition arrives to it.\nErase first such transition."),
                              QMessageBox::Ok);
   return;
  }
  QString OldInitial=project->GetInitState();
  QMessageBox::warning(this, APPNAME,
                             tr("If this state is the initial one, state %1 will not be such anymore. State %1 changed to Intermediate.").arg(OldInitial),
                             QMessageBox::Ok);
  project->SetStateType(OldInitial,Intermediate);
 }

 QPoint oldpos=project->GetStatePosition(OrigStateName);

 State st(eName->text(),eFile->text(),eComments->document()->toPlainText(),oldpos.x(),oldpos.y(),t);

 bool retval=project->AlterState(OrigStateName,st);

 if (DEB)
  cout << "State " << eName->text().toStdString() << (retval ? " modified.\n" : " not modified.\n");

 if (retval)
  project->Modified();

 popup.done( retval ?  1 : 0);
}

void NewStateDialog::DoStateCancel()
{
 if (DEB)
 {
  cout << "State creation cancelled.\n";
  cout.flush();
 }

 popup.done(0);
}
