#include "global.h"

bool FileAccessible(QString fname)
{
 ifstream f(fname.toStdString().c_str());
 if (!f.is_open())
  return false;
 f.close();
 return true;
}
