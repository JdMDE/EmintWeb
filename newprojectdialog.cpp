/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/

#include "newprojectdialog.h"

extern bool DEB;

NewProjectDialog::NewProjectDialog(QWidget *parent,Project *Pp,QTextEdit *map) : CreationDialog(parent,Pp,map)
{
 QLabel *lHeaderG = new QLabel(tr("Global configuration"));
 lHeaderG->setFrameStyle(FrameStyle);
 QLabel *lHeaderL = new QLabel(tr("Local configuration"));
 lHeaderL->setFrameStyle(FrameStyle);
 QLabel *lHeaderC = new QLabel(tr("Compiler configuration"));
 lHeaderC->setFrameStyle(FrameStyle);

 lName = new QLabel(tr("Project Name: "));
 lName->setFrameStyle(FrameStyle);
 eName = new QLineEdit;
 if (!Pp->IsEmpty())
  eName->setText(Pp->GetName());

 QLabel *lPModeFlag = new QLabel(tr("Use HTTPS: "));
 lPModeFlag->setFrameStyle(FrameStyle);

 cbPModeFlag = new QCheckBox;
 // By default, let's start with HTTP
 if (Pp->IsEmpty())
  cbPModeFlag->setChecked(false);
 else
  if (Pp->GetHTTPSMode())
   cbPModeFlag->setChecked(true);
  else
   cbPModeFlag->setChecked(false);

 QLabel *lPEMFile = new QLabel(tr("PEM file: "));
 lPEMFile->setFrameStyle(FrameStyle);
 ePEMFile = new QLineEdit;
 if (!Pp->IsEmpty())
  ePEMFile->setText(Pp->GetPEMFile());
  
 QLabel *lPEMKey = new QLabel(tr("PEM key file: "));
 lPEMKey->setFrameStyle(FrameStyle);
 ePEMKey = new QLineEdit;
 if (!Pp->IsEmpty())
  ePEMKey->setText(Pp->GetPEMKey());
  
 QLabel *lPEMKeySize = new QLabel(tr("RSA key size: "));
 lPEMKeySize->setFrameStyle(FrameStyle);
 ePEMKeySize = new QLineEdit;
 if (!Pp->IsEmpty())
 {
  stringstream ss;
  ss << Pp->GetPEMKeySize();
  ePEMKeySize->setText(QString::fromStdString(ss.str()));
 } 
 QLabel *lDHFile = new QLabel(tr("DH file: "));
 lDHFile->setFrameStyle(FrameStyle);
 eDHFile = new QLineEdit;
 if (!Pp->IsEmpty())
  eDHFile->setText(Pp->GetDHFile());
  
 lComments = new QLabel(tr("Comments: "));
 lComments->setFrameStyle(FrameStyle);
 eComments = new QPlainTextEdit;
 eComments->setMaximumHeight(10*(eComments->fontMetrics()).xHeight());
 eComments->setMinimumWidth(50*(eComments->fontMetrics()).averageCharWidth());
 if (!Pp->IsEmpty())
  eComments->setPlainText(Pp->GetComments());

 QPushButton *bPDir = new QPushButton(tr("Project directory (absolute path)"));
 QFileDialog fdPDir(this);

 ePDir = new QLineEdit;
 if (!Pp->IsEmpty())
  ePDir->setText(Pp->GetPDir());
 else
  ePDir->setText(HERE);

 QLabel *lHSub = new QLabel(tr("Project HTML subdirectory: "));
 lHSub->setFrameStyle(FrameStyle);
 eHSub = new QLineEdit;
 if (Pp->IsEmpty())
  eHSub->setText("HTMLFiles"+QString(PLATFORM_DIRSLASH));
 else
  eHSub->setText(Pp->GetHTMLDir());

 QLabel *lCSub = new QLabel(tr("Project Code subdirectory: "));
 lCSub->setFrameStyle(FrameStyle);
 eCSub = new QLineEdit;
 if (Pp->IsEmpty())
  eCSub->setText("CodeFiles"+QString(PLATFORM_DIRSLASH));
 else
  eCSub->setText(Pp->GetCodeDir());

 QLabel *lDebFlag = new QLabel(tr("Server debugging flag: "));
 lDebFlag->setFrameStyle(FrameStyle);

 cbDebFlag = new QCheckBox;
 if (Pp->IsEmpty())
  cbDebFlag->setChecked(true);
 else
  if (Pp->GetServerDebFlag())
   cbDebFlag->setChecked(true);
  else
   cbDebFlag->setChecked(false);

 QLabel *lCompiler = new QLabel(tr("C++ compiler: "));
 lCompiler->setFrameStyle(FrameStyle);
 eCompiler = new QLineEdit();
 if (Pp->IsEmpty())
  eCompiler->setText(DEFAULT_CPP_COMPILER);
 else
  eCompiler->setText(Pp->GetCppCompiler());

 QLabel *lCompilerFlags = new QLabel(tr("Compiler flags: "));
 lCompilerFlags->setFrameStyle(FrameStyle);
 eCompilerFlags = new QLineEdit();
 if (Pp->IsEmpty())
  eCompilerFlags->setText(DEFAULT_CPP_FLAGS);
 else
  eCompilerFlags->setText(Pp->GetCompFlagsAsString());
 
 QString expladds="Write a comma-separated list of additional sources; they must be in subdirectory "+Pp->GetCodeDir()+"Transitions";
 QLabel *lExplAddSources = new QLabel(tr(expladds.toStdString().c_str()));
 QLabel *lAddSources = new QLabel(tr("Additional sources"));
 lAddSources->setFrameStyle(FrameStyle);
 eAddSources = new QLineEdit();
 if (!Pp->IsEmpty())
 {
  QStringList asl=Pp->GetAdditionalSources("");
  QString as;
  for (int i=0;i<asl.size()-1;i++)
   as += asl[i]+",";
  as += asl[asl.size()-1]; 
  eAddSources->setText(as);
 }
 else
  eAddSources->setText(CONTEXT_BASENAME+DEFAULT_CPP_EXTENSION);
  
 QPushButton *bAddInclude = new QPushButton("Add include directory");
 QLabel *lAddIncludes = new QLabel(tr("Additional include dirs"));
 lAddIncludes->setFrameStyle(FrameStyle);
 eAddIncludes = new QLineEdit();
 //if (!Pp->IsEmpty())
  eAddIncludes->setText(Pp->GetAdditionalIncludeDirsAsString());
 //else
 // eAddIncludes->setText(QString("-I ")+DEFAULT_LOCAL_INCLUDE_DIR);
  
 QPushButton *bAddLibDir = new QPushButton("Add library directory");
 QLabel *lAddLibDirs = new QLabel(tr("Additional library dirs"));
 lAddLibDirs->setFrameStyle(FrameStyle);
 eAddLibDirs = new QLineEdit();
 //if (!Pp->IsEmpty())
  eAddLibDirs->setText(Pp->GetAdditionalLibDirsAsString());
 //else
 // eAddLibDirs->setText(QString("-L")+DEFAULT_LOCAL_LIB_DIR);
  
 QLabel *lExplAddLibrary = new QLabel(tr("(Write a list of space-sepparated libraries preceding each one by -l)"));
 QLabel *lAddLibraries = new QLabel(tr("Additional libraries"));
 lAddLibraries->setFrameStyle(FrameStyle);
 eAddLibraries = new QLineEdit();
 //if (!Pp->IsEmpty())
  eAddLibraries->setText(Pp->GetAdditionalLibrariesAsString());
 //else
 // eAddLibraries->setText(QString("-l ")+DEFAULT_HT5CPP_LIB);
 
 QGridLayout *layout = new QGridLayout;
 layout->addWidget(lHeaderG,         0, 0, 1, 2, Qt::AlignHCenter);
 layout->addWidget(lName,            1, 0); layout->addWidget(eName,           1, 1);
 layout->addWidget(lPModeFlag,       2, 0); layout->addWidget(cbPModeFlag,     2, 1);
 layout->addWidget(lPEMFile,         3, 0); layout->addWidget(ePEMFile,        3, 1);
 layout->addWidget(lPEMKey,          4, 0); layout->addWidget(ePEMKey,         4, 1);
 layout->addWidget(lPEMKeySize,      5, 0); layout->addWidget(ePEMKeySize,     5, 1);
 layout->addWidget(lDHFile,          6, 0); layout->addWidget(eDHFile,         6, 1);
 layout->addWidget(lComments,        7, 0); layout->addWidget(eComments,       7, 1);

 layout->addWidget(lHeaderL,         8, 0, 1, 2, Qt::AlignHCenter);
 layout->addWidget(lDebFlag,         9, 0); layout->addWidget(cbDebFlag,       9, 1);
 layout->addWidget(bPDir,           10, 0); layout->addWidget(ePDir,          10, 1);
 layout->addWidget(lHSub,           11, 0); layout->addWidget(eHSub,          11, 1);
 layout->addWidget(lCSub,           12, 0); layout->addWidget(eCSub,          12, 1);

 layout->addWidget(lHeaderC,        13, 0, 1, 2, Qt::AlignHCenter);
 layout->addWidget(lCompiler,       14, 0); layout->addWidget(eCompiler,      14, 1);
 layout->addWidget(lCompilerFlags,  15, 0); layout->addWidget(eCompilerFlags, 15, 1);

 layout->addWidget(lAddSources,     16, 0); layout->addWidget(eAddSources,    16, 1);
 layout->addWidget(lExplAddSources, 17, 0, 1, 2, Qt::AlignHCenter);

 layout->addWidget(lAddIncludes,    18, 0); layout->addWidget(eAddIncludes,   18, 1);
 layout->addWidget(bAddInclude,     19, 0, 1, 2, Qt::AlignHCenter);

 layout->addWidget(lAddLibDirs,     20, 0); layout->addWidget(eAddLibDirs,    20, 1);
 layout->addWidget(bAddLibDir,      21, 0, 1, 2, Qt::AlignHCenter);

 layout->addWidget(lAddLibraries,   22, 0); layout->addWidget(eAddLibraries,  22, 1);
 layout->addWidget(lExplAddLibrary, 23, 0, 1, 2, Qt::AlignHCenter);

 layout->addWidget(&bCreate,        24, 0, Qt::AlignHCenter);
 layout->addWidget(&bCancel,        24, 1, Qt::AlignHCenter);

/* Connects:
   Cancel button to do nothing (close dialog)
   Create button to get all the fields and set the project values (what about the empty project?)
   The four add buttons to get the corresponding field and modify the QLineEdit text
*/
 popup.setLayout(layout);
 popup.setWindowTitle(tr("Project creation"));

 connect(bPDir,       SIGNAL(clicked()),this,SLOT(DoChangePDir()));
 connect(bAddInclude, SIGNAL(clicked()),this,SLOT(DoAddInclude()));
 connect(bAddLibDir,  SIGNAL(clicked()),this,SLOT(DoAddLibDir()));

 if (Pp->IsEmpty())
  connect(&bCreate,    SIGNAL(clicked()),this,SLOT(DoProjectCreation()));
 else
  connect(&bCreate,    SIGNAL(clicked()),this,SLOT(DoProjectModification()));
  
 connect(&bCancel,    SIGNAL(clicked()),this,SLOT(DoProjectCancel()));
}

