#ifndef CGISERVICE_H
#define CGISERVICE_H

#include <string>

#include <pthread.h>
#include <fcgiapp.h>

#include "cam.h"

class CgiService
{

public:
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
    std::string _pIndexHtml;
    panasonic::cam *c1;

    static void SignalHandler(int signum);

    std::string randomString( size_t length );
    std::string getFileContents(const std::string &filename);
};

#endif
