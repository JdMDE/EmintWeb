// This is a global mutex to protect the shared memory area that will contain the array of used ports.
// The main program and all the theads will have read/write access to that array so the mutex is needed to arbitrate the access
// We call it 'only' because we don't need any other by now.
pthread_mutex_t only_mutex;

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
  errst << "Received IOException from the POCO libraries after sendBytes\n";
  errst << exc.displayText() << endl;
 } 
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
  errst << "Received IOException from the POCO libraries after sendBytes\n";
  errst << exc.displayText() << endl;
 } 
 if (sts.getError())
  AnnounceError(Send,sts.lastErrorDesc(),port,errst,nbytes,len);
 
 // File is opened and sent in chuncks of 128 bytes. Each one is checked after sending it
 ifstream f(path.c_str());
 char *buf=new char[FILE_BUF_SIZE];
 while (!f.eof())
 {
  f.read(buf,128);
  try
  {
   nbytes=sts.sendBytes(buf,f.gcount(),0);
  }
  catch (Poco::IOException& exc)
  {
   errst << "Received IOException from the POCO libraries after sendBytes\n";
   errst << exc.displayText() << endl;
  } 
  if (sts.getError())
   AnnounceError(Send,sts.lastErrorDesc(),port,errst,nbytes,len);
 }
 f.close();
 delete[] buf;
}

// This function generates the page that the general server sends to each client as soon as it connects.
// It is simply a page to redirect the client to the server open in the port that has been assigned to it.
// Indeed, if the option SEND_AUTOMATIC_REDIRECTION_IN_SERVER has been defined as true, the page provokes
// the automatic redirection in most (hopefully, all) browsers.
// The new URI to send the client to is composed by the name of the server plus the port.
// sts is the currently opened socket to which we'll send the page.
#ifdef SECURE_HTTP
bool SendInitialPage(string server_name,unsigned short port,Poco::Net::SecureStreamSocket &sts,ostream &errst)
#else
bool SendInitialPage(string server_name,unsigned short port,Poco::Net::StreamSocket &sts,ostream &errst)
#endif
{
 string protocol;
 #ifdef SECURE_HTTP
 protocol="https";
 #else
 protocol="http";
 #endif
 
 string page;
 if (SEND_AUTOMATIC_REDIRECTION_IN_SERVER)
 {
  // This page uses the Refresh attribute in head to send the client automatically to the new port.
  page  ="<html>\n";
  page += "<head>\n";
  page += "  <meta http-equiv=\"Refresh\" content=\"0; URL="+protocol+"://"+server_name+":"+std::to_string(port)+"\" >\n";
  page += "</head>\n";
  page += "<body>\n";
  page += "If you have not been redirected, please click <a href=\""+protocol+"://"+server_name+":"+std::to_string(port)+"\">here</a> to start the application.<br/>\n";
  page += "</body>\n";
  page += "</html>\n";
 }
 else
 {
  // This page does not set enything on the head. It requires the client explicitly clicks on a button to start
  page  ="<html>\n";
  page += "<head></head>\n";
  page += "<body>\nPlease, click <a href=\""+protocol+"://"+server_name+":"+std::to_string(port)+"\">here</a> to start the application.<br/>\n</body>\n";
  page += "</html>\n";
 }
 
 ostringstream ostr;
 Poco::Net::HTTPResponse sresp;
 
 sresp.setVersion("HTTP/1.1");
 sresp.setContentType("text/html; charset=UTF-8");
 sresp.setContentLength(page.size());
 sresp.setKeepAlive(true);
 sresp.write(ostr);

 ostr << page;
 
 int len=ostr.str().size();
 
 int nbytes;
 try
 {
  nbytes=sts.sendBytes(ostr.str().c_str(),len,0);
 }
 catch (Poco::IOException& exc)
 {
  errst << "Received IOException from the POCO libraries after sendBytes\n";
  errst << exc.displayText() << endl;
 } 
 if (sts.getError())
 {
  AnnounceError(Send,sts.lastErrorDesc(),port,errst,nbytes,len);
  return false;
 }
 
 if (SERVERDEB)
  cout << "Main server: sending redirection page for opening thread listening in port " << port << endl;
  
 return true;
}