void NewProjectDialog::DoChangePDir()
{
 QString NewPath = QFileDialog::getExistingDirectory(this,
                         tr("Project directory"),ePDir->text());

 if (!NewPath.isEmpty())
 {
  if (!NewPath.endsWith(PLATFORM_DIRSLASH))
   NewPath+=QString(PLATFORM_DIRSLASH);
  ePDir->setText(NewPath);
 }
}

void NewProjectDialog::DoAddInclude()
{
 QString IncludePath = QFileDialog::getExistingDirectory(this,
                         tr("Add include directory"),ePDir->text());

 if (!IncludePath.isEmpty())
 {
  if (!IncludePath.endsWith(PLATFORM_DIRSLASH))
   IncludePath+=PLATFORM_DIRSLASH;
  eAddIncludes->setText(eAddIncludes->text()+" -I "+IncludePath);
 }
}

void NewProjectDialog::DoAddLibDir()
{
 QString LibPath = QFileDialog::getExistingDirectory(this,
                         tr("Add library directory"),ePDir->text());

 if (!LibPath.isEmpty())
 {
  if (!LibPath.endsWith(PLATFORM_DIRSLASH))
   LibPath+=PLATFORM_DIRSLASH;
  eAddLibDirs->setText(eAddLibDirs->text()+" -L "+LibPath);
 }
}

