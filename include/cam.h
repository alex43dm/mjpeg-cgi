#ifndef CAM_H
#define CAM_H

#include <string>

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "HttpClient.h"

namespace panasonic
{

std::string const CAM_CAPABILITY_REQUEST = "/cam.cgi?mode=getinfo&type=capability";
std::string const CAM_GETSTATE			 = "/cam.cgi?mode=getstate";
std::string const CAM_STREAM			 = "/cam.cgi?mode=startstream&value=";
std::string const CAM_STOPSTREAM		 = "/cam.cgi?mode=stopstream";
std::string const CAM_PLAYMODE			 = "/cam.cgi?mode=camcmd&value=playmode";
std::string const CAM_PICTMODE			 = "/cam.cgi?mode=camcmd&value=pictmode";
std::string const CAM_RECMODE			 = "/cam.cgi?mode=camcmd&value=recmode";
std::string const CAM_ZOOMOUT			 = "/cam.cgi?mode=camcmd&value=wide-normal";
std::string const CAM_ZOOMOUT_FAST		 = "/cam.cgi?mode=camcmd&value=wide-fast";
std::string const CAM_ZOOMIN			 = "/cam.cgi?mode=camcmd&value=tele-normal";
std::string const CAM_ZOOMIN_FAST		 = "/cam.cgi?mode=camcmd&value=tele-fast";
std::string const CAM_ZOOMSTOP			 = "/cam.cgi?mode=camcmd&value=zoomstop";

enum state_t
{
    initional,
    connected,
    ready,
    startstream,
    stream,
    dispose
};

enum zoom_t
{
    fastOut = -2,
    out,
    none,
    in,
    fastIn
};

class cam
{
public:
    pthread_cond_t  CondVar;
    pthread_mutex_t Mtx;
    unsigned char *Buffer;
    size_t Len;

    cam(const std::string &address, unsigned short remotePort, unsigned short localPort);
    ~cam();

    bool init();
    void run(const state_t &s);
    void reciever();
    bool stream(bool OnOff);
protected:
    bool exit;
private:
    int devSock;
    bool zoomChanged;
    std::string request(const std::string &cmd);
    state_t state;
    zoom_t zoom;
    sockaddr_in sa;
    pthread_t _pThread;
    HttpClient *httpClient;
    unsigned short localPort;
    std::string address;


    int applyZoom();

    cam(const cam&);
    cam& operator=(const cam&);

    static const char JpegHeaderStart[3];
    static const char JpegHeaderEnd[3];

    static void *streamUpdate(void*);
};
}
#endif
