#include "context.h"

// Fill/change this functions if you need to, but keep the prototypes

bool Context::SetArgs(int argc,char *argv[])
{
 if (argc>0) 
 {
  cout << "Arguments passed to Context as program arguments:" << endl;
  for (int i=0;i<argc;i++)
   cout << "Argument " << i << ": " << argv[i] << endl; 
  // Supress the former lines and do here whatever you need with the passed arguments.
  // Particularly, you should check that the number of arguments is correct and exit with an error if not,
  // including the case in which you do not expect to receive any argument (but there might be one or more, incorrectly).
 }
 return true;
} 

bool Context::InitializeContext()
{
 // This function is provided empty for you to add here whatever initialization tasks you need 
 // after private variables of the context have been filled from the values of the program arguments
 // or in any other way. If you don't have anything to do, just leave it as is (but don't delete it).
 return true;
}

string Context::RequestedFile(string &URI,string &MimeType,size_t &filelength)
{
 // Do here whatever you need according to the requested file.
 // You can serve it, or not. Don't forget to fill the filelength and MimeType fields.
 // You can use statements like
 // if (URI=="a valid URI requesting a valid file...")
 // {
 //  string file = (extract it from the URI...)
 //  ifstream g(URI.c_str()); 
 //  if (!g.is_open())
 //   return "";
 //  g.seekg(0,ios::end);
 //  filelength=g.tellg();
 //  g.close();
 //  MimeType= (the mime type of the file, as a string)
 //  return file;
 // }
 // else
 //  return "";

 // Just to avoid a warning in the initially created file:
 return "";
}

bool Context::InvalidChars(string &s,char subs)
{
 ostream &errst = (outlet_type==OUT_TO_COUT ? cout : ((outlet_type==OUT_TO_CERR) ? cerr : outlet_ofstream));
 bool invalid=false;
 size_t i=0;
 while (i<s.size() && s[i]!='\0')
 {
  if ( !( iswalpha(s[i]) || isdigit(s[i]) || (s[i]=='_') || (s[i]=='.') || (s[i]=='/') || (s[i]=='=') || (s[i]=='+') || (s[i]=='-') || (s[i]=='%') || (s[i]=='?') || (s[i]=='&') ) )
  {
   errst << "Character " << i << " (" << s[i] << ") substituted by " << subs << "\n";
   s[i]=subs;
   invalid=true;
  }
  i++;
 }
 return invalid;
}

string Context::ReadableError()
{
 if (last_error==NO_ERROR)
  return("There were no error!");
 
 string reterror="";
 if (last_error & URL_TOO_LONG)         reterror += "|URL too long ";
 if (last_error & FORBIDDEN_CHAR)       reterror += "|Forbidden character in URL ";
 if (last_error & TOO_FEW_VARIABLES)    reterror += "|Too few variables passed after URL ";
 if (last_error & TOO_MANY_VARIABLES)   reterror += "|Too many variables passed after URL ";
 if (last_error & UNKNOWN_VARIABLE)     reterror += "|Unknown variable passed after URL ";
 if (last_error & UNEXPECTED_VAR_VALUE) reterror += "|Unexpected variable value passed after URL ";
 if (last_error & POCO_IO_EXCEPTION)    reterror += "|Poco library I/O exception ";
 if (last_error & SQL_ERROR)            reterror += "|SQL error. Activate debug in context for more details ";
 if (reterror == "") reterror = "Unknown error";
 
 return reterror;
}

void Context::EmitOVars(vector<pair<string,string>> &ov,unsigned long numerr,string fun_name)
{
 if (SERVERDEB)
 {
  ostream &errst = (outlet_type==OUT_TO_COUT ? cout : ((outlet_type==OUT_TO_CERR) ? cerr : outlet_ofstream));
  errst << "Thread in port " << GetPort() << ": function " << fun_name << " emits this error(s): " << ReadableError() << "|\n";;
  errst << "    " << ov.size() << " pairs variable/value which is/are ";
  for (unsigned i=0;i<ov.size();i++)
   errst << "(" << ov[i].first << "," << ov[i].second << ") ";
  errst << "\n";
  errst.flush();
 }
}

