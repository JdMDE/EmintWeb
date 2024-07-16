// Function to exit from the thread correctly, which means let the parent know we are not using the assigned port any more,
// releasing the access to the shared memory and returning null from the thread.
// It needs four arguments, but since we want to install this function to be called at the unexpected exit provoked by a signal
// and these functions admit a pointer to void as its only argument, we need a structure to group the arguments.

void signal_handler_thread(int signum)
{
 switch (signum)
 {
  case SIGSEGV: cerr << "SEGV"; break;
  case SIGBUS:  cerr << "BUS"; break;
  case SIGFPE:  cerr << "FPE"; break;
  default: cerr << "UNKNOWN (signal number is " << signum << ")\n"; break;
 }
 cerr << " signal received by thread with identifier " << pthread_self() << endl;
 pthread_exit(NULL);
}

void abrt_handler_thread(int signum)
{
 cerr << "Received ABRT signal.\n";
 thread_info *used_ports = (thread_info *) shmat(shmid, NULL, 0);
 if ((void *)used_ports == (void *)(-1))
 {
  cerr << "No thread info memary can be acessed for the SIGABRT handler. This is catastrophic. We have to leave. Sorry.\n";
  pthread_exit(NULL);
 }
 
 pthread_t me=pthread_self();
 int place=0;
 while (place<port_extension && used_ports[place].thpid != me)
  place++;
 if (place>=port_extension)
 {
  cerr << "No thread identifier was found in the shared memory area. This is catastrophic. We have to leave. Sorry.\n";
  pthread_exit(NULL);
 } 
 
 pthread_mutex_lock(&only_mutex);
 used_ports[place].state = (RUNNING | RECEIVED_ABRT);
 pthread_mutex_unlock(&only_mutex);
 
 while(true)
 {
  sleep(3600);
 }

}

typedef struct
{
 thread_info *used_ports;
 unsigned short place;
 ostream *errst;
} args_to_LeaveThreadCorrectly;

static void LeaveThreadCorrectlyOneArg(void *argp)
{
 args_to_LeaveThreadCorrectly *a=(args_to_LeaveThreadCorrectly *)argp;
 
 LeaveThreadCorrectly(a->used_ports,a->place,a->errst,true);
}

void LeaveThreadCorrectly(thread_info *used_ports,unsigned short place,ostream *errst,bool caused_by_signal)
{
 if (ALLOW_PORT_REUSE)
 {
  /* We are not using this port anymore, and this is signaled into the shared memory array of used ports
     so that the superserver can assign it again.
     Change is done with the precaution of using the mutex that protects the access to the used_ports array */
  pthread_mutex_lock(&only_mutex);
  used_ports[place].state=AVAILABLE;
  pthread_mutex_unlock(&only_mutex);
 }
 /* It is possible that we don't want to reuse the port, for testing purposes (an automatic test program needs to know
    which the used ports will be in a predictable way). This is mainly for tests with JMeter. */
 // Whatever the case, this part of the shared memory is dettached from our user space
 if (used_ports!=nullptr)
 {
  if (shmdt(used_ports)!=0)
   (*errst) << "Error from thread with ID " << pthread_self() << ": cannot detach the shared memory attached to know the used ports. Not fatal.\n";
 }
 if (SERVERDEB)
 {
  (*errst) << "Leaving thread with ID " << pthread_self() << endl;
  if (caused_by_signal)
   (*errst) << "THIS IS AN UNEXPECTED TERMINATION CAUSED BY THE RECEPTION OF A SIGNAL (SIGABRT, SIGBUS, SIGSEGV or SIGFPE). Please, consult the log of the metaserveer.\n";
 }
 (*errst).flush();
}

