#ifndef _HTML5EMINTWEB_H
#define _HTML5EMINTWEB_H

#include <gumbo.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <string.h>
#include <ctype.h>

#include "global.h"
#include "html5document.h"

using namespace std;

//! Class to contain all the methods to parse and generate C++ code from HTML that can be used by EmIntWeb
/*!
    @author Juan Domingo (Juan.Domingo@uv.es)
    @date 2023
*/
class HDoc
{
 public:
  //! Only constructor
  /*! 
       \param hfile Name of the HTML input file
       \param stn   State that will be the name of the C++ function
       \param hn    Name of the header file that will be generated
       \param gt    true if a main for testing has been requested, false otherwise
  */
  HDoc(string hfile,string stn,string hn,bool gt);

  //! Destructor
  /*!
       The destructor simply calls to gumbo_destroy_output
  */
  ~HDoc();

  //! Generates a string containing the C++ code
  /*!
       \param  gtest  true to generate a main after the function for testing purposes
       \return A string with the generated program (contains carriage returns)
  */
  string GenCCode(bool gtest);

  //! Generates the header file with the function prototype
  /*!
      \return A string with the contents of the header file, the _hv file and the call file, respectively
  */
  string GenHeader() { if (!HeaderGenerated) GenHeaderAndAuxiliar(); return HCode; };

  //! Generates the contents for the _hv.h auxiliary file
  /*!
      \return A string with the contents of the _hv.h file
  */
  string GenHV() { if (!HeaderGenerated) GenHeaderAndAuxiliar(); return HV; };

  //! Generates the call file with the contents for the _call.cpp auxiliary file
  /*!
      \return A string with the contents of the _call.cpp auxiliary file
  */
  string GenCall() { if (!HeaderGenerated) GenHeaderAndAuxiliar(); return Call; };

  //! Generates the vars file with the contents for the _vars.cpp auxiliary file
  /*!
      \return A string with the contents of the _var.cpp auxiliary file
  */
  string GenVars() { if (!HeaderGenerated) GenHeaderAndAuxiliar(); return Vars; };

  //!  Sets the unit of indentation. Each level adds a new unit. Default value is a blank space
  /*!
       \param iu  A string to be used as indentation unit, usually a set of blanks or tabulators
  */
  void SetIndentUnit(string iu) { indent_unit=iu; };

  //!  Sets the name of the root (initial) variable. All other variables are named after this. Default value is 'mvar'
  /*!   
       \param rvn Any string that will be used as the name of the main variable
  */
  void SetRootVarName(string rvn) { rootvar_name=rvn; };

 private:
  
  //! Generates the test part of the program, a main with dummy values for the needed variables that calls the generated function
  void GenTest();                         
  
  //! Generates the prototype of the function and the declaration of the first variable of type HTMLDocument
  /*!
      \param  hname Name of the header file to be put under a #include directive
      \return true if the starting code has been succesfully generated, false otherwise
  */
  bool GenStartCode();

  //! Runs through the HTML input string extracting the variables and adds them to the prototype
  /*! This function looks for strings of the form $X_ident where X is i,I,f,F,d,D,s or S to indicate the type of the variable (respectively,
      integer,float,double or string) and ident is an identifier (anything contaning the characters allowed by HDoc::AllowedInId). The variable
      declaration is extracted in the form vartype varname where vartype is the simple C++ type for usual variables or a vector for arrays.
      Simple arrays are managed as vector<vartype> varname. Matrices are managed as vector< vector<vartype> > varname, and so on for higher
      dimensional arrays. The prototypes are added to the current list of prototypes as soon as the variable is found; later findings of
      the variable with same name and type are not added, but later findings of the same variable name with a different type provoke an error.

      \param n A GumboNode, the HTML tree node we are currently exploring
      \return true if the prototype has been correctly extracted and false if not (there has been any error) 
  */
  bool GetPrototype(GumboNode *n);

  //! Generates the C++ code corresponding to a given node of the HTML tree
  /*! When running through the HTML tree (in DFS), each node is passed to this function that generates the C++ code corresponding to it.
      The code for each node is embraced by '{' .. '}' (what I call a synthetic scope) just to keep variables isolated and to avoid
      name conflicts. Furthermore, the names of the variables are generated using a root name, inherited from the ancestor node, and a
      number (which is the order in which the node appears with respect to all its siblings in the same level)

      \param n The current GumboNode we are analysing
      \param level The identation level that should ge given to the piece of code generated by this node
      \param rootvarname An identifier used to generate the names of the variables that will be needed
      \param varnum The number of this node among its siblings
      \param  hname Name of the header file to be put under a #include directive
      \return true if the code has been generated correctly and false if there has been any error
  */
  bool InnerGenCode(GumboNode *n,int level,string rootvarname,int varnum);

