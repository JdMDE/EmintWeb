/**********************************************************
 *                                                        *
 * @ Juan Domingo, 2023                                   *
 *                                                        *
 * This program is under GNU license                      *
 *                                                        *
 **********************************************************/


#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QCheckBox>
#include <QRadioButton>
#include <QFileDialog>
#include "creationdialog.h"

class NewProjectDialog : public CreationDialog
{
 Q_OBJECT

 public:
  explicit NewProjectDialog(QWidget *parent = 0,Project *Pp = NULL,QTextEdit *map=NULL);

 private slots:
  void DoProjectCancel();
  void DoProjectCreation();
  void DoProjectModification();
  void DoChangePDir();
  void DoAddInclude();
  void DoAddLibDir();

 private:
  // Section of global configuration
  // Project name is into the edit field eName of the superclass.
  // Project mode
  QCheckBox *cbPModeFlag;
  // PEM file
  QLineEdit *ePEMFile;
  // PEM Key
  QLineEdit *ePEMKey;
  // PEM RSA key size
  QLineEdit *ePEMKeySize;
  // DH file
  QLineEdit *eDHFile;
  // Project file name is not defined here. It is the name of the file from which we read, or the project name + .emw for a new project.
  //QLineEdit *ePFile;
  // Comments is into the edit field eComments of the superclass
  QCheckBox *cbDebFlag;

  // Section of local configuration
  // Project directory
  QLineEdit *ePDir;
  // Project HTML subdirectory
  QLineEdit *eHSub;
  // Project code subdirectory
  QLineEdit *eCSub;

  // Section of compiler configuration
  // Compiler
  QLineEdit *eCompiler;
  // Compiler flags
  QLineEdit *eCompilerFlags;
  // Additional sources
  QLineEdit *eAddSources;
  // Additional headers
  QLineEdit *eAddIncludes;
  // Additional library directories
  QLineEdit *eAddLibDirs;
  // Additional libraries
  QLineEdit *eAddLibraries;
  
  void AlterProjectContent(QString pfile,bool kill_st_tr);
};

#endif // NEWPROJECTDIALOG_H
