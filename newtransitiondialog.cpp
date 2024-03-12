/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/
#include "newtransitiondialog.h"

extern bool DEB;

NewTransitionDialog::NewTransitionDialog(QWidget *parent,Project *Pp,QTextEdit *ma,QString TName,int px,int py,
                                         QString ISName,QList<QString> ESName,QString FSName) : CreationDialog(parent,Pp,ma)
{
 // This should never happen here, but just in case...
 if (Pp->IsEmpty())
 {
  QMessageBox::warning(parent,APPNAME,tr("Cannot create transition. A non-empty project must exist first"),QMessageBox::Ok);
  return;
 }

 // This is used for both, transition creation and edition. Both uses are differentiated by the fact that, for creation, TName is an empty string
 // whereas for edition contains the transition name.
 if (DEB)
 {
  cout << "Constructor of new transition is to be created at (" << px << "," << py << ")\n";
  if (!TName.isEmpty())
  {
   cout << "Creating new transition from state " << ISName.toStdString() << " to state(s) ";
   for (int i=0;i<ESName.size();i++)
    cout << ESName[i].toStdString() << " ";
   cout << endl;
   cout << "with fallback state " << FSName.toStdString() << endl;
  }
 }

 pWherex=px;
 pWherey=py;

 QLabel *lName = new QLabel(tr("Transition Name "));
 lName->setFrameStyle(FrameStyle);
 eName = new QLineEdit();
 if (!TName.isEmpty())
 {
  eName->setText(TName);
  OrigTransitionName=TName;
 }

 lComments = new QLabel(tr("Comments "));
 lComments->setFrameStyle(FrameStyle);
 eComments = new QPlainTextEdit;
 eComments->setMaximumHeight(10*(eComments->fontMetrics()).xHeight());
 eComments->setMinimumWidth(50*(eComments->fontMetrics()).averageCharWidth());
 if (!TName.isEmpty())
  eComments->setPlainText(Pp->GetTransitionComments(TName));

 /*
 QLabel *lFile = new QLabel(tr("C++ code file "));
 lFile->setFrameStyle(FrameStyle);
 QLabel *lFileName = new QLabel(" ");
 if (!TName.isEmpty())
  lFileName->setText(TName+DEFAULT_CPP_EXTENSION);
 */

 QLabel *lIState = new QLabel(tr("Initial state name: "));
 lIState->setFrameStyle(FrameStyle);

 QLabel *lFallState = new QLabel(tr("Fallback state name: "));
 lFallState->setFrameStyle(FrameStyle);

 mFall = new QComboBox(this);
 mFall->setStyleSheet(CSS_FOR_COMBOBOX);
 QScrollBar *mFallscbar = (mFall->view())->verticalScrollBar();
 mFallscbar->setStyleSheet(CSS_STYLE_FOR_COMBOBOX_VBAR);

 QList<QString> L=Pp->GetStateNameList();
 int sindex=0;
 QString isearch=(TName.isEmpty()) ? ISName : project->GetTransitionFallStateName(TName);
 for (int i=0;i<L.size();i++)
 {
  mFall->addItem(L[i]);
  if (L[i]==isearch)
   sindex=i;
 }
 mFall->setCurrentIndex(sindex);

 QLabel *lFStates = new QLabel(tr("Final state names: "));
 lFStates->setFrameStyle(FrameStyle);

 QString sf;
 if (TName.isEmpty())
  sf=ListToCSV(ESName);
 else
  sf=ListToCSV(project->GetTransitionFinalStateList(TName));
 lFStateNames = new QLabel(sf);
 lFStateNames->setFrameStyle(FrameStyle);

 if (TName.isEmpty())
 {
  // Creation part.
  bCreate.setText(tr("Create"));
  // Initial state comes from canvas (ISName) and is therefore fixed.
  lIStateName = new QLabel(ISName);
  lIStateName->setFrameStyle(FrameStyle);

  QGridLayout *layout = new QGridLayout;

  layout->addWidget(lName,     0, 0); layout->addWidget(eName,           0, 1);
  layout->addWidget(lComments, 1, 0); layout->addWidget(eComments,       1, 1);
  //layout->addWidget(lFile,     2, 0); layout->addWidget(lFileName, 2, 1);
  layout->addWidget(lIState,   2, 0); layout->addWidget(lIStateName,     2, 1);
  layout->addWidget(lFallState,3, 0); layout->addWidget(mFall,           3, 1);
  layout->addWidget(lFStates,  4, 0); layout->addWidget(lFStateNames ,   4, 1);
  layout->addWidget(&bCreate,  5, 0, Qt::AlignHCenter);
  layout->addWidget(&bCancel,  5, 1, Qt::AlignHCenter);

  popup.setLayout(layout);
  popup.setWindowTitle(tr("Transition creation"));
  connect(&bCreate,SIGNAL(clicked()),this,SLOT(DoTransitionCreation()));
 }
 else
 {
  // Edition part.
  bCreate.setText(tr("Modify"));
  // Initial state is to be known from the project.
  mIStateName = new QComboBox(this);
  mIStateName->setStyleSheet(CSS_FOR_COMBOBOX);
  QScrollBar *mIStatescbar = (mIStateName->view())->verticalScrollBar();
  mIStatescbar->setStyleSheet(CSS_STYLE_FOR_COMBOBOX_VBAR);
  
  QList<QString> L=Pp->GetStateNameList();
  int sindex=0;
  for (int i=0;i<L.size();i++)
  {
   mIStateName->addItem(L[i]);
   if (L[i]==project->GetTransitionInitialState(TName))
    sindex=i;
  }
  mIStateName->setCurrentIndex(sindex);

  QLabel *lEditFStates = new QLabel(tr("Add/remove final state..."));
  mEditFStates = new QComboBox(this);
  mEditFStates->setStyleSheet(CSS_FOR_COMBOBOX);
  QScrollBar *mEditFStatesscbar = (mEditFStates->view())->verticalScrollBar();
  mEditFStatesscbar->setStyleSheet(CSS_STYLE_FOR_COMBOBOX_VBAR);
  
  for (int i=0;i<L.size();i++)
   mEditFStates->addItem(L[i]);

  QPushButton *bAdd = new QPushButton(tr("Add"));
  QPushButton *bRemove = new QPushButton(tr("Remove"));

  QGridLayout *layout = new QGridLayout;

  layout->addWidget(lName,        0, 0); layout->addWidget(eName,          0, 1, 1, 2);
  layout->addWidget(lComments,    1, 0); layout->addWidget(eComments,      1, 1, 1, 2);
  //layout->addWidget(lFile,       2, 0); layout->addWidget(lFileName, 2, 1);
  layout->addWidget(lIState,      2, 0); layout->addWidget(mIStateName,    2, 1, 1, 2);
  layout->addWidget(lFallState,   3, 0); layout->addWidget(mFall,          3, 1, 1, 2);
  layout->addWidget(lFStates,     4, 0); layout->addWidget(lFStateNames,   4, 1, 1, 2);
  layout->addWidget(lEditFStates, 5, 0); layout->addWidget(mEditFStates,   5, 1, 1, 2);
  layout->addWidget(bAdd,      6, 1);    layout->addWidget(bRemove,        6, 2);
  layout->addWidget(&bCreate,  7, 0, Qt::AlignHCenter);
  layout->addWidget(&bCancel,  7, 2, Qt::AlignHCenter);

  popup.setLayout(layout);
  popup.setWindowTitle(tr("Transition edition"));

  connect(bAdd,    SIGNAL(clicked()),this,SLOT(DoFinalStateAddition()));
  connect(bRemove, SIGNAL(clicked()),this,SLOT(DoFinalStateRemoval()));
  connect(&bCreate,SIGNAL(clicked()),this,SLOT(DoTransitionEdition()));
 }

 connect(&bCancel,SIGNAL(clicked()),this,SLOT(DoTransitionCancel()));
}