void Context::ModifyPage(string &page,unsigned long error)
{
 if (error==NO_ERROR)
  return;

 size_t p=page.find("</body>");
 if (p==string::npos)
  return;

 string errorhead="<p/><p/><div align=\"center\">\n<table border=\"1\">\n";
 errorhead+="<th><font color=\"#cc0000\">WARNING:</font> you have arrived to this state because of some error(s).</th>\n";
 errorhead+="<tr><td>The error(s) are:</td></tr>\n";
 string toolong="<tr><td>URL too long. Have you added characters to the URL sent by the application?</td></tr>\n";
 string forbchar="<tr><td>Forbidden characters in the URL sent by the application. Please, don't introduce non-ASCII characters in the requested fields; they have been substituted.</td></tr>\n";
 string toofew="<tr><td>Too few variables have arrived. Have you deleted part of the URL sent by the application?</td></tr>\n";
 string toomany="<tr><td>Too many variables have arrived. Have you added any variable to the URL sent by the application?</td></tr>\n";
 string unknown="<tr><td>Unknonw variable(s) in the received URL. Have you altered manually the string sent by your browser?</td></tr>\n";
 string unexpected="<tr><td>Unexpected value for one or more of the variables. Have you left any field of the forms empty?</td></tr>\n";
 string pocoioerror="<tr><td>Input/ouput exception error</td></tr>\n";
 string errortail="</table>\n</div>";

 string newpage=page.substr(0,p);
 newpage+=errorhead;
 if (error & URL_TOO_LONG)
  newpage+=toolong;
 if (error & FORBIDDEN_CHAR)
  newpage+=forbchar;
 if (error & TOO_FEW_VARIABLES)
  newpage+=toofew;
 if (error & TOO_MANY_VARIABLES)
  newpage+=toomany;
 if (error & UNKNOWN_VARIABLE)
  newpage+=unknown;
 if (error & UNEXPECTED_VAR_VALUE)
  newpage+=unexpected;
 if (error & POCO_IO_EXCEPTION)
  newpage+=pocoioerror;

 string dberror="<tr><td>Error from the database system. May be a duplicate primary key, incorrect value type or similar.\n";
 if (error & SQL_ERROR)
  newpage+=dberror;

 newpage+=errortail;

 newpage+=page.substr(p);
  
 page=newpage;
}

void Context::SetOutletType(unsigned char outtype)
{
 switch (outtype)
 {
  case OUT_TO_COUT:
  case OUT_TO_CERR: outlet_type=outtype; break;
  case OUT_TO_FILE:
  {
   if (port==0)
   {
    cerr << "Error in Context::SetOutletType(OUT_TO_FILE): port has not been set. Output will go to cout.\n";
    outlet_type=OUT_TO_COUT;
   }
   else
   {
    char dname[40];
    sprintf(dname,"%s/%s%05d.txt",DEBFILE_DIRECTORY,DEBFILE_ROOTNAME,port);
    outlet_ofstream.open(dname);
    if (!outlet_ofstream.is_open())
    {
     cerr << "Error in Context::SetOutletType(OUT_TO_FILE): file " << dname << " cannot be opened. Output will go to cout.\n";
     outlet_type=OUT_TO_COUT;
    }
    else
     outlet_type=OUT_TO_FILE;
   }
  }
  break;
  default: 
  {
   cerr << "Error: invalid argument to Context::SetOutletType. Output will go to cout.\n";
   outlet_type=OUT_TO_COUT;
  }
  break;
 }
}

ostream& Context::GetOutletStream()
{
 if (outlet_type==OUT_TO_COUT)
  return cout;
 if (outlet_type==OUT_TO_CERR)
  return cerr;
 return outlet_ofstream; 
}

// ... to here

Context::Context()
{
 // Add here all the initializations of your private fields, dynamic memory booking, etc.

 indentstring=HTML_INDENT_STRING;
 ResetError();
 SetPort(0);
}

Context::~Context()
{
 // Add here all the cleaning, dynamic memory releasing, etc.
}

// Your turn: implement here the public and private methods of the Context class.

