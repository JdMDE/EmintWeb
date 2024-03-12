#include "hdoc.h"

extern bool DEB;

HDoc::HDoc(string hfile,string stn,string hn,bool gt)
{
 stname=stn;
 hname=hn;
 gentest=gt;
 prototype.clear();
 out=NULL;
 indent_unit=" ";
 rootvar_name="mvar";
 HeaderGenerated=false;
 CCode=HCode=HV=Call=Vars="";
 // doc is initialized by its default constructor

 ifstream fin(hfile.c_str());
 if (!fin.is_open())
 {
  cerr << "Error: cannot open input file " << hfile << endl;
  exit(1);
 }
 fin.seekg(0,ios::end);
 size_t fsize=fin.tellg();
 fin.seekg(0,ios::beg);
  
 string buf;
 buf.resize(fsize);
 fin.read(&buf[0],fsize);
 fin.close();

 out=gumbo_parse(buf.c_str());
}

HDoc::~HDoc()
{
 gumbo_destroy_output(&kGumboDefaultOptions, out);
}

string HDoc::GenCCode(bool gtest)
{
 int varnum=0;
 bool v=InnerGenCode(out->document,0,rootvar_name,varnum);
 if (v==false)
 {
  cerr << "Error generating C++ code.\n";
  exit(1);
 }
 if (gtest)
  GenTest();

 return CCode;
}

void HDoc::GenHeaderAndAuxiliar()
{
 // Code for the .h file
 HCode="#ifndef "+stname+"_H\n#define "+stname+"_H\n";
 HCode += "string State_"+stname+"(";
 for (unsigned i=0;i<prototype.size();i++)
  HCode += prototype[i].first+" "+prototype[i].second+",";
 HCode += "string indentstring);\n#endif\n";

 // Code for the _hv.h file
 if (prototype.size()>0)
 {
  HV=",";
  for (unsigned i=0;i<prototype.size();i++)
  {
   HV += prototype[i].first+" &"+prototype[i].second;
   if (i<prototype.size()-1)
    HV += ",";
  }
 }
 else
  HV="";

 // Code for the _call.cpp file
 Call="";
 for (unsigned i=0;i<prototype.size();i++)
 {
  Call += prototype[i].second;
  if (i<prototype.size()-1)
   Call += ",";
 }

 // Code for the _vars.cpp file
 Vars="";
 for (unsigned i=0;i<prototype.size();i++)
  Vars += "  "+prototype[i].first+" "+prototype[i].second+";\n";
 Vars += "\n";
 
 HeaderGenerated=true;
}

void HDoc::GenTest()
{
 CCode += "\nint main(int argc, char *argv[])\n{\n";
 // This will be for the next version...
 // CCode += " InitializeHT5CPlusLibrary(true);\n\n";
 string call="";
 int iv=1;
 float fv=0.5;
 double dv=1234.56;
 for (unsigned i=0;i<prototype.size();i++)
 {
  call += prototype[i].second+",";
  CCode += " "+prototype[i].first+" "+prototype[i].second;

  if (prototype[i].first.find("vector")!=string::npos)
   CCode += ";\n // WARNING: don't forget to initialize "+prototype[i].second+"\n";
  if (prototype[i].first=="int")
  {
   CCode += " = "+tostring(iv)+";\n";
   iv++;
  }
  if (prototype[i].first=="float")
  {
   CCode += " = "+tostring(fv)+";\n";
   fv+=1.0;
  }
  if (prototype[i].first=="double")
  {
   CCode += " = "+tostring(dv)+";\n";
   fv+=123.45;
  }
  if (prototype[i].first=="string")
  {
   CCode += " = \"";
   for (int j=0;j<4;j++)
    CCode += char(97+(122-97)*float(rand())/RAND_MAX);
   CCode += "\";\n";
  }
 }

 CCode += " string indentstring=\" \";\n\n";
 CCode += " string page=State_"+stname+"("+call+"indentstring);\n";
 CCode += " if (argc==2)\n {\n  ofstream f(argv[1]);\n  f << page << endl;\n  f.close();\n }\n else\n  cout << page << endl;\n\n";
 CCode += " return 0;\n}\n";
}

void HDoc::Indent(int l)
{
 for (int i=0;i<l;i++)
  CCode += indent_unit;
}

