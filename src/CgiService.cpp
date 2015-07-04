#include <fcgi_stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>

#include "Log.h"
#include "CgiService.h"
#include "Response.h"
#include "Request.h"
#include "base64.h"
#include "Config.h"

#include <typeinfo>
#include <iostream>
#include <vector>
#include <fstream>
#include <streambuf>

#define THREAD_STACK_SIZE PTHREAD_STACK_MIN + 10 * 1024

CgiService::CgiService(unsigned threadsNumber, const std::string &serverSocketPath):
    _pMtx(PTHREAD_MUTEX_INITIALIZER),
    _pThreadsNumber(threadsNumber),
    _pServerSocketPath(serverSocketPath)
{
    FCGX_Init();

    mode_t old_mode = umask(0);
    socketId = FCGX_OpenSocket(_pServerSocketPath.c_str(), _pThreadsNumber);
    if(socketId < 0)
    {
        Log::err("Error open socket. exit");
        ::exit(1);
    }
    umask(old_mode);

    struct sigaction actions;

    memset(&actions, 0, sizeof(actions));
    actions.sa_flags = 0;
    actions.sa_handler = SignalHandler;

    sigaction(SIGHUP,&actions,NULL);
    sigaction(SIGPIPE,&actions,NULL);


    pthread_cond_init(&_pCondVar,NULL);

    pthread_attr_t* attributes = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
    pthread_attr_init(attributes);
    pthread_attr_setstacksize(attributes, THREAD_STACK_SIZE);

    _pThreads = new pthread_t[_pThreadsNumber + 1];

    for(unsigned i = 0; i < _pThreadsNumber; i++)
    {
        if(pthread_create(&_pThreads[i], attributes, &this->Serve, this))
        {
            Log::err("creating thread failed");
            ::exit(1);
        }
    }

    pthread_attr_destroy(attributes);
    free(attributes);

    _pBuffer = new unsigned char[cfg->BufferSize];

    _pIndexHtml = getFileContents(cfg->indexFile);
}

std::string CgiService::getFileContents(const std::string &filename)
{
    int fd;
    struct stat stat_buf;
    std::string retString;

    if( (fd = open(filename.c_str(), O_RDONLY))<2 )
    {
        Log::err("error open file: %s",filename.c_str());
    }

    ssize_t sz = fstat(fd, &stat_buf) == 0 ? stat_buf.st_size : -1;
    char *buf = (char*)malloc(sz);

    bzero(buf,sz);

    int ret = read(fd, buf, sz);

    if( ret != sz )
    {
        printf("Error read file: %s",filename.c_str());
    }
    close(fd);

    retString = std::string(buf,ret);

    free(buf);

    return retString;
}

void CgiService::run()
{
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> vjpg;
    unsigned i = 0;

    if ((dir = opendir (cfg->imagesDir.c_str())) != NULL)
    {
        while((ent = readdir (dir)) != NULL)
        {
            if(std::string(ent->d_name) == "." || std::string(ent->d_name) == "..")continue;
#ifdef DEBUG
            printf ("%s\n", ent->d_name);
#endif // DEBUG
            vjpg.push_back(getFileContents(cfg->imagesDir+"/"+std::string(ent->d_name)));

            i++;
        }
        closedir (dir);
    }

    i = 0;
   //main loop
    for(;;)
    {
        pthread_mutex_lock(&_pMtx);

        if(vjpg.size() && i < vjpg.size())
        {
            memcpy(_pBuffer,vjpg[i].c_str(),vjpg[i].size());
            _pLen = vjpg[i].size();

            pthread_cond_broadcast(&_pCondVar);

            i++;
        }
        else
        {
            i = 0;
        }

        pthread_mutex_unlock(&_pMtx);

        usleep(cfg->FPS);
    }
}

CgiService::~CgiService()
{
    pthread_cond_destroy(&_pCondVar);

   for(unsigned i = 0; i < _pThreadsNumber; i++)
    {
        pthread_join(_pThreads[i], 0);
    }

   delete []_pThreads;
}


void *CgiService::Serve(void *data)
{
    CgiService *csrv = (CgiService*)data;

    FCGX_Request request;

    if(FCGX_InitRequest(&request, csrv->socketId, 0) != 0)
    {
        Log::err("Can not init request");
        return nullptr;
    }

    sigset_t sigpipe_mask;
    sigemptyset(&sigpipe_mask);
    sigaddset(&sigpipe_mask, SIGPIPE);
    sigset_t saved_mask;
    if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1)
    {
        perror("pthread_sigmask");
        exit(1);
    }


    static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

    for(;;)
    {
        pthread_mutex_lock(&accept_mutex);
        int rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if(rc < 0)
        {
            Log::err("Can not accept new request");
            break;
        }


        csrv->ProcessRequest(&request);
    }

    Log::info("thread: %dl  exit.",pthread_self());

    return nullptr;
}

void CgiService::ProcessRequest(FCGX_Request *req)
{
    //response any way
    Response resps = Response(req);

    Request reqst = Request(req);

    if(reqst.status != 200)
    {
        resps.status(reqst.status);
        return;
    }


    if (reqst.param("action") == "control")
    {
        if(reqst.param("dir")=="up")
        {
            std::cout<<"thread: "<<pthread_self()<<" control command: up"<<std::endl;
        }
        else if(reqst.param("dir")=="down")
        {
            std::cout<<"thread: "<<pthread_self()<<" control command: down"<<std::endl;
        }
        else if(reqst.param("dir")=="left")
        {
            std::cout<<"thread: "<<pthread_self()<<" control command: left"<<std::endl;
        }
        else if(reqst.param("dir")=="right")
        {
            std::cout<<"thread: "<<pthread_self()<<" control command: right"<<std::endl;
        }

        resps.status(200);
        return;
    }

    if(reqst.param("action") == "stream")
    {
        resps.startMJPG();

        Log::gdb("stream thread: %dl start",pthread_self());
        for(;;)
            try
            {
                pthread_mutex_lock(&_pMtx);
                pthread_cond_wait(&_pCondVar,&_pMtx);
                pthread_mutex_unlock(&_pMtx);

                if(!resps.nextMJPG(_pBuffer,_pLen))
                {
                    Log::gdb("stream thread: %dl exit",pthread_self());
                    resps.endMJPG();
                    return;
                }
            }
            catch (std::exception const &ex)
            {
                Log::err("exception %s: name: %s while processing:", typeid(ex).name(), ex.what());//, query.c_str());
                resps.status(503);
                return;
            }

    }

    resps.ok(_pIndexHtml,"");
}

void CgiService::SignalHandler(int signum)
{
    switch(signum)
    {
    case SIGHUP:
        Log::info("CgiService: sig hup");
        break;
    case SIGPIPE:
        Log::info("CgiService: sig pipe");
        break;
    }
}
