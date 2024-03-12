 // Initially, next_state is empty
 string next_state="";

 // Variable to cover the special case of the first page
 bool at_start=true;
 
 // This is the buffer in which the request sent by the user as a URL will be stored.
 char *buf=new char[BUFFER_SIZE+1];

 // Now, the code to manage the interaction through sockets, either with or without SSL.
#ifdef SECURE_HTTP
 Poco::Net::Context::Ptr ptr_ctx=PrepareSSLContext(nullptr,cerr);
 if (SERVERDEB)
  cout << "Main server: SSL context prepared. Pointer at " << ptr_ctx << endl;
 Poco::Net::SecureServerSocket listener(ptr_ctx);
#else
 Poco::Net::ServerSocket listener;
#endif

 // The socket 'listener' is bound to the chosen port.
 ostringstream address_and_port_stream;
 address_and_port_stream << Poco::Net::DNS::thisHost().name() << ":" << port;
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
  AnnounceError(Bind,listener.lastErrorDesc(),port,cerr,0,0);
 
 // Setting the size of the queue.  I'm not really sure about the right value here...
 // I have set the maximum in /proc/sys/net/core/somaxconn to 65535 so I'll use this value here, too... 
 int queue_size=65535;
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
  AnnounceError(Listen,listener.lastErrorDesc(),port,cerr,0,0);
 
 // These variables are the IP adress of the client the first time it connect with us, and later.
 Poco::Net::SocketAddress Cli,IniCli;
 // Variable to mark if we are in the first stablished connection
 bool initial_connection=true;
 
#ifdef SECURE_HTTP
 if (SERVERDEB)
  cout << "*** Program starts accepting requests of the form https://" << address_and_port << " ***\n";
#else
 if (SERVERDEB)
  cout << "*** Program starts accepting requests of the form http://" << address_and_port << " ***\n";