  //! Generates the C++ code of the whole document by passing it the only GumboNode of type GUMBO_NODE_DOCUMENT
  /*!
      \param node The document node
      \param parentvarname The name we want to give to the parent variable and that will be the name of the only HTMLDocument variable
      \param  hname Name of the header file to be put under a #include directive
      \return true if the code has been generated correctly and false if there has been any error
  */
  bool GenDocumentCode(GumboNode* node,string parentvarname);

  //! Generates the C++ code of an element of type GUMBO_NODE_ELEMENT
  /*!
      \param node The document node
      \param level The level of indentation for the code generated from this element
      \param parentvarname The name of the variable used in the parent of this node
      \param varnum The number of this node among its siblings
      \return true if the code has been generated correctly and false if there has been any error
  */
  bool GenElementCode(GumboNode* node,int level,string parentvarname,int varnum);

  //! Generates the C++ code of an element of type GUMBO_NODE_TEXT
  /*! Differently to ELEMENT nodes, text nodes are leaves and no further tree is generated from them. In the generated code,
      their content is added to the variable of the parent node using var.add(content)

      \param node The document node
      \param level The level of indentation for the code generated from this element
      \param parentvarname The name of the variable used in the parent of this node
      \return true if the code has been generated correctly and false if there has been any error
  */
  bool GenTextCode(GumboNode* node,int level,string parentvarname);

  //! Generates the C++ code of an element of type GUMBO_NODE_CDATA
  /*! The CDATA in a HTML document are transformed into C++ comments in the form<br>
      * CDATA in source html file begins:<br>
         (CDATA copied verbatim here)<br>
         CDATA in source html file ends. *

      \param node The document node
      \param level The level of indentation for the code generated from this element
      \return true always, since no error can happen here
  */  
  bool GenCDATACode(GumboNode* node,int level);

  //! Generates the C++ code of an element of type GUMBO_NODE_COMMENT
  /*! The comments in a HTML document are transformed into C++ comments in the form<br> 
      * Comment in source html file begins:<br>
          (Comment copied verbatim here)<br>
         Comment in source html file ends. *

      \param node The document node
      \param level The level of indentation for the code generated from this element
      \return true always, since no error can happen here
  */  
  bool GenCommentCode(GumboNode* node,int level);

  //! Generates a new name for the son variable of a given one
  /*!
      \param tv The name of the parent variable
      \param ordervar The order number to avoid variable name repetition. Is is changed to the next one after generation.
      \returns A new variable name derived from the former one but not equal to any other derived from it.
  */
  string NewSonVarname(string tv,int &ordervar);
  
  //! Generates the string that will be included in the C++ code to be added as attribute to the variable of the node
  /*! Essentially, this function convert the list of HTML attributes of the form name=value into a string that will
      be used by the generated C++ function variable.SetAttributes(...). The important thing is that this string is
      not equal to the HTML string of attributes because some problems with the double quotes must be checked, and
      over all, the variables must be substituted (YES, you can use $X_ident in both, attribute names and attribute
      values. Nice, isn't it?)

      \param node The GumboNode whose attributes will be converted to a string
      \param OK Returns with true is there has been no error in the substitution and false otherwise.
      \return The string to be used in the generated code.
  */
  string NodeAttributes(GumboNode* node,bool &OK);

  //! Adds to the prototype of the generated C++ function the variables and their declarations that might be present in a piece of HTML
  /*! The prototypes are stored into a vector of pairs, internally stored as variable HDoc::prototype
      HTML lines without variables have no effect

      \param s The HTML piece we are currently analysing
      \return true if there were no variables or the variables have been correctly managed. false otherwise.
  */
  bool AddToPrototype(string s);

  //! Substitute the variables in a piece of HTML or of C++string according to the variable list previously stored in HDoc::prototype
  /*! Variable substitution is done differently if the string comes from usual HTML, either as content or as attributes,
      or from a C++script. A variable is passed to indicate which of these two modes must be used

      \param s The HTML piece we are currently analysing
      \param addquotes true if the piece comes from HTML content or attribute, false if it comes from a C++script
      \param OK Returns with true if the substitution has been correctly done, false otherwise
      \return A string with the variables substituted
  */
  string SubstituteVariables(string s,bool addquotes,bool &OK);

