#ifndef $PROJECT_NAME_CALL_H
#define $PROJECT_NAME_CALL_H

#include <mutex>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

// Inactivity time in seconds just before start and at any other time
#define INACTIVITY_FIRST_WAIT_TIME 15
#define INACTIVITY_NORMAL_WAIT_TIME 360
 
#include "$PRNAME_WITH_EXT"

// Arguments to be passed to each thread. They include the arguments passed to the main
// of the metaserverm plus the particular arguments passed to each server, which come
// from the command line and will be copied in newargv
typedef struct
{
 int newargc;
 
 unsigned short port;
 unsigned short initial_port;
 
 char **newargv;
} args_to_main;

// Different states in which the thread associated to a port may be at any moment
// They are defined to allow a logical-OR, since a thread could have received several signals.
typedef enum : unsigned short
{
 AVAILABLE=0x00,               // No thread is associated to that port. It can be used.
 RUNNING=0x01,                 // The thread is running (it can't be available and running simultaneously)
 RECEIVED_ABRT=(0x01 << 1),    // The thread has received a SIGABRT signal
 RECEIVED_SEGV=(0x01 << 2),    // A SIGSEGV signal
 RECEIVED_BUS=(0x01 << 3),     // A SIGBUS signal
 RECEIVED_FPE=(0x01 << 4),     // A SIGFPE signal
 			       // Add more possibilities if more signals are handled..
 SLEEPING=(0x01 << 15)
} ThreadState;

// This structure will be put in each place of an array of as many places as possible ports
// to indicate the current state of the thread associated to that port and, if running, its identifier.
// This will be used by the watchdog thread to kill a thread if needed.
typedef struct
{
 unsigned short state;
 pthread_t thpid;
} thread_info;

void LeaveThreadCorrectly(thread_info *used_ports,unsigned short place,ostream *errst,bool caused_by_signal);
string NameOfDebFile(unsigned short port);

// Prototype of the thread to launch each instance. To be passed as argument to pthread_create
void *$PROJECT_NAMECall(void *);

extern pthread_mutex_t only_mutex;
extern int shmid;
extern int port_extension;

#endif