#endif
  
 Poco::Net::HTMLForm FormWithIVars;
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
   AnnounceError(AcceptCon,listener.lastErrorDesc(),port,cerr,0,0);
   continue;
  }
 
  // If this is our first connection, let's take note of the address, to verify it wlll not change later
  // This is to prevent some forms of person-in-the-middle attack (even in this case, Eve will have tried
  // to impersonate the IP when introducing her fake packets, too, but...). Admittedly, paranoid.
  if (initial_connection)
  {
   IniCli=Cli;
   initial_connection=false;
  }
  if ( IniCli.host() != Cli.host() )
  {
   ostringstream err_desc_st;
   err_desc_st << "Received request from a different address (" << Cli.host() << " instead of " << IniCli.host() << "). We'll not reply. Trial of attack?";
   AnnounceError(ChangeAddr,err_desc_st.str(),port,cerr,0,0);
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
    cerr << "  We decide to ignore such rejection and wait for next packet.\n";
    cerr << "  Error was " << e << endl;
   }
   else
    if (e.find("0A00009C")!=string::npos)
    {
     cerr << "It seems client is trying to send us a HTTP request while we expect cyphered HTTPS.\n";
     cerr << "  We decide to ignore such request and wait for next packet.\n";
     cerr << "  Error was " << e << endl;
    }
    else
     cerr << "Error is " << e << endl;
   continue;
  } 
  if (sts.getError())
  {
   AnnounceError(Receive,sts.lastErrorDesc(),port,cerr,nbytes,0);
   continue;
  }
 
  // To prevent someone to have sent exactly the number of bytes to be accepted but not the end-of-string...
  buf[BUFFER_SIZE]='\0';
   
  theContext.ResetError();

  // Now, the middle level: the contents of the buffer of bytes is parsed to get the URL and 
  // the list of variables/values (the only thing we are really interested in).
  // The POCO libraries make this very well...
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
    
  FormWithIVars.load(rq);

  string URI=rq.getURI();

  // These are checks to prevent malicious change of the URL by the client just before sending it to us
  // Check of maximum length...
  if (URI.length()>=MAX_URI_LENGTH)
  {
   URI=URI.substr(0,MAX_URI_LENGTH);
   theContext.SetError(URL_TOO_LONG);
   cerr << "WARNING: received URL too long. " << URI.length() << " characters, more than the " << MAX_URI_LENGTH << " allowed. It will be truncated." << endl;
  }
  // ...and check of introduction of spurious characters
  if (theContext.InvalidChars(URI,'e'))
  {
   theContext.SetError(FORBIDDEN_CHAR);
   cerr << "WARNING: received URL with invalid characters. They have all been substituted by the character 'e'" << endl;
  }

  // This is just to give info about the received URI
  if (SERVERDEB && URI.find("favicon")==string::npos)
  {
   cout << "Received request with URI " << URI;
   if (FormWithIVars.size() > 0)
   {
    cout << " and variables:\n";
    for (Poco::Net::NameValueCollection::ConstIterator it=FormWithIVars.begin(); it!=FormWithIVars.end(); ++it)
     cout << it->first << " --> " << it->second << endl;
   }
   else
    cout << " and no variables.\n";
   cout.flush();
  }

  // If no variables have been passed in the URL, either the user is asking for a file or it is an error.
  bool try_to_send_file = (FormWithIVars.size()==0);
  bool file_sent;
  if (try_to_send_file)
  {
   // This is for the case the client is not asking for the next state but he/she is asking for a file
   string mimetype;
   size_t filelength;
   // The function RequestedFile analyzes the request and see if it should be honored
   // If if should, fsend contains the path of the file to be sent. Otherwise the function returs the empty string.
   string fsend=theContext.RequestedFile(URI,mimetype,filelength);

   if (fsend == "")
    file_sent=(URI.find("favicon")!=string::npos);
   else
   {
    if (SERVERDEB)
    {
     cout << "Requested the valid file " << fsend << " with length " << filelength;
     cout << ((filelength<=0) ? ". The file exists but, since it is empty, we will not send it.\n" : ". Sending it.\n");
    } 
    if (filelength>0)
     SendFile(fsend,mimetype,filelength,port,sts,cerr);
    // We consider the file as "sent" even if it was emtpy. This is because asking for a file, even empty, does not change state.
    file_sent=true;
   }
  }
  
  // The next part covers the case we were not trying to send any file but the corresponding page for the next state.
  // In this case the URL MUST have variables; otherwise, it is an error.   
  if (try_to_send_file==false || file_sent==false)
  {
   // The tf map maps the name of the current state to the function (indeed, one of the wrappers)
   // that, possibly according to the values of the variables, chooses the next state and generates
   // the page this next state has to send.
   // Remember tf is a map from string --> pointer to function, where the addressed function has
   // the prototype
   //   string function(Poco::Net::NameValueCollection &,string &,Context &)
     
   if (this_state=="")
   {
    cerr << "Error: variable this_state is empty ???\n";
        
    exit(1);
   }
   
   if (at_start)
   {
    if (SERVERDEB)
     cout << "System is in state " << this_state << ", ready to send the initial page.\n";
    SendPage(page,port,sts,cerr);
    at_start=false;
   }
   else
   {
    try
    {
     page = tf[this_state](FormWithIVars,next_state,theContext);
    }
    catch (const std::exception &e)
    {
     cerr << "Error: The former exception happened in main server when executing the function tf[" << this_state << "]\n";    
     exit(1);
    }
       
    if (theContext.GetError()>=TOO_FEW_VARIABLES && SERVERDEB)
     cerr << "WARNING: the call to the transition function " << nf[this_state] << " has provoked this error: " << theContext.ReadableError() << endl;
    if (SERVERDEB)
    {
     cout << "System is going from state " << this_state << " to state " << next_state << endl;
     cout << "Sending the page for state " << next_state << endl;
    }
     
    // This is the ckeck to see if the user has passed invalid values for at least one variable
    // In this case, the Context class raises and error, and function ModifyPage is called to inject
    // some info about the the error in the page to be sent so that the client is aware of what he/she
    // did badly, or knows that his/her malicious trials to cheat us have been defeated.
    unsigned long context_err=theContext.GetError();
    try
    {
     if (context_err!=NO_ERROR)
      theContext.ModifyPage(page,context_err);
    }
    catch (const std::exception &e)
    {
     cerr << "Error: The former exception happened in main server when modifying the page:\n==========\n";
     cerr << page << "\n==========\n";
        
     exit(1);
    }
       
    // This function injects the complete page (contained into string page) into the open socket
    SendPage(page,port,sts,cerr);
  
    // And finally, the state is updated.
    this_state = next_state;
   }
  }
 }
 while ( this_state != "End_of_program" );

 if (SERVERDEB)
  cout << "Process ends.\n";
 
 return 0;
}

