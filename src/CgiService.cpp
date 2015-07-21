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
#include <time.h>

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
    _pThreadsNumber(threadsNumber),
    _pServerSocketPath(serverSocketPath)
{
    unlink(_pServerSocketPath.c_str());

    int lfp = open(_pServerSocketPath.c_str(),O_RDWR|O_CREAT,0640);

    if(lfp < 0)
    {
      syslog(LOG_ERR, "unable to create lock file %s, code=%d (%s)",
             _pServerSocketPath.c_str(), errno, strerror(errno));
      exit(EXIT_FAILURE);
    }

    unlink(_pServerSocketPath.c_str());

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

    _pIndexHtml = Config::getFileContents(cfg->indexFile);

    c1 = new panasonic::cam(cfg->camIp,cfg->camPort,cfg->camJpgLocalPort);

    rot = new Serial(cfg->rotatorDev,
                cfg->rotatorSpeedX,cfg->rotatorShiftX,
                cfg->rotatorSpeedY,cfg->rotatorShiftY);
}

void CgiService::run()
{
    c1->reciever();
}

CgiService::~CgiService()
{
    delete c1;
    delete rot;

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
        Log::err("pthread_sigmask");
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
            rot->Up();
        }
        else if(reqst.param("dir")=="down")
        {
            rot->Down();
        }
        else if(reqst.param("dir")=="left")
        {
            rot->Left();
        }
        else if(reqst.param("dir")=="right")
        {
            rot->Right();
        }
        else if(reqst.param("dir")=="zoomin")
        {
            c1->applyZoom(panasonic::zoom_t::in);
        }
        else if(reqst.param("dir")=="zoominfast")
        {
            c1->applyZoom(panasonic::zoom_t::fastIn);
        }
        else if(reqst.param("dir")=="zoomout")
        {
            c1->applyZoom(panasonic::zoom_t::out);
        }
        else if(reqst.param("dir")=="zoomoutfast")
        {
            c1->applyZoom(panasonic::zoom_t::fastOut);
        }
        else if(reqst.param("dir")=="zoomstop")
        {
            c1->applyZoom(panasonic::zoom_t::none);
        }
        else if(reqst.param("dir")=="shot")
        {
            c1->takeAshot();
        }
        else if(reqst.param("dir")=="gray")
        {
            if(c1->filterGray)
                c1->filterGray = false;
            else
                c1->filterGray = true;
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
                struct timespec ts = Config::getThreadWait();

                pthread_mutex_lock(&c1->Mtx);
                pthread_cond_timedwait(&c1->CondVar,&c1->Mtx, &ts);
                //pthread_cond_wait(&c1->CondVar,&c1->Mtx);
                pthread_mutex_unlock(&c1->Mtx);

                if(!resps.nextMJPG(c1->Buffer,c1->Len))
                {
                    Log::gdb("stream thread: %dl exit",pthread_self());
                    resps.endMJPG();
                    return;
                }

                if(c1->flagImageGet)
                {
                    c1->flagImageGet = false;
                    resps.sendMJPG(c1->ImageBuff);
                    c1->ImageBuff.clear();
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