bool NewProjectDialog::AlterProjectContent(QString pfile,bool kill_st_tr)
{
 QString pd=ePDir->text();
 
 // Project directory must be an absolute path (i.e.: begin with /) and, if it does end with /, we add it for convenience
 if (!pd.startsWith(PLATFORM_DIRSLASH))
 {
  QMessageBox::warning(this,tr("Change project dir"),tr("The project dirctory is not an absolute path. Please, change it."),QMessageBox::Ok);
  return false;
 }
 if (!pd.endsWith(PLATFORM_DIRSLASH))
  pd += PLATFORM_DIRSLASH;
 
 if (!QDir(pd).exists())
 {
  int ret=QMessageBox::question(this,tr("Create dir"),tr("Directory %1 does not exist.\nDo you want to create it?").arg(pd),QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
  if (ret==QMessageBox::Yes)
  {
   if (!QDir().mkdir(pd))
   {
    QMessageBox::warning(this,tr("Directory not created"),tr("Directory %1 could not be created. Permissions problem?").arg(pd),QMessageBox::Ok);
    return false;
   }
  }
 } 
 project->Reset(eName->text(),
                cbPModeFlag->isChecked(),
                ePEMFile->text(),
                ePEMKey->text(),
                ePEMKeySize->text(),
                eDHFile->text(),
                pfile,
                pd,
                eHSub->text(),
                eCSub->text(),
                eComments->document()->toPlainText(),
                cbDebFlag->isChecked(),
                eCompiler->text(),
                eCompilerFlags->text(),kill_st_tr);

 QList<QString> v;
 
 v=CSVToList(eAddSources->text());
 v.removeDuplicates();
 project->SetAdditionalSources(v);
 
 v=CSVToList(eAddIncludes->text());
 project->SetAdditionalIncludeDirs(v);
 
 v=CSVToList(eAddLibDirs->text());
 project->SetAdditionalLibDirs(v);
 
 v=CSVToList(eAddLibraries->text());
 project->SetAdditionalLibraries(v);
 
 project->NotEmpty();
 project->Modified();
 
 return true;
}

void NewProjectDialog::DoProjectModification()
{
 if (AlterProjectContent(project->GetPFile(),false))
  popup.done(0);
}

void NewProjectDialog::DoProjectCreation()
{
 QString pfile=eName->text()+".emw";

 if (AlterProjectContent(pfile,true))
  popup.done(0);
}

void NewProjectDialog::DoProjectCancel()
{
 if (DEB)
 {
  cout << "Project creation or edition cancelled.\n";
  cout.flush();
 }
 popup.done(0);
}