// This is the function that the general server sends to a client when no more ports are available and the client cannot be currently served
// It is simple a page to warn the client about it.
// entry_port is the port in which the main server (i.e. ourselves) is running. It is passed only because
// an error function needs to know it.
#ifdef SECURE_HTTP
bool SendBusyPage(unsigned short entry_port,Poco::Net::SecureStreamSocket &sts,ostream &errst)
#else
bool SendBusyPage(unsigned short entry_port,Poco::Net::StreamSocket &sts,ostream &errst)
#endif
{ 
 string page="<html>\n";
 page += "<head>\n";
 page += "</head>\n";
 page += "<body>\n";
 page += "Sorry, currently the server cannot find a port to serve you. We are too busy.<br/>\n";
 page += "Please, wait for a while and retry your petition.<br/>\n";
 page += "</body>\n";
 page += "</html>\n";
 
 ostringstream ostr;
 Poco::Net::HTTPResponse sresp;
 
 sresp.setVersion("HTTP/1.1");
 sresp.setContentType("text/html; charset=UTF-8");
 sresp.setContentLength(page.size());
 sresp.setKeepAlive(true);
 sresp.write(ostr);

 ostr << page;
 
 int len=ostr.str().size();
 
 int nbytes;
 try
 {
  nbytes=sts.sendBytes(ostr.str().c_str(),len,0);
 }
 catch (Poco::IOException& exc)
 {
  errst << "Received IOException from the POCO libraries after sendBytes\n";
  errst << exc.displayText() << endl;
 } 
 if (sts.getError())
 {
  AnnounceError(Send,sts.lastErrorDesc(),entry_port,errst,nbytes,len);
  return false;
 }
 
 if (SERVERDEB)
  cout << "Main server: sending warning of busy page to client.\n";
  
 return true;
}

// This is needed to recover the application name from the name of the program. 
// In our system the name of the main server is built from the project name + _Server.
// But the program is at a path so the last / has to be found, if there is one.
string RecoverAppName(string app_path)
{
 size_t n=app_path.size()-1;
 while ((n>0) && (app_path[n]!='_'))
  n--;
 if ( n==0 || app_path.substr(n,string::npos) != "_Server" )
 {
  cerr << "Error: progam full pathname does not end in _Server\n";
  exit(1);
 }
 
 string appname;
 size_t q=n-1;
 if (q==0)
  appname=app_path.substr(q,1);
 else
 {
  while ((q>0) && (app_path[q]!='/'))
   q--;
  if (q==0)
   appname=app_path.substr(0,n);
  else
   appname=app_path.substr(q+1,n-q-1);
 }
 return appname;
}

// The arguments to the main server are three compulsory arguments: 
//  The hostname in which this server is opened, in the way we wish the clients look for us when they are redirected
//  The port in which this server is opened
//  The range of ports in which the new servers (thread servers) can be opened in the form initial_port-final_port
// These are called NATIVE_ARGS. It is checked that they are numbers with correct values (all in range 0-65535 and server port not inside the range for threads)
// Later, other arguments can be given, which will be passed without changes to the new servers
string ProcessArgs(int argc,char *argv[],string &dname,unsigned short &entry_port,unsigned short &iniport,unsigned short &finport)
{
 if (argc<NUM_NATIVE_ARGS_TO_PROG)
 {
  cerr << "Usage:\n";
  cerr << "    " << argv[0] << " hostname base_port port_range [arguments_to_instance]\n";
  cerr << "where\n";
  cerr << "   hostname is the name by which this host can be accessed. It can be the FQDN or the IP.\n";
  cerr << "   base_port is the port where the default server (hub) will be opened.\n";
  cerr << "   port_range has the form initialport-finalport to indicate the range of ports in which the instances will be opened.\n";
  cerr << "   arguments_to_instance is a list of as many arguments as needed which will be passed to each instance without changes.\n\n";
 
  iniport = IPPORT_RESERVED+2;
  finport = std::numeric_limits<unsigned short>::max();
  entry_port = IPPORT_RESERVED+1;
  
  cerr << "    For this machine a recommendation is to use " << entry_port << " as base port and " << iniport << "-" << finport << " as range,\n";
  cerr << "    unless you know that some ports in range are being used by other services or programs.\n\n";
  exit(1);
 }
 
 
 string appname=RecoverAppName(string(argv[0]));

 dname = string(argv[1]);
 entry_port = atoi(argv[2]);
 string srange=string(argv[3]);
 size_t p=srange.find("-");
 if (p==string::npos)
 {
  cerr << "   Error: " << srange << " is not a valid port range (no '-' symbol in it).\n";
  exit(1);
 }
 int dummy = atoi(srange.substr(0,p).c_str());
 if ((dummy<0) || (dummy > std::numeric_limits<unsigned short>::max()))
 {
  cerr << "   Error: invalid initial port range. It is bigger than " << std::numeric_limits<unsigned short>::max() << ".\n";
  exit(1);
 }
 else
  iniport = (unsigned short) dummy;
 dummy = atoi(srange.substr(p+1,string::npos).c_str());
 if ((dummy<0) || (dummy > std::numeric_limits<unsigned short>::max()))
 {
  cerr << "   Error: invalid final port range. It is bigger than " << std::numeric_limits<unsigned short>::max() << ".\n";
  exit(1);
 }
 else
  finport = (unsigned short) dummy;
  
 if (iniport>=finport)
 {
  cerr << "   Error: invalid range [" << iniport << "..." << finport << "]\n";
  exit(1);
 }
 if ((entry_port>=iniport) && (entry_port<=finport))
 {
  cerr << "   Error: port " << entry_port << " is inside the range [" << iniport << "..." << finport << "]\n";
  exit(1);
 }
 return appname;
}

