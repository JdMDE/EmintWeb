// Function to be called when the program finishes
string Terminal_transition(Poco::Net::NameValueCollection &FormWithIVars,string &next_st,Context &usercontext)
{
 next_st="End_of_program";
 return "";
}

// Function to write sensible descriptions of the errors (either to cerr or to an opened file stream)
void AnnounceError(ServerErrors error_type,string err_desc,unsigned short port,ostream &errst,int len,int exp_len)
{
 errst << "Error from program/thread at port " << port;
 switch (error_type)
 {
  case Bind:       errst << ". AnnounceBindError." << endl;
                   errst << "Error description is " << err_desc << endl;
                   break;
  case Listen:     errst << ". AnnounceListenError." << endl;
                   errst << "Error description is " << err_desc << endl;
                   break;
  case AcceptCon:  errst << ". AnnounceAcceptConnectionError." << endl;
                   errst << "Error description is " << err_desc << endl; 
                   break;
  case ChangeAddr: errst << ". AnnounceChangeAddrError." << endl;
                   errst << "Error description is:\n  " << err_desc << endl;
                   break;
  case Send:       errst << ": ProcessSendMsgError." << endl;
                   errst << "In call to the sendBytes method of the listener socket already in accepting state,\n";
                   errst << "the number of sent bytes, as returned by sendBytes, is " << len << " whereas the number of bytes we wanted to send was " << exp_len << endl;
                   errst << "Error description is " << err_desc << endl;
                   break;
  case Receive:    errst << ". ProcessReceiveError." << endl;
                   errst << "In call to the receiveBytes method of the listener socket already in accepting state,\n";
                   errst << "the received number of bytes, as returned by receiveBytes, is " << len << endl;
                   errst << "Error description is " << err_desc << endl; 
                   break; 
  default:         errst << ". Unexpected error. No description.\n";
                   break;         
 }
 errst.flush();
}

#ifdef SECURE_HTTP
// Procedure to prepare the context for SSL transmission
Poco::Net::Context::Ptr PrepareSSLContext(pthread_t *pthread_id,ostream &errst)
{
 // The check of the existance and readability of RSA key files is checked only in the case of
 // the application being launched as an autonomous server. If it is launched as a thread, this is checked only once by the superserver.

 ifstream pemfile(PEM_FILE);
 if (!pemfile.is_open())
 {
  if (pthread_id==nullptr)
   errst << "Error from server: cannot open PEM file " << PEM_FILE << ". Check for existance and permissions.\n";
  else
  {
   errst << "Error from thread with ID " << *pthread_id << ": cannot open PEM file " << PEM_FILE << ". Check for existance and permissions.\n";
   pthread_exit(nullptr);
  }
 }
 pemfile.close();
 ifstream pemkfile(PEM_KEY);
 if (!pemkfile.is_open())
 {
  if (pthread_id==nullptr)
   errst << "Error from server: cannot open PEM file " << PEM_FILE << ". Check for existance and permissions.\n";
  else
  {
   errst << "Error from thread with ID " << *pthread_id << ": cannot open PEM KEY file " << PEM_KEY << ". Check for existance and permissions.\n";
   pthread_exit(nullptr);
  }
 }
 pemkfile.close();
 
 // This is the code to open a socket to manage encrypted transmissions
 Poco::Net::Context::Ptr ctx_ptr = new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE,PEM_KEY,PEM_FILE,"",Poco::Net::Context::VERIFY_NONE);
 
 return(ctx_ptr);
}
#endif

// Procedure to send a HTML page contained in a string.
// The socket sts whose pointer is passed is supposed to be opened and binded to port 'port'
#ifdef SECURE_HTTP
void SendPage(string page,unsigned short port,Poco::Net::SecureStreamSocket &sts,ostream &errst)
#else
void SendPage(string page,unsigned short port,Poco::Net::StreamSocket &sts,ostream &errst)
#endif
{
 // A string stream that will be associated to the socket: everything wrote in it will be sent
 ostringstream ostr;
 // The HTTP header of the response that will be followed by the page
 Poco::Net::HTTPResponse sresp;
 
 sresp.setVersion("HTTP/1.1");
 sresp.setContentType("text/html; charset=UTF-8");
 sresp.setContentLength(page.size());
 // This is important in our framwork: connection must be persistent
 sresp.setKeepAlive(true);
 sresp.write(ostr);

 ostr << page;
 int len=ostr.str().length();

 int nbytes;
 try
 {
  nbytes=sts.sendBytes(ostr.str().c_str(),len,0);
 }
 catch (Poco::IOException& exc)
 {
  cerr << "Received IOException from the POCO libraries after sendBytes in SendPage\n";
  cerr << exc.displayText() << endl;
 }
 /*
 catch (Poco::TimeoutException& exc2)
 {
  cerr << "Received TimeoutException from the POCO libraries after sendBytes in SendPage\n";
  cerr << exc2.displayText() << endl;
 }
 */
 if (sts.getError())
  AnnounceError(Send,sts.lastErrorDesc(),port,errst,nbytes,len);
}

// Procedure to send a file whose URI has been requested by the client
// The socket sts whose pointer is passed is supposed to be opened and binded to port 'port'
// The path should be an absolute path in the sever's filesystem. Existence and adequate permissions
// should have been checked before calling this function.
// mimetype and filelength are those of the file to be sent
#ifdef SECURE_HTTP
void SendFile(string path,string mimetype,size_t filelength,unsigned short port,Poco::Net::SecureStreamSocket &sts,ostream &errst)
#else
void SendFile(string path,string mimetype,size_t filelength,unsigned short port,Poco::Net::StreamSocket &sts,ostream &errst)
#endif
{
 ostringstream ostr;
 Poco::Net::HTTPResponse sresp;

 sresp.setVersion("HTTP/1.1");
 sresp.setContentType(mimetype);
 sresp.setContentLength(filelength);
 sresp.setKeepAlive(true);
 sresp.write(ostr);

 int len=ostr.str().length();
 
 int nbytes;
 try
 {
  nbytes=sts.sendBytes(ostr.str().c_str(),len,0);
 }
 catch (Poco::IOException& exc)
 {
  cerr << "Received IOException from the POCO libraries after sendBytes in SendFile (header)\n";
  cerr << exc.displayText() << endl;
 }
 /*
 catch (Poco::TimeoutException& exc2)
 {
  cerr << "Received TimeoutException from the POCO libraries after sendBytes in SendFile (header))\n";
  cerr << exc2.displayText() << endl;
 }
 */
 if (sts.getError())
  AnnounceError(Send,sts.lastErrorDesc(),port,errst,nbytes,len);
 
 // File is opened and sent in chuncks of 128 bytes. Each one is checked after sending it
 ifstream f(path.c_str());
 char *buf=new char[FILE_BUF_SIZE];
 int chunk=0;
 while (!f.eof())
 {
  f.read(buf,128);
  try
  {
   nbytes=sts.sendBytes(buf,f.gcount(),0);
  }
  catch (Poco::IOException& exc)
  {
   cerr << "Received IOException from the POCO libraries after sendBytes in SendFile, chunk " << chunk << endl;
   cerr << exc.displayText() << endl;
  }
  /*
  catch (Poco::TimeoutException& exc2)
  {
   cerr << "Received TimeoutException from the POCO libraries after sendBytes in SendFile, chunk " << chunk << endl;
   cerr << exc2.displayText() << endl;
  }
  */
  if (sts.getError())
   AnnounceError(Send,sts.lastErrorDesc(),port,errst,nbytes,len);
  chunk++;
 }
 f.close();
 delete[] buf;
}


