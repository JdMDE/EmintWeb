// This could be changed with a project option, as we do for the SERVERDEB flag, but it's no worth, probably....
#define SEND_AUTOMATIC_REDIRECTION_IN_SERVER false
// The same happens with this. Its normal value should be true but now we are doing automatic tests with JMeter...
// Change ALLOW_PORT_REUSE to true for common use; to make automatic tests (for example, with JMeter) we prefer false
#define ALLOW_PORT_REUSE false

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <limits>
#include <map>

using namespace std;

#include <Poco/Exception.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTMLForm.h>

#ifdef SECURE_HTTP
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SecureServerSocket.h>
#else
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#endif

#include <Poco/Timespan.h>
#include <Poco/Net/DNS.h>

enum ServerErrors {
 Bind=1,
 Listen=2,
 AcceptCon=3,
 ChangeAddr=4,
 Send=5,
 Receive=6
};

// The size of the chunk to send files
#define FILE_BUF_SIZE   128

#include <html5document.h>