  //! Extracts the variable type from a string containing a use of the variable, like $I_var which would give 'int' , $F_var[i][j] which would give vector< vector<float>>, etc.
  /*! For details of the extraction procedure, look at the comments to HDoc::GetPrototype

      \param vu A string containing a variable use
      \param OK Returns with true if the prototype has been correctly extracted and false otherwise
      \return A string with the variable type, as it will be stored in the prototype of the function
  */
  string ExVType(string vu,bool &OK);

  //! Extracts the variable name from a string containing a use of the variable, like $I_count which would give count, $F_m[i][j] which would give m, etc.
  /*! The identifier (variable name) is anything from the fourth character to the last, or up to the finding of an open brace, '['

      \param vu A string containing a variable use
      \return A string with the variable name, as it will be stored in the prototype of the function   
  */
  string ExVName(string vu);

  //! Extracts all the variable names and types from a piece of HTML or C++script.
  /*! This function not only gets the names and types of the variables but also divides the input string into pieces which contain
      either a single use of a variable, or no variables. Each of these pieces is stored as the second component of a pair whose
      first component is a boolean field to indicate if the second has a variable (true) or not (false).

      \param s The string where variables will be looked for
      \param vartypes A vector of strings that will return with the type of all the variables found
      \param varnames A vector of strings that will return with the name of all the variables found
      \param pieces A vector of pairs (bool,string) that will the return with the found pieces that compose the string s
      \return true if the variables have been correctly extracted and false otherwise 
  */
  bool ExtractVariables(string s,vector<string> &vartypes,vector<string> &varnames,vector< pair<bool,string> > &pieces);

  //! Adds a variable with type and name to the list stored in HDoc::prototype unless it is already present or it is not consistent with previous uses
  /*!
      \param vtype The type of the variable to be added
      \param vname The name of the variable to be added
      \return true if the 'type name' declaration has been added, or the 'type name' declaration was already present<br>
              false if there is a declaration 'type name' with the same variable name but a different type (fatal error: the HTML must be corrected)
  */
  bool AddIfAbsent(string vtype,string vname);
  
  //! Writes the C++ script fragments to the generated code, appropriately substituting the variables.
  /*! The code the user has put into a C++script is mainly copied verbatim, except that $X_ident is changed by ident

      \param comment The HTML comment containing the C++script
      \param level The current indentation level
      \return true if the code has been succesfully added (always, unless the variable substitution has failed) and false otherwise
  */
  bool WriteCppScript(string comment,int level);

  //! Outputs the requested number of indentation units
  /*!
      \param l Number of indentation units
  */
  void Indent(int l);

  //! Checks if a character is allowed as part of a variable indentifier.
  /*! 
      \param c The character to be tested
      \return true if it is allowed as part of a variable identifier and false if not<br>
      Characters allowed in identifiers are those in ranges [a..z],[A..Z] and [0..9] plus the underscore '_' and the open and closed braces '[' and ']'
  */
  bool AllowedInId(char c);

  //! Generates the header, hv, call and vars of the page filling the appropriate internal variables.
  void GenHeaderAndAuxiliar();

  string stname;			   //!< Name of the state and of the C++ function
  string hname;                            //!< Name of the header file (needed to be included in the source file)
  bool gentest;                            //!< True if a main for testing has been requested
  vector< pair<string,string> > prototype; //!< A vector of components of the prototype in the form pair(prototype,variable name)
  GumboOutput *out;                        //!< A pointer to be used by the gumbo library to return the parsed tree
  bool HeaderGenerated;                    //!< A flag to know if the next four variables have been filled
  string CCode;                            //!< The output where the generated C++ code will be accumulated
  string HCode;                            //!< The output where the generated header code will be accumulated
  string HV;                               //!< The output where the generated _hv.h will be stored
  string Call;                             //!< The output where the call to the function will be stored
  string Vars;                             //!< The output where the vars to be declared before the call will be stored
  string indent_unit;                      //!< The indentation unit. Defaults to a blank space. Can be changed by HDoc::SetIndentUnit
  string rootvar_name;                     //!< The name of the main variable (the HTMLDocument var) after which all other variables will be generated. Defaults to 'mvar'. Can be changed by HDoc::SetRootVarName
  HTML5Document doc;                       //!< The document that will be generated
};

#endif