void NewTransitionDialog::DoFinalStateAddition()
{
 QString StoAdd=mEditFStates->currentText();
 QString CSlist=lFStateNames->text();

 if (DEB)
  cout << "Trying to add " << StoAdd.toStdString() << " to list " << CSlist.toStdString() << endl;

 QList<QString> L=CSVToList(CSlist);
 if (L.contains(StoAdd))
 {
  QMessageBox::information(this,tr("Addition state information"),tr("Cannot add state %1; it is already in the list of final states.").arg(StoAdd),QMessageBox::Ok);
  return;
 }
 L.push_back(StoAdd);
 lFStateNames->setText(ListToCSV(L));
}

void NewTransitionDialog::DoFinalStateRemoval()
{
 QString StoRemove=mEditFStates->currentText();
 QString CSlist=lFStateNames->text();

 if (DEB)
  cout << "Trying to remove " << StoRemove.toStdString() << " from list " << CSlist.toStdString() << endl;

 QList<QString> L=CSVToList(CSlist);

 if (!L.contains(StoRemove))
 {
  QMessageBox::information(this,tr("Removal state information"),tr("Cannot remove state %1; it is not currently in the list of final states.").arg(StoRemove),QMessageBox::Ok);
  return;
 }
 if (L.size()==1)
 {
  QMessageBox::information(this,tr("Removal state information"),tr("Cannot remove final state. There must be at least one. Add another first, and then remove this one.").arg(StoRemove),QMessageBox::Ok);
  return;
 }
 L.removeOne(StoRemove);
 lFStateNames->setText(ListToCSV(L));
}