void *$PROJECT_NAMECall(void *arg)
{
 args_to_main *a = (args_to_main *)arg;
 
 unsigned short port = a->port;
 unsigned short iniport=a->initial_port;
 
 if ((port<iniport) || (port>iniport+port_extension))
 {
  cerr << "Port " << port << ": error. Port number not in [" << iniport << "," << iniport+port_extension << "]\n";
  pthread_exit(nullptr);
 }
 
 Context theContext;
 theContext.SetPort(port);
 if (SERVERDEB)
  theContext.SetOutletType(OUT_TO_FILE);
 else
  theContext.SetOutletType(OUT_TO_CERR);
  
 theContext.SetArgs(a->newargc,a->newargv);
 theContext.InitializeContext();
 if (theContext.GetError()!=NO_ERROR)
 {
  theContext.GetOutletStream() << "Error in context creation. We can't execute the program. Review you additions to the Context() constructor.\n";
  // LeaveThreadCorrectlyOneArg has not yet been installed, so we must call LeaveThreadCorrectly explicitly and exit from the thread afterwards
  LeaveThreadCorrectly(nullptr,port-iniport,&(theContext.GetOutletStream()),false);
  // Nothing to return, since this thread is not joinable
  pthread_exit(nullptr);
 }
 
 ostringstream address_and_port_stream;
 address_and_port_stream << Poco::Net::DNS::thisHost().name() << ":" << port;
 string address_and_port = address_and_port_stream.str();
 
 pthread_t pthread_id=pthread_self();
 if (SERVERDEB)
 {
  string prot;
#ifdef SECURE_HTTP
  prot="https";
#else
  prot="http";
#endif

  theContext.GetOutletStream() << "*** Thread with ID " << pthread_id << " starts accepting requests of the form " << prot << "://" << address_and_port << " ***\n";
  theContext.GetOutletStream().flush();
 }

 thread_info *used_ports = (thread_info *) shmat(shmid, NULL, 0);
 if ((void *)used_ports == (void *)(-1))
 {
  theContext.GetOutletStream() << "Error in shmat: the shared memory to know the currently available ports cannot be mapped to this thread's local address space. Exiting from the thread.\n";
  // LeaveThreadCorrectlyOneArg has not yet been installed, so we must call LeaveThreadCorrectly explicitly and exit from the thread afterwards
  LeaveThreadCorrectly(used_ports,port-iniport,&(theContext.GetOutletStream()),false);
  // Nothing to return, since this thread is not joinable
  pthread_exit(nullptr);
 }
 
 bool retsetargs;
 if (a->newargc>0)
  retsetargs=theContext.SetArgs(a->newargc,a->newargv);
 else
  retsetargs=theContext.SetArgs(0,NULL);
 if (!retsetargs)
 {
  // LeaveThreadCorrectlyOneArg has not yet been installed, so we must call LeaveThreadCorrectly explicitly and exit from the thread afterwards
  LeaveThreadCorrectly(used_ports,port-iniport,&(theContext.GetOutletStream()),false);
 // Nothing to return, since this thread is not joinable
  pthread_exit(nullptr);
 }
 
 // Now, let's install the signal handler for the most likely signals that provoke unexpected errors: SEGV, BUS and PFE
 args_to_LeaveThreadCorrectly alc;
 alc.used_ports = used_ports;
 alc.place = port-iniport;
 alc.errst = &(theContext.GetOutletStream());
 
 struct sigaction action;
 
 action.sa_handler = signal_handler_thread;
 sigemptyset(&action.sa_mask);
 sigaddset(&action.sa_mask,SIGSEGV | SIGFPE | SIGBUS | SIGABRT);
 action.sa_flags = SA_RESETHAND;
 sigaction(SIGBUS, &action, NULL);
 sigaction(SIGSEGV, &action, NULL);
 sigaction(SIGFPE, &action, NULL);
 
 action.sa_handler = abrt_handler_thread;
 sigemptyset(&action.sa_mask);
 action.sa_flags = SA_RESETHAND;
 sigaction(SIGABRT, &action, NULL);
 
 // Finally, let's install the clean-up function to be called at pthread_exit
 pthread_cleanup_push(LeaveThreadCorrectlyOneArg,&alc);