bool HDoc::AllowedInId(char c)
{
 return ( (c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_' || c=='[' || c==']' );
}

bool HDoc::AddIfAbsent(string vtype,string vname)
{
 pair<string,string> dec(vtype,vname);

 if (prototype.size()==0)
 {
  prototype.push_back(dec);
  return true;
 }

 unsigned i=0;
 bool found=false;
 do
 {
  if (vname==prototype[i].second)
  {
   if (vtype!=prototype[i].first)
   {
    cerr << "Error: variable " << vname << " declared with two different types: " << vtype << " and " << prototype[i].first << endl;
    return false;
   }
   found=true;
  }
  i++;
 }
 while (i<prototype.size() && !found);

 if (!found)
  prototype.push_back(dec);

 return true;
}

string HDoc::ExVType(string vu,bool &OK)
{
 string prot;
 switch (vu[1])
 {
  case 'i':
  case 'I': prot="int"; break;
  case 'f':
  case 'F': prot="float"; break;
  case 'd':
  case 'D': prot="double"; break;
  case 's':
  case 'S': prot="string"; break;
  default: cerr << "Error using variable. " << vu << " is not an allowed variable identifier.\n"; OK=false; return ""; break;
 }
 size_t p=vu.find_first_of("[");
 if (p!=string::npos)
 {
  size_t q=vu.find_last_of("]");
  if (q==string::npos)
  {
   cerr << "Error using variable. " << vu << " is not an allowed variable identifier. Open bracket(s) '[' found with no closed bracket(s) ']'\n";
   OK=false;
   return "";
  }
  int adim;
  int nob=0,ncb=0;
  for (unsigned i=p;i<=q;i++)
  {
   if (vu[i]=='[')
    nob++;
   if (vu[i]==']')
    ncb++;
  }
  if (nob!=ncb)
  {
   cerr << "Error using variable. " << vu << " is not an allowed variable identifier: unbalanced open '[' and closed ']' brackets\n";
   OK=false;
   return "";
  }
  adim=nob;
  for (int d=0;d<adim;d++)
   prot="vector< "+prot+" >";
 }
 OK=true;
 return prot;
}

string HDoc::ExVName(string vu)
{
 size_t p=vu.find_first_of("[");
 return (p==string::npos) ? vu.substr(3,vu.size()-3) : vu.substr(3,p-3);
}

bool HDoc::AddToPrototype(string s)
{
 vector<string> vartypes;
 vector<string> varnames;
 vector< pair<bool,string> > pieces;
 if (!ExtractVariables(s,vartypes,varnames,pieces))
  return false;
 unsigned i=0;
 bool ret=true;
 while (i<vartypes.size() && ret)
 {
  ret=AddIfAbsent(vartypes[i],varnames[i]);
  i++;
 }
 return ret;
}

string HDoc::SubstituteVariables(string s,bool addquotes,bool &OK)
{
 if (s=="")
 {
  OK=true;
  return s;
 }

 vector<string> vartypes;
 vector<string> varnames;
 vector< pair<bool,string> > pieces;
 if (!ExtractVariables(s,vartypes,varnames,pieces))
  OK=false;
 
 if (vartypes.size()==0)
 {
  OK=true;
  return (addquotes ? "\""+s+"\"" : s);
 }

 string ssubst="";

 for (unsigned i=0;i<pieces.size();i++)
 {
  string tp,before, after;

  // If this piece is a variable:
  if (pieces[i].first)
  {
   // tp will contain the variable name
   tp=pieces[i].second.substr(3,pieces[i].second.size()-3);

   // If it is a string variable, nothing should be prepended or appended to transform it into a string
   if (pieces[i].second[1]=='s' || pieces[i].second[1]=='S')
    before=after="";
   // otherwise, the transformation is done with the polimorphic function tostring(var)
   else
   {
    before="tostring(";
    after=")";
   }
  }
  // If this piece is not a variable, it must be treated as a constant string. 
  // tp is the string itself and they will have to be surrounded by double quotes.
  else
  {
   tp=pieces[i].second;
   before=after="\"";
  }
  
  // If the piece is part of a C++ script, nothing has to be done.
  // Otherwise, the strings that surround the piece are set
  if (addquotes) 
  {
   tp=before+tp+after;
   if (i>0)  // Moreover, if it is not the first piece precede it with a '+'
   tp="+"+tp;
  }

  ssubst+=tp;
 }

 OK=true;

 // Just for debugging...
 /*
 for (unsigned i=0;i<pieces.size();i++)
  cout << "Piece " << i << (pieces[i].first ? " V --> " : " T --> '") << pieces[i].second << "'\n";
 cout << "Orig: '" << s << "'\n";
 cout << "Subs: '" << ssubst << "'\n";
 */

 return ssubst;
}

bool HDoc::ExtractVariables(string s,vector<string> &vartypes,vector<string> &varnames,vector< pair<bool,string> > &pieces)
{
 pair<bool,string> p(false,"");

 // No piece can have less than three characters, since a variable has the form $X_ident
 unsigned l=s.size();
 if ( s.find("$")==string::npos || l<3)
 {
  p.second=s;
  pieces.push_back(p);
  return true;
 }

 // spvars will contain the start position of the variables (if any) and epvars their end positions.
 vector<unsigned> spvars,epvars;
 for (unsigned k=0;k<l-3;k++)
 {
  if ( s[k]=='$' && s[k+2]=='_' )
  {
   spvars.push_back(k);
   unsigned k1=k+3;
   while (k1<l && AllowedInId(s[k1]) )
    k1++;
   epvars.push_back(k1-1);
  }
 }
 // If the string contains no variables the whole string is the only piece.
 if (spvars.size()==0)
 {
  p.second=s;
  pieces.push_back(p);
  return true;
 }

 // Now we run through the string adn get the pieces.
 // If the first variable does not start at the beginning of the string,
 // the first piece goes from the beginning the the first variable start
 if (spvars[0]!=0)
 {
  p.second=s.substr(0,spvars[0]);
  pieces.push_back(p);
  p.second=s.substr(spvars[0],epvars[0]-spvars[0]+1);
  pieces.push_back(p);
 }
 else // Otherwise, the first piece is the first variable
 {
  p.second=s.substr(0,epvars[0]+1);
  pieces.push_back(p);
 }
 // For any other piece, it is limited by this value of spvars and the former value of epvars
 for (unsigned i=1;i<spvars.size();i++)
 {
  if (spvars[i]!=epvars[i-1]+1)
  {
   p.second=s.substr(epvars[i-1]+1,spvars[i]-epvars[i-1]-1);
   pieces.push_back(p);
  }
  p.second=s.substr(spvars[i],epvars[i]-spvars[i]+1);
  pieces.push_back(p);
 } 
 // The last piece is special, too. We must check if it ends at the end of the string or not.
 // If not, we add another piece with the last part
 if (epvars[epvars.size()-1]!=(l-1))
 {
  p.second=s.substr(epvars[epvars.size()-1]+1,l-epvars[epvars.size()-1]);
  pieces.push_back(p);
 }

 // A second pass to detect the variables, mark the bool of the piece and store variable type and name
 unsigned i=0;
 bool ret=true;
 while (i<pieces.size() && ret)
 {
  if ( pieces[i].second[0]=='$' && pieces[i].second[2]=='_' )
  {
   vartypes.push_back(ExVType(pieces[i].second,ret));
   varnames.push_back(ExVName(pieces[i].second));
   pieces[i].first=true;
  }
  i++;
 }

 if (varnames.size()==0)
 {
  cerr << "Warning: variables detected in string " << s << " but no variable has been extracted.\n";
  cerr << "The pieces were:\n";
  for (unsigned i=0;i<pieces.size();i++)
   cerr << "Piece " << i << " --> '" << pieces[i].second << "'\n";
  return false;
 } 

 return true;
}

bool HDoc::GetPrototype(GumboNode *r)
{
 bool ret=false;

 switch (r->type)
 {
  // Variables cannot be present as first-level elements of document or into cdata or whitespace
  // (and if they are, they will be ignored)
  case GUMBO_NODE_DOCUMENT:
  case GUMBO_NODE_CDATA:
  case GUMBO_NODE_WHITESPACE: 
  case GUMBO_NODE_TEMPLATE: ret=true; break;
  // Variables can be as (or as part of) text content or as a part of comments (because the C++ scripts are treated as comments)
  case GUMBO_NODE_TEXT:
  case GUMBO_NODE_COMMENT:    ret=AddToPrototype(string(r->v.text.text)); break;
  // For any other node, we must inspect:
  case GUMBO_NODE_ELEMENT: 
  {
   // Variables can be present in the attributes of the tag...
   GumboVector *att = &r->v.element.attributes;
   unsigned i=0;
   ret=true;
   while (i<att->length && ret)
   {
    GumboAttribute *t = (GumboAttribute *)att->data[i];
    string tx=string(t->value);
    ret=AddToPrototype(tx);
    i++;
   }  
   // or in any of its children
   GumboVector *children = &r->v.element.children;
   i=0;
   ret=true;
   while (i<children->length && ret)
   {
    ret=GetPrototype((GumboNode *)children->data[i]);
    i++;
   }
  }
 }

 return ret;
}

bool HDoc::GenStartCode()
{
 if (GetPrototype(out->root)==false)
 {
  cerr << "Error obtaining the  prototype.\n";
  return false;
 }

 if (gentest)
  CCode += "/*\n  Test program for state "+stname+"\n";
 else
  CCode += "/*\n  Test function for state "+stname+"\n";
 if (!gentest)
 {
  CCode += "  This function will be compiled by EmIntweb to give an object code.\n\n";
  CCode += "  If you need to compile manually, see the compilation command generated into emintweb into the message area.\n";
 }
 else
 {
  CCode += "  This function will be compiled by EmIntweb to give an object code.\n\n";
  CCode += "  If you need to compile manually, see the compilation command generated into emintweb into the message area.\n";
  CCode += "     The program without arguments will output to the console a web page with dummy values for the variables.\n";
  CCode += "     and the program with one argument will output the page to the file named by the argument (overwriting it).\n";
 }
 CCode += "\n*/\n\n";
 CCode += "#include <iostream>\n#include <fstream>\n#include <string>\n#include <vector>\n#include <cstdlib>\n\n";
 CCode += "\nusing namespace std;\n\n";
 CCode += "#include \"html5document.h\"\n";
 // WARNING: check if something is wrong here...
 // CCode += "#include \""+hname+(gentest ? "_test" : "")+".h\"\n\n";
 CCode += "#include \""+hname+".h\"\n\n";

 CCode += "string State_"+stname+"(";
 for (unsigned i=0;i<prototype.size();i++)
  CCode += prototype[i].first+" "+prototype[i].second+",";
 CCode += "string indentstring)\n";

 return true;
}

string HDoc::NewSonVarname(string tv,int &ordervar)
{
 int d=ordervar;
 string ns="";
 string digit=" ";

 if (ordervar==0)
  ns="0";
 else
  while (d>0)
  {
   digit[0]=char('0'+(d%10));
   ns=digit+ns;
   d/=10;
  }
 
 size_t p=tv.find_last_of("_");
 if (p==string::npos)
  return( tv+"_"+ns);
 else
 {
  ordervar++;
  return ( tv.substr(0,p+1)+ns );
 }
}

bool HDoc::WriteCppScript(string comment,int level)
{
 stringstream ss(comment);
 string item;
 vector<string> lines;
 while (getline(ss,item,'\n'))
  lines.push_back(item);
 //Indent(level);
 //CCode += "// C++ script begins here...\n";
 for (unsigned i=0;i<lines.size();i++)
 {
  if (lines[i].find("C++",0)==string::npos)
  {
   bool OK=false;
   string s=SubstituteVariables(lines[i],false,OK);
   Indent(level);
   CCode +=  s+"\n";
   if (!OK)
   {
    Indent(level);
    CCode += "// C++ script ABNORMALLY ends here..\n";
    return false;
   }
  }
 }
 //Indent(level);
 //CCode += "// C+ script ends here...\n";
 return true;
}

string HDoc::NodeAttributes(GumboNode *node,bool &OK)
{
 string attrs="";
 
 GumboVector *att = &node->v.element.attributes;

 if (att->length==0)
 {
  OK=true;
  return "";
 }

 for (unsigned i=0;i<att->length;i++)
 {
  GumboAttribute *t = (GumboAttribute *)att->data[i];
  string attname=string(t->name);
  string attval=string(t->value);
  attrs += attname+"=\\\""+attval+"\\\" ";
 }

 string s=SubstituteVariables(attrs,true,OK);
 if (!OK)
  cerr << "Error getting node attributes.\n";
 return s;
}

bool HDoc::GenElementCode(GumboNode* node,int level,string parentvarname,int varnum)
{
 TaglabelDefs *syntax=TaglabelDefs::instance();
 string ttype=syntax->toName(node->v.element.tag); //tagnames[node->v.element.tag];
 bool OK=false;
 string attrs = NodeAttributes(node,OK);

 if (!OK)
  return false;

 GumboVector *children = &node->v.element.children;

 Indent(level);
 CCode += "{\n";
 Indent(level+1);

 string thisvarname=NewSonVarname(parentvarname+"_",varnum);

 if (attrs=="")
  CCode += "Tag "+thisvarname+"(\""+ttype+"\");\n";
 else
  CCode += "Tag "+thisvarname+"(\""+ttype+"\","+attrs+");\n";

 int varnumber=0;
 for (unsigned i=0;i<children->length;i++)
  if (InnerGenCode((GumboNode *)children->data[i],level+1,thisvarname,varnumber)==false)
  {
   cerr << "Error parsing children number " << i << " of node with type " << ttype << endl;
   return false;
  }

 Indent(level+1);
 CCode += parentvarname+".add("+thisvarname+");\n";
 Indent(level);
 CCode += "}\n";

 return true;
}

bool HDoc::GenCDATACode(GumboNode* node,int level)
{
 // CDATA is written verbatim as a C++ comment
 Indent(level);
 CCode += "/* CDATA in source html file begins:\n";
 Indent(level+1);
 CCode += node->v.text.text;
 CCode += "\n";
 Indent(level);
 CCode += "CDATA in source html file ends.*/\n";

 return true;
}

bool HDoc::GenCommentCode(GumboNode* node,int level)
{
 string comment=string(node->v.text.text);

 // The comment may be a C++script. In this case, write it and return
 if (comment.find("C++",0)!=string::npos)
  return WriteCppScript(comment,level);

 // Otherwise, write the comment verbatim as a C++ comment
 Indent(level);
 CCode += "/* Comment in source html file begins:\n";
 Indent(level+1);
 CCode += comment+"\n";
 Indent(level);
 CCode += " Comment in source html file ends. */\n";

 return true;
}

bool HDoc::GenDocumentCode(GumboNode* node,string parentvarname)
{
 if (!GenStartCode())
  return false;

 CCode += "{\n";
 Indent(1);
 CCode += "HTML5Document "+parentvarname+";\n";

 // The document node has no sensible attributes, neither sensible lenght for the list of attibutes
 // neither sensible name for the tag. Don't try to consult or use them. (Gumbo library bug?)

 GumboVector *children = &node->v.element.children;

 for (unsigned i=0;i<children->length;i++)
  if (!InnerGenCode((GumboNode *)children->data[i],1,parentvarname,0))
   CCode += "// ERROR before this point\n";

 Indent(1);
 CCode += "return "+parentvarname+".Generate(indentstring);\n";
 CCode += "}\n";

 return true;
}

bool HDoc::GenTextCode(GumboNode* node,int level,string varname)
{
 Indent(level);
 string t=node->v.text.text;

 // Some strings appear to be preceeded of followed by carriage returns that we must get rid of
 if (t[0]=='\n')
  t=t.substr(1,t.size()-1);
 if (t[t.size()-1]=='\n')
  t=t.substr(0,t.size()-1);

 bool OK=false;
 t=SubstituteVariables(t,true,OK);
 CCode += varname+".SetContent("+t+");\n";

 return OK;
}

bool HDoc::InnerGenCode(GumboNode* node,int level,string parentvarname,int varnum)
{
 bool ret=false;
 switch (node->type)
 {
  case GUMBO_NODE_DOCUMENT:   ret=GenDocumentCode(node,parentvarname); break;
  case GUMBO_NODE_ELEMENT:    ret=GenElementCode(node,level,parentvarname,varnum);  break;
  case GUMBO_NODE_TEXT:       ret=GenTextCode(node,level,parentvarname); break;
  case GUMBO_NODE_CDATA:      ret=GenCDATACode(node,level); break;
  case GUMBO_NODE_COMMENT:    ret=GenCommentCode(node,level); break;
  case GUMBO_NODE_WHITESPACE: 
  case GUMBO_NODE_TEMPLATE:   ret=true; break;
 }
 return ret;
}