bool NewTransitionDialog::PreviousChecks(QList<QString> &FSNames,QList<StateType> &FSTypes,bool creating)
{
 QString TName=eName->text();
 if (creating && project->TransitionExists(TName))
 {
  QMessageBox::warning(this,tr("Transition creation warning"),tr("You can't create a transition named %1. There is already one transition with that name.\n").arg(TName));
  return false;
 }

 QString FallSName=mFall->currentText();
 if (!project->StateExists(FallSName))
 {
  QMessageBox::warning(this,tr("Transition creation warning"),tr("Something very strange is happening. Fallback state %1 does not exist. Transition will not be created.\n").arg(FallSName));
  return false;
 }
 FSNames=CSVToList(lFStateNames->text());
 FSTypes.clear();
 for (int i=0;i<FSNames.size();i++)
 {
  if (!project->StateExists(FSNames[i]))
  {
   QMessageBox::warning(this,tr("Transition creation warning"),tr("Something very strange is happening. Final state %1 does not exist. Transition will not be created.\n").arg(FSNames[i]));
   return false;
  }
  FSTypes.push_back(project->GetStateType(FSNames[i]));
 }

 return true;
}

void NewTransitionDialog::DoTransitionCreation()
{
 cout << "Calling DoTransitionCreation()\n";

 QList<QString> FSNames;
 QList<StateType> FSTypes;
 QString CFName=eName->text()+DEFAULT_CPP_EXTENSION;
 if (!PreviousChecks(FSNames,FSTypes,true))
  return;

 Transition ntr(eName->text(),lIStateName->text(),FSNames,FSTypes,mFall->currentText(),CFName,eComments->document()->toPlainText());

 bool retval=project->AddTransition(ntr);

 if (DEB)
  cout << "Transition " << eName->text().toStdString() << (retval ? " created.\n" : " not created.\n");

 if (retval)
  project->Modified();

 popup.done( retval ?  1 : 0);
}

void NewTransitionDialog::DoTransitionEdition()
{
 cout << "Calling DoTransitionEdition()\n";

 QList<QString> FSNames;
 QList<StateType> FSTypes;
 QString CFName=eName->text()+DEFAULT_CPP_EXTENSION;
 if (!PreviousChecks(FSNames,FSTypes,false))
  return;

 // Differently from the states, there is no need to do special things if the name of the transition changes.
 // Transitions are not referred by any other structure, except the project itself.
 Transition ntr(eName->text(),lIStateName->text(),FSNames,FSTypes,mFall->currentText(),CFName,eComments->document()->toPlainText());

 bool retval=project->AlterTransition(OrigTransitionName,ntr);

 if (DEB)
  cout << "Transition " << eName->text().toStdString() << (retval ? " changed.\n" : " not changed.\n");

 if (retval)
  project->Modified();

 popup.done( retval ?  1 : 0);
}

void NewTransitionDialog::DoTransitionCancel()
{
 if (DEB)
 {
  cout << "Transition creation cancelled.\n";
  cout.flush();
 }
 popup.done(0);
}
