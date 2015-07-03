#ifndef CGISERVICE_H
#define CGISERVICE_H

#include <string>

#include <pthread.h>

#include <fcgiapp.h>

class CgiService
{

public:
    pthread_cond_t  _pCondVar;
    pthread_mutex_t _pMtx;
    int socketId;
    CgiService(unsigned, const std::string &);
    ~CgiService();
    static void *Serve(void*);
    void run();

private:
    void ProcessRequest(FCGX_Request*);
private:
    unsigned _pThreadsNumber;
    std::string _pServerSocketPath;
    std::string _pCookieName;
    pthread_t *_pThreads;
    unsigned char *_pBuffer;
    size_t _pLen;
    std::string _pIndexHtml;

    static void SignalHandler(int signum);

    std::string randomString( size_t length );
    std::string getFileContents(const std::string &filename);
};

#endif