int main(int argc,char *argv[])
{
 unsigned short iniport,finport,range,entry_port;
 string dname;
 
 string appname=ProcessArgs(argc,argv,dname,entry_port,iniport,finport);
 range=finport-iniport;
 
 // Request an array of range bytes (boolean marks) that will mark which ports are used at each moment)
 // as a shared memory segment, whose identifier the threads will know.
 int shmid = shmget(IPC_PRIVATE,range,0777|IPC_CREAT);
  
 // The former array could be altered by the main program and by its threads, so we declare a mutex to protect it
 if (pthread_mutex_init(&only_mutex, NULL) != 0)
 {
  cerr << "Mutex initialization has failed\n";
  exit(1);
 }
 
 // This copies the non-native arguments (see function ProcessArgs) to be passed to the threads 
 // in a field of the args_to_main strcture
 args_to_main new_args;
 new_args.newargc = argc-NUM_NATIVE_ARGS_TO_PROG;
 
 if (new_args.newargc>0)
 {
  new_args.newargv = new char*[new_args.newargc];
 
  for (int i=0;i<new_args.newargc;i++)
  {
   new_args.newargv[i] = new char[strlen(argv[NUM_NATIVE_ARGS_TO_PROG+i])];
   strcpy(new_args.newargv[i],argv[NUM_NATIVE_ARGS_TO_PROG+i]);
  }
 }
 else
  new_args.newargv = nullptr;

 // The other fields of the strcture are filled here with appropriate values
 new_args.sharedmem_id = shmid;
 new_args.initial_port = iniport;
 new_args.port_range = range;
 
 // This is the buffer in which the request sent by the user as a URL will be stored, but it will not be used.
 char *buf=new char[BUFFER_SIZE+1];
 
 // This shared memory is attached to an address in our address space
 bool *used_ports = (bool *) shmat(shmid,0,0);
 
 // Now, the code to manage the interaction through sockets, either with or without SSL.
#ifdef SECURE_HTTP
 Poco::Net::Context::Ptr ptr_ctx=PrepareSSLContext(nullptr,cerr);
 if (SERVERDEB)
  cout << "Main server: SSL context prepared. Pointer at " << ptr_ctx << endl;
 Poco::Net::SecureServerSocket listener(ptr_ctx);
#else
 Poco::Net::ServerSocket listener;
#endif

 // socket is bound to the port we wish..
 ostringstream address_and_port_stream;
 address_and_port_stream << Poco::Net::DNS::thisHost().name() << ":" << entry_port;
 string address_and_port = address_and_port_stream.str();
 Poco::Net::SocketAddress sa(address_and_port);

 try
 {
  listener.bind(sa);
  // In principle, this socket (which is opened in blocking mode by default) should wait forever
  listener.setBlocking(true);
  // Also, the connection, once stablished. should be kept alive all the time (it's like a session)
  listener.setKeepAlive(true);
  // and the port should not be reused (but change define ALLOW_PORT_REUSE up, if you want...)
  listener.setReusePort(ALLOW_PORT_REUSE);
  // We could set a timeout here, like in the thread version, but since this is essentially to
  // test the application, we prefer not to do it. socket will await for client's request indefinitely.
 }
 catch (Poco::IOException& exc)
 {
  cerr << "Received IOException from the POCO libraries after binding of socket to host " << address_and_port << endl;
  string e=exc.displayText();
  if (e.find("Address already in use") != std::string::npos)
   cerr << e << "\nWe don't know how to deal with this dirty error. You will have to wait for a while until the system decides you can use the port again...\n";
  else
   cerr << "It might be due to the binding of the socket ifself or to the fact of setting it as non-blocking, keep-alive and not-reuse-port.\n" << e << endl;
  exit(1);
 }
 if (listener.getError())
  AnnounceError(Bind,listener.lastErrorDesc(),entry_port,cerr,0,0);
 
 // Setting the size of the queue.  I'm not really sure about the right value here... the maximum, in /proc/sys/net/core/somaxconn1 is 65535 
 int queue_size=32768;
 try
 {
  listener.listen(queue_size);
 }
 catch (Poco::IOException& exc)
 {
  cerr << "Received IOException from the POCO libraries after call to listen with queue size of " << queue_size << endl;
  cerr << exc.displayText() << endl;
 }
 if (listener.getError())
  AnnounceError(Listen,listener.lastErrorDesc(),entry_port,cerr,0,0);

 unsigned short newport,newpos;
 pthread_t pidt;
 srand(time(nullptr));
 
 // Let's create a thread argument for all the threds that will be created. The reason, instead
 // of using the default values, it that we want to make the threads dettached so when they finish,
 // its resources are automatically liberated 
 pthread_attr_t attr;
 pthread_attr_init(&attr);
 pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
 
 Poco::Net::SocketAddress Cli;
 // Just for information
 if (SERVERDEB)
 {
#ifdef SECURE_HTTP
  cout << "++++ Main server is listening requests of the form https://" << address_and_port << "/" << appname << " ****\n";
#else
  cout << "++++ Main server is listening requests of the form http://" << address_and_port << "/" << appname << " ****\n";
#endif
 }
 cout.flush();
 
 do
 {
  // Socket is set to accept connections and return a socket in connected state
#ifdef SECURE_HTTP
  Poco::Net::SecureStreamSocket sts;
#else
  Poco::Net::StreamSocket sts;
#endif 

  try
  {
   sts=listener.acceptConnection(Cli);
  }
  catch (Poco::IOException& exc)
  {
   cerr << "Received IOException from the POCO libraries after acceptConnection:\n";
   cerr << exc.displayText() << endl;
   continue;
  }
  if (listener.getError())
  {
   AnnounceError(AcceptCon,listener.lastErrorDesc(),entry_port,cerr,0,0);
   continue;
  }

  // Here we see how many bytes, up to BUFFER_SIZE, we receive as the URL sent.
  // Overflow is checked to avoid attacks
  buf[0]='\0';
  int nbytes;
  try
  {
   nbytes=sts.receiveBytes(buf,BUFFER_SIZE,0);
  }
  catch (Poco::IOException& exc)
  {
   cerr << "Received IOException from the POCO libraries after receiveBytes:\n";
   string e=exc.displayText();
   if ((e.find("0A000416")!=string::npos) ||
       (e.find("0A000412")!=string::npos) ||
       (e.find("0A000126")!=string::npos)
      )
   {
    cerr << "  Browser send us a rejection of certificate, even before user has opted for accepting or rejecting it.\n";
    cerr << "  We decide to ignore such request and wait for next packet.\n";
    cerr << "  Error was " << e << endl;
   }
   else
    if (e.find("0A00009C")!=string::npos)
    {
     cerr << "It seems client is trying to send us a HTTP request while we expect cyphered HTTPS.\n";
     cerr << "  We decide to ignore such rejection and wait for next packet.\n";
     cerr << "  Error was " << e << endl;
    }
    else
     cerr << "Error is " << e << endl;
   continue;
  } 
  if (sts.getError())
  {
   AnnounceError(Receive,sts.lastErrorDesc(),entry_port,cerr,nbytes,0);
   continue;
  }
 
  // To prevent someone to have sent exactly the number of bytes to be accepted but not the end-of-string...
  buf[BUFFER_SIZE]='\0';
   
  // Now, let's look if this is a http/https request. Otherwise, we'll ignore it
  // (remember: we are a web server)
  string sreq(buf);
  istringstream istr(sreq);
  Poco::Net::HTTPRequest rq;
  try
  {
   rq.read(istr);
  }
  catch (Poco::IOException &exc)         // This could happen if someone send bytes to our server port which are not a HTTP(S) request
  {
   cerr << "Received MessageException from the POCO libraries:\n";
   cerr << exc.displayText() << endl;
   cerr << "Ignoring that packet.\n";
   // This is not fatal, we ignore those bytes and start listening again
   continue;
  } 
    
  string URI=rq.getURI();            // It we have really received a HTTP(S) request...
   
  if (URI=="/favicon.ico")
   continue;
 
  if (URI!="/"+appname)                   // ...let's see if it is for our application. The convention is that the URI is http(s)://server:port/application_name
  {
   if (SERVERDEB)
    cerr << "Main server receives HTTP request with URI '" << URI << "' instead of '/" << appname << "' We ignore such URI.\n";
   continue;
  }
  else
   if (SERVERDEB)
    cout << "Main server receives HTTP resquest with URI '" << URI << "', as expected.\n";
  
  // Now, let's choose the next unused port in the range.
  // The array of booleans of used parts has 'range' items where the first one represents the first port in the range, and so on.
  // This array is changed here, marking as true the fact we are using a new port, and by every thread marking as false just before leaving
  // to announce that it does not need the port anymore. Since the array is altered by this main program and by the threads, it has
  // to be protected by a mutex.
  pthread_mutex_lock(&only_mutex);
  
  // Choose a new position in the array which is not true (i.e.: port used)
  // For sequentially chosen ports:
  newpos=0;
  while (newpos<range && used_ports[newpos])
   newpos++;  
  // For randomly assigned ports:
  //do
  //{
  // newpos=(unsigned short)(float(rand())*range/RAND_MAX);
  //}
  //while (used_ports[newpos]);
   
  if (newpos<range)
   used_ports[newpos]=true;
  
  pthread_mutex_unlock(&only_mutex);
    
  if (newpos>=range)
  {
   cerr << "From main server: NO PORTS LEFT!!! ";
   if (SendBusyPage(entry_port,sts,cerr))
    cerr << "Warning page sent to client.\n";
   else
    cerr << "and we are so busy that not even the Warning page has been sent... This is serious!!!\n"; 
  }
  else
  { 
   newport = iniport+newpos;
    
   // This function injects the initial page into the open socket
   if (SendInitialPage(dname,newport,sts,cerr))
   {
    bool thread_created_correctly=true;
    // if the redirection page has been sent without errors, the thread is opened
    new_args.port=newport;
    int s;
    void *sp;
    s = posix_memalign(&sp, sysconf(_SC_PAGESIZE), 0x800000);
    if (s != 0)
    {
     cerr << "Error in posix_memalign. Error code is " << s << endl;
     thread_created_correctly=false;
    }
    else
    {
     s = pthread_attr_setstack(&attr, sp, 0x800000);
     if (s != 0)
     {
      cerr << "Error in pthread_attr_setstack. Error code is " << s << endl;
      thread_created_correctly=false;
     }
     else
     {  
      int pret=pthread_create(&pidt,&attr,CALL_TO_THREAD,&new_args);
      if (pret!=0)
      {
       cerr << "From main server: pthread_create has failed. Thread was not created. Error is ";
       switch (pret)
       {
        case EAGAIN: cerr << "EAGAIN (Insufficient resources)\n"; break;
        case EINVAL: cerr << "EINVAL (Invalid setting in attr)\n"; break;
        case EPERM: cerr << "EPERM (No permissions to set the scheduling policiy)\n"; break;
        default: cerr << "Unknown error ???\n"; break;
       }
       thread_created_correctly=false;
      }
     }
     if (!thread_created_correctly)
     {
      pthread_mutex_lock(&only_mutex);
      used_ports[newpos]=false;
      pthread_mutex_unlock(&only_mutex);
     }
    }
   }
   else
   {                                             // If the redirection page cannot be sent, the thread is not created and the port is decleared as available
    pthread_mutex_lock(&only_mutex);
    used_ports[newpos]=false;
    pthread_mutex_unlock(&only_mutex);
   }
  }
 }
 while (1);

 // At the end, the shared memory is dettached...
 shmdt(used_ports);
 // ... and returned to the system as not used anymore
 shmctl(shmid,IPC_RMID,0);
 
 return 0;
}

