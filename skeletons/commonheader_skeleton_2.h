// Prototypes for the defined functions\n

void AnnounceError(ServerErrors error_type,string err_desc,unsigned short port,ostream &errst,int len,int exp_len);
string Terminal_transition(Poco::Net::NameValueCollection &FormWithIVars,string &next_st,Context &usercontext);
#ifdef SECURE_HTTP
Poco::Net::Context::Ptr PrepareSSLContext(pthread_t *pthread_id,ostream &errst);
void SendPage(string page,unsigned short port,Poco::Net::SecureStreamSocket &sts,ostream &errst);
void SendFile(string path,string mimetype,size_t filelength,unsigned short port,Poco::Net::SecureStreamSocket &sts,ostream &errst);
#else
void SendPage(string page,unsigned short port,Poco::Net::StreamSocket &sts,ostream &errst);
void SendFile(string path,string mimetype,size_t filelength,unsigned short port,Poco::Net::StreamSocket &sts,ostream &errst);
#endif

