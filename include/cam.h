#ifndef CAM_H
#define CAM_H

#include <string>

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace panasonic
{

std::string const CAM_CAPABILITY_REQUEST = "GET /cam.cgi?mode=getinfo&type=capability";
std::string const CAM_GETSTATE			 = "GET /cam.cgi?mode=getstate";
std::string const CAM_STREAM			 = "GET /cam.cgi?mode=startstream&value=%d";
std::string const CAM_STOPSTREAM		 = "GET /cam.cgi?mode=stopstream";
std::string const CAM_PLAYMODE			 = "GET /cam.cgi?mode=camcmd&value=playmode";
std::string const CAM_PICTMODE			 = "GET /cam.cgi?mode=camcmd&value=pictmode";
std::string const CAM_RECMODE			 = "GET /cam.cgi?mode=camcmd&value=recmode";
std::string const CAM_ZOOMOUT			 = "GET /cam.cgi?mode=camcmd&value=wide-normal";
std::string const CAM_ZOOMOUT_FAST		 = "GET /cam.cgi?mode=camcmd&value=wide-fast";
std::string const CAM_ZOOMIN			 = "GET /cam.cgi?mode=camcmd&value=tele-normal";
std::string const CAM_ZOOMIN_FAST		 = "GET /cam.cgi?mode=camcmd&value=tele-fast";
std::string const CAM_ZOOMSTOP			 = "GET /cam.cgi?mode=camcmd&value=zoomstop";

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
    bool isValid()
    {
        return state != state_t::dispose;
    }
    void zoomin()
    {
        if (((state == state_t::stream) || (state == state_t::startstream)) && (zoom < fastIn)) zoom = (zoom_t)(zoom + 1), zoomChanged = true;
    }
    void zoomout()
    {
        if (((state == state_t::stream) || (state == state_t::startstream)) && (zoom > fastOut)) zoom = (zoom_t)(zoom - 1), zoomChanged = true;
    }
    void zoomstop()
    {
        if ((state == state_t::stream) || (state == state_t::startstream)) zoom = zoom_t::none, zoomChanged = true;
    }
    void startstream()
    {
        if (state == state_t::ready) state = state_t::startstream;
    }
    void stopstream()
    {
        if ((state == state_t::stream) || (state == state_t::startstream)) state = state_t::ready;
    }

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
    char httpReqEnd[100];
    char camReqStreamString[50];
    state_t state;
    zoom_t zoom;
    sockaddr_in sa;
    pthread_t _pThread;


    void dispatcher();
    int applyZoom();

    cam(const cam&);
    cam& operator=(const cam&);

    static const char JpegHeaderStart[3];
    static const char JpegHeaderEnd[3];

    static void *streamUpdate(void*);
};
}
#endif
