#ifndef _CONTEXT_H
#define _CONTEXT_H

#define HTML_INDENT_STRING "  "

// This is the buffer where the client request with the variables will be stored.
// It is not to write the page, that is written directly to the socket
// Therefore, 64 Kb should be enough (even too much) but feel free to change as needed
#define BUFFER_SIZE 64*1024

// The maximum length allowed for an URI (to avoid buffer overflow attacks)
#define MAX_URI_LENGTH 1024

// These are the codes that will signal errors in the read URL
// Normally, they will be set with the SetError function in main
// or the transition wrappers, so you don't need to worry about,
// but you can consult them with GetError and decide which error
// message to show in the page of the fallback state (if any)
#define NO_ERROR             0
#define URL_TOO_LONG         1
#define FORBIDDEN_CHAR       (1<<1)
#define TOO_FEW_VARIABLES    (1<<2)
#define TOO_MANY_VARIABLES   (1<<3)
#define UNKNOWN_VARIABLE     (1<<4)
#define UNEXPECTED_VAR_VALUE (1<<5)
#define POCO_IO_EXCEPTION    (1<<6)

// Add here codes for whatever error you may need, each defined as (1<<num_error)
// where num_error can be between 7 and 31 included. This allow the OR'ing of the errors.
// You can use them in the same way as the URL errors with the SetError/GetError mechanism
// DON'T forget to update function ReadableError if you want significant texts.
#define SQL_ERROR			 (1<<7)

#define PREPEND_PATH "/usr/share/emintweb/pages"

// Constants to determine the outlet type
#define OUT_TO_COUT 0
#define OUT_TO_CERR 1
#define OUT_TO_FILE 2

// Constants to set the destination of the debug files per thread
#define DEBFILE_DIRECTORY "/tmp"
#define DEBFILE_ROOTNAME  "emw_th"

// string is used by the automatically generated code and iostream by the error messages
// vector is used by the default context
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// This is to include the automatically generated header that contains exclusively the
// constant SERVERDEB which can be used by the main program and by any transition code
#include "serverdeb.h"

// Add here whatever includes you need.

using namespace std;

class Context
{
 public:

 // Don't change anything from here...
  Context();
  ~Context();
  int GetError() { return last_error; };
  void SetError(unsigned long errnum) { last_error|=errnum; };
  void ResetError() { last_error=NO_ERROR; };
  void SetOutletType(unsigned char outtype);
  ostream& GetOutletStream();
  string ReadableError();
  void EmitOVars(vector<pair<string,string>> &ov,unsigned long errnum,string fun_name);
  
  // Function to parse arguments, if passed to the main program.
  // The first two arguments (program name and port number) are NOT passed here
  // argv contains only the arguments from the third one (if any)
  // numargs will contain the number of real arguments in argv.
  // WARNING: the implementation of this function is provided to you, empty.
  // You will know what do you want to do with the program arguments.
  // If you don't need program arguments, forget this note.
  bool SetArgs(int numargs,char *argv[]);
 
  // Function to check for a file that will be returned for requests without variables,
  // if this is allowed. It will return the ofstream object to the file, that will be
  // opened or not depending on if the file exists and can be served or not.
  // We will also set the mimetype variable to the correct type for the file
  // and the filelength
  // WARNING: the implementation of this function is provided to you, empty
  // and you must fill it.
  // You will know how (and if) you want to serve files, and which ones.
  string RequestedFile(string &URI,string &MimeType,size_t &filelength);

  // Function to look for invalid characters in a string and substitute them for the requested subst char
  // You must implement this function because only you know which characters you will accept.
  // This function shoudl return true if any invalid character has been found.
  bool InvalidChars(string &s,char subs);

  // Function to modify the page contained in its first argument so that errors signaled by error
  // are shown in it (if you want to show them, which could give clues to the hackers...)
  void ModifyPage(string &page,unsigned long error);

  // Functions to set and get the string used as the indentation unit by the C++ page code generator
  void SetIndentString(string s) { indentstring=s; };
  string GetIndentString()       { return indentstring; };

  // Functions to get/set the port that this thread opens. Useful in the multithreaded-server case.
  unsigned short GetPort() { return port; };
  void SetPort(unsigned short p) { port=p; };

  // Function to do whatever you consider necessary after the context has been created and its private
  // arguments have been set to their correct values. It should return false to signal any error and true otherwise.
  bool InitializeContext();

// ... to here

// Add here the public interface to your Context clase...

 private:
  unsigned long last_error;
  string indentstring;
  unsigned short port;
  unsigned char outlet_type;
  ofstream outlet_ofstream;
// Add here any private variables and functions you need

};
#endif
